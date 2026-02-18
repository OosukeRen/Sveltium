import path from 'node:path'
import { writeJson } from '../utils/file-utils.js'

/**
 * Base scripts for all projects
 */
const BASE_SCRIPTS = {
  dev: 'vite',
  build: 'vite build',
  preview: 'vite preview',
}

/**
 * Scripts added when autoGeneration is enabled
 */
const AUTO_GENERATION_SCRIPTS = {
  'sveltium:build': 'sveltium-build',
  'sveltium:run': 'sveltium-run',
}

/**
 * Base devDependencies for all projects
 */
const BASE_DEV_DEPENDENCIES = {
  vite: '^6.0.0',
  svelte: '^5.0.0',
  '@sveltejs/vite-plugin-svelte': '^5.0.0',
  '@types/node': '^22.0.0',
}

/**
 * Legacy mode devDependencies (only when enableLegacy is true)
 */
const LEGACY_DEV_DEPENDENCIES = {
  '@vitejs/plugin-legacy': '^6.0.0',
}

/**
 * TypeScript-specific devDependencies
 */
const TYPESCRIPT_DEV_DEPENDENCIES = {
  typescript: '^5.6.0',
  'svelte-check': '^4.0.0',
  '@tsconfig/svelte': '^5.0.0',
}

/**
 * ESLint devDependencies
 */
const ESLINT_DEV_DEPENDENCIES = {
  eslint: '^9.0.0',
  '@sveltium/eslint-rules': '^1.0.0',
  'eslint-config-prettier': '^10.0.0',
}

/**
 * AutoGeneration devDependencies
 */
const AUTO_GENERATION_DEV_DEPENDENCIES = {
  '@sveltium/build-tools': '^0.1.0',
}

/**
 * Build scripts object based on options
 * @param {object} options
 * @param {boolean} options.autoGeneration
 * @returns {object}
 */
function buildScripts(options) {
  const scripts = { ...BASE_SCRIPTS }

  if (options.autoGeneration) {
    Object.assign(scripts, AUTO_GENERATION_SCRIPTS)
  }

  return scripts
}

/**
 * Build devDependencies object based on options
 * @param {object} options
 * @param {'typescript' | 'javascript'} options.language
 * @param {boolean} options.enableLegacy
 * @param {boolean} options.customEslintRules
 * @param {boolean} options.autoGeneration
 * @returns {object}
 */
function buildDevDependencies(options) {
  const deps = { ...BASE_DEV_DEPENDENCIES }

  const isTypeScript = options.language === 'typescript'
  if (isTypeScript) {
    Object.assign(deps, TYPESCRIPT_DEV_DEPENDENCIES)
  }

  if (options.enableLegacy) {
    Object.assign(deps, LEGACY_DEV_DEPENDENCIES)
  }

  if (options.customEslintRules) {
    Object.assign(deps, ESLINT_DEV_DEPENDENCIES)
  }

  if (options.autoGeneration) {
    Object.assign(deps, AUTO_GENERATION_DEV_DEPENDENCIES)
  }

  return deps
}

/**
 * Sort object keys alphabetically
 * @param {object} obj
 * @returns {object}
 */
function sortKeys(obj) {
  const sorted = {}
  const keys = Object.keys(obj).sort()

  for (const key of keys) {
    sorted[key] = obj[key]
  }

  return sorted
}

/**
 * Generate package.json for the scaffolded project
 *
 * @param {string} targetDir - Target directory
 * @param {object} options - Generation options
 * @param {string} options.packageName - Package name
 * @param {'typescript' | 'javascript'} options.language - Selected language
 * @param {boolean} options.enableLegacy - Legacy mode flag (stored in sveltium config)
 * @param {boolean} options.customEslintRules - Include ESLint
 * @param {boolean} options.autoGeneration - Include build tools
 */
export async function generatePackageJson(targetDir, options) {
  const { packageName, language, enableLegacy, customEslintRules, autoGeneration } = options

  const scripts = buildScripts({ autoGeneration })
  const devDependencies = buildDevDependencies({
    language,
    enableLegacy,
    customEslintRules,
    autoGeneration,
  })

  const packageJson = {
    name: packageName,
    version: '0.0.0',
    private: true,
    type: 'module',
    scripts,
    devDependencies: sortKeys(devDependencies),
  }

  const filePath = path.join(targetDir, 'package.json')
  await writeJson(filePath, packageJson)
}

export {
  BASE_SCRIPTS,
  AUTO_GENERATION_SCRIPTS,
  BASE_DEV_DEPENDENCIES,
  LEGACY_DEV_DEPENDENCIES,
  TYPESCRIPT_DEV_DEPENDENCIES,
  ESLINT_DEV_DEPENDENCIES,
  AUTO_GENERATION_DEV_DEPENDENCIES,
  buildScripts,
  buildDevDependencies,
}
