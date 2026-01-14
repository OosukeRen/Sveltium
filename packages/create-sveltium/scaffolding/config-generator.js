import path from 'node:path'
import { writeFile } from '../utils/file-utils.js'

/**
 * Generate sveltium.config.js content
 * @param {object} options
 * @param {boolean} options.enableLegacy
 * @returns {string}
 */
function generateConfigContent(options) {
  const { enableLegacy } = options

  return `// @ts-check
/**
 * Sveltium Build Configuration
 *
 * Define build profiles for packaging your app with NW.js.
 * Each profile can target different NW.js versions and platforms.
 *
 * Available types for autocompletion:
 * @typedef {import('@sveltium/build-tools').NWVersions} NWVersions
 *
 * @typedef {Object} BuildProfile
 * @property {NWVersions.Version} version - NW.js version (e.g., '0.107.0', '0.15.4')
 * @property {Array<'win32'|'win64'|'linux32'|'linux64'|'osx64'|'osxarm64'>} platforms
 * @property {NWVersions.Flavor} flavor - 'normal' or 'sdk'
 * @property {string} [outputDir] - Custom output directory (optional)
 */

/**
 * @typedef {Object} SveltiumConfig
 * @property {boolean} enableLegacy - Enable legacy browser support (Chrome 40)
 * @property {Record<string, BuildProfile>} profiles - Build profiles
 */

/** @type {SveltiumConfig} */
export default {
  enableLegacy: ${enableLegacy},

  /**
   * Build profiles - define your own or use predefined ones.
   *
   * Example profiles:
   *
   * dev: {
   *   version: '0.107.0',
   *   platforms: ['win64'],
   *   flavor: 'sdk',
   * },
   *
   * xp: {
   *   version: '0.15.4',
   *   platforms: ['win32'],
   *   flavor: 'sdk',
   * },
   *
   * release: {
   *   version: '0.107.0',
   *   platforms: ['win32', 'win64'],
   *   flavor: 'normal',
   * },
   */
  profiles: {
    // Add your profiles here
  },
}
`
}

/**
 * Generate sveltium.config.js for autoGeneration feature
 *
 * @param {string} targetDir - Target directory
 * @param {object} options - Generation options
 * @param {boolean} options.enableLegacy - Legacy mode flag
 */
export async function generateSveltiumConfig(targetDir, options) {
  const content = generateConfigContent(options)
  const filePath = path.join(targetDir, 'sveltium.config.js')
  await writeFile(filePath, content)
}

export { generateConfigContent }
