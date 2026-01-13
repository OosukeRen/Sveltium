import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const ASSETS_PATH = path.resolve(__dirname, '../../assets/nw_versions.json')
const OUTPUT_PATH = path.resolve(__dirname, 'nw-versions.d.ts')

// Parse file entry like "win-x64" into { os, arch }
function parseFileEntry(file) {
  const [osPart, arch] = file.split('-')

  const osMap = {
    win: 'windows',
    linux: 'linux',
    osx: 'macos',
  }

  return {
    os: osMap[osPart] || osPart,
    arch,
  }
}

// Create a unique key from files array for grouping
function createFilesKey(files) {
  return [...files].sort().join('|')
}

// Create a unique key from flavors array for grouping
function createFlavorsKey(flavors) {
  return [...flavors].sort().join('|')
}

// Extract unique OS values from files
function extractOSes(files) {
  const oses = new Set()

  for (const file of files) {
    const { os } = parseFileEntry(file)
    oses.add(os)
  }

  return [...oses].sort()
}

// Extract unique arch values from files
function extractArches(files) {
  const arches = new Set()

  for (const file of files) {
    const { arch } = parseFileEntry(file)
    arches.add(arch)
  }

  return [...arches].sort()
}

// Extract OS-arch combinations
function extractPlatforms(files) {
  return files.map((file) => {
    const { os, arch } = parseFileEntry(file)
    return { os, arch }
  })
}

// Remove 'v' prefix from version
function normalizeVersion(version) {
  return version.startsWith('v') ? version.slice(1) : version
}

// Group versions by their files+flavors combination
function groupVersions(versions) {
  const groups = new Map()

  for (const entry of versions) {
    const filesKey = createFilesKey(entry.files)
    const flavorsKey = createFlavorsKey(entry.flavors)
    const groupKey = `${filesKey}::${flavorsKey}`

    if (!groups.has(groupKey)) {
      groups.set(groupKey, {
        files: entry.files,
        flavors: entry.flavors,
        versions: [],
      })
    }

    groups.get(groupKey).versions.push({
      version: normalizeVersion(entry.version),
      date: entry.date,
      components: entry.components,
    })
  }

  return groups
}

// Generate TypeScript type for a union of string literals
function generateUnionType(values) {
  if (values.length === 0) return 'never'
  return values.map((v) => `'${v}'`).join(' | ')
}

// Generate the .d.ts content
function generateTypes(data) {
  const groups = groupVersions(data.versions)

  // Collect all unique values across all versions
  const allOSes = new Set()
  const allArches = new Set()
  const allFlavors = new Set()
  const allVersions = []

  for (const entry of data.versions) {
    for (const file of entry.files) {
      const { os, arch } = parseFileEntry(file)
      allOSes.add(os)
      allArches.add(arch)
    }

    for (const flavor of entry.flavors) {
      allFlavors.add(flavor)
    }

    allVersions.push(normalizeVersion(entry.version))
  }

  // Generate group interfaces
  const groupInterfaces = []
  let groupIndex = 0

  for (const [key, group] of groups) {
    groupIndex++
    const oses = extractOSes(group.files)
    const arches = extractArches(group.files)
    const platforms = extractPlatforms(group.files)
    const versions = group.versions.map((v) => v.version)

    // Filter to only normal and sdk flavors
    const flavors = group.flavors.filter((f) => f === 'normal' || f === 'sdk')

    const platformType = platforms
      .map((p) => `{ os: '${p.os}'; arch: '${p.arch}' }`)
      .join('\n    | ')

    groupInterfaces.push(`
  /**
   * Version group ${groupIndex}
   * Versions: ${versions.slice(0, 5).join(', ')}${versions.length > 5 ? `, ... (${versions.length} total)` : ''}
   */
  export interface NWVersionGroup${groupIndex} {
    os: ${generateUnionType(oses)};
    arch: ${generateUnionType(arches)};
    flavor: ${generateUnionType(flavors)};
    version: ${generateUnionType(versions)};
    platform: ${platformType};
  }`)
  }

  // Generate the main output
  const output = `/**
 * NW.js Version Types
 * Auto-generated from nw_versions.json
 * Generated: ${new Date().toISOString()}
 */

export namespace NWVersions {
  /** All supported operating systems */
  export type OS = ${generateUnionType([...allOSes].sort())};

  /** All supported architectures */
  export type Arch = ${generateUnionType([...allArches].sort())};

  /** All supported flavors (filtered to normal and sdk only) */
  export type Flavor = 'normal' | 'sdk';

  /** All available versions (without 'v' prefix) */
  export type Version = ${generateUnionType(allVersions)};

  /** Latest version */
  export type Latest = '${normalizeVersion(data.latest)}';

  /** Stable version */
  export type Stable = '${normalizeVersion(data.stable)}';

  /** LTS version */
  export type LTS = '${normalizeVersion(data.lts)}';

  /** Platform combination (os + arch) */
  export interface Platform {
    os: OS;
    arch: Arch;
  }

  /** Version entry with full metadata */
  export interface VersionEntry {
    version: Version;
    os: OS;
    arch: Arch;
    flavor: Flavor;
  }
${groupInterfaces.join('\n')}

  /** Union of all version groups */
  export type AnyVersionGroup = ${Array.from({ length: groupIndex }, (_, i) => `NWVersionGroup${i + 1}`).join(' | ')};
}

export default NWVersions;
`

  return output
}

// Main execution
function main() {
  console.log('Reading nw_versions.json...')
  const rawData = fs.readFileSync(ASSETS_PATH, 'utf-8')
  const data = JSON.parse(rawData)

  console.log(`Found ${data.versions.length} versions`)

  console.log('Generating types...')
  const typesContent = generateTypes(data)

  console.log(`Writing to ${OUTPUT_PATH}...`)
  fs.writeFileSync(OUTPUT_PATH, typesContent, 'utf-8')

  // Print summary
  const groups = groupVersions(data.versions)
  console.log(`\nGenerated ${groups.size} version groups:`)

  let i = 0
  for (const [key, group] of groups) {
    i++
    const oses = extractOSes(group.files)
    const arches = extractArches(group.files)
    const flavors = group.flavors.filter((f) => f === 'normal' || f === 'sdk')

    console.log(`  Group ${i}: ${group.versions.length} versions`)
    console.log(`    OS: ${oses.join(', ')}`)
    console.log(`    Arch: ${arches.join(', ')}`)
    console.log(`    Flavors: ${flavors.join(', ')}`)
  }

  console.log('\nDone!')
}

main()
