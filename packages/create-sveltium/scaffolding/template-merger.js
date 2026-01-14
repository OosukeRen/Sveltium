import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { copyDir } from '../utils/file-utils.js'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const TEMPLATES_DIR = path.resolve(__dirname, '../../../templates')

const TEMPLATE_BASE = 'base'
const TEMPLATE_TYPESCRIPT = 'sveltium-project-ts'
const TEMPLATE_JAVASCRIPT = 'svelitum-project-js'
const TEMPLATE_LINTING = 'linting'

const LANGUAGE_TEMPLATE_MAP = {
  typescript: TEMPLATE_TYPESCRIPT,
  javascript: TEMPLATE_JAVASCRIPT,
}

/**
 * Merge templates in layered order based on user options
 * Order: base -> language-specific -> linting (if enabled)
 *
 * @param {string} targetDir - Target directory to copy templates into
 * @param {object} options - Merge options
 * @param {'typescript' | 'javascript'} options.language - Selected language
 * @param {boolean} options.customEslintRules - Whether to include ESLint config
 */
export async function mergeTemplates(targetDir, options) {
  const { language, customEslintRules } = options

  // Layer 1: Copy base template
  const basePath = path.join(TEMPLATES_DIR, TEMPLATE_BASE)
  await copyDir(basePath, targetDir)

  // Layer 2: Overlay language-specific template
  const languageTemplate = LANGUAGE_TEMPLATE_MAP[language]
  const languagePath = path.join(TEMPLATES_DIR, languageTemplate)
  await copyDir(languagePath, targetDir)

  // Layer 3: Overlay linting template if enabled
  if (customEslintRules) {
    const lintingPath = path.join(TEMPLATES_DIR, TEMPLATE_LINTING)
    await copyDir(lintingPath, targetDir)
  }
}

export {
  TEMPLATES_DIR,
  TEMPLATE_BASE,
  TEMPLATE_TYPESCRIPT,
  TEMPLATE_JAVASCRIPT,
  TEMPLATE_LINTING,
  LANGUAGE_TEMPLATE_MAP,
}
