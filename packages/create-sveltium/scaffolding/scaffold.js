import path from 'node:path'
import { ensureDir, renameGitignore } from '../utils/file-utils.js'
import { mergeTemplates } from './template-merger.js'
import { replacePlaceholders } from './placeholder-replacer.js'
import { generatePackageJson } from './package-json-generator.js'
import { generateSveltiumConfig } from './config-generator.js'

/**
 * @typedef {Object} ScaffoldOptions
 * @property {string} projectName - Formatted project name
 * @property {string} packageName - Valid npm package name
 * @property {boolean} enableLegacy - Enable legacy browser support
 * @property {'typescript' | 'javascript'} language - Selected language
 * @property {boolean} customEslintRules - Include ESLint config
 * @property {boolean} autoGeneration - Include build tools setup
 */

/**
 * Scaffold a new Sveltium project
 *
 * Steps:
 * 1. Create target directory
 * 2. Merge templates (base -> language -> linting)
 * 3. Rename _gitignore to .gitignore
 * 4. Replace placeholders in template files
 * 5. Generate package.json
 * 6. Generate sveltium.config.js (if autoGeneration)
 *
 * @param {ScaffoldOptions} options - User responses from questions
 * @returns {Promise<string>} - Path to created project
 */
export async function scaffold(options) {
  const {
    projectName,
    packageName,
    enableLegacy,
    language,
    customEslintRules,
    autoGeneration,
  } = options

  const targetDir = path.resolve(process.cwd(), projectName)

  // Step 1: Create target directory
  await ensureDir(targetDir)

  // Step 2: Merge templates in layered order
  await mergeTemplates(targetDir, {
    language,
    customEslintRules,
  })

  // Step 3: Rename _gitignore to .gitignore
  await renameGitignore(targetDir)

  // Step 4: Replace placeholders in template files
  await replacePlaceholders(targetDir, {
    projectName,
    language,
  })

  // Step 5: Generate package.json
  await generatePackageJson(targetDir, {
    packageName,
    language,
    enableLegacy,
    customEslintRules,
    autoGeneration,
  })

  // Step 6: Generate sveltium.config.js (if autoGeneration enabled)
  if (autoGeneration) {
    await generateSveltiumConfig(targetDir, {
      enableLegacy,
    })
  }

  return targetDir
}
