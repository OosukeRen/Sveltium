import path from 'node:path'
import { readFile, writeFile } from '../utils/file-utils.js'

const PLACEHOLDER_APP_NAME = '[[APP_NAME]]'
const PLACEHOLDER_MAIN = '[[MAIN]]'

const FILES_WITH_PLACEHOLDERS = ['index.html']

/**
 * Get the main entry file name based on language
 * @param {'typescript' | 'javascript'} language
 * @returns {string}
 */
function getMainEntry(language) {
  const isTypeScript = language === 'typescript'
  const mainEntry = isTypeScript ? 'main.ts' : 'main.js'
  return mainEntry
}

/**
 * Replace all placeholders in a string
 * @param {string} content - File content
 * @param {object} replacements - Placeholder to value map
 * @returns {string}
 */
function replaceAllPlaceholders(content, replacements) {
  let result = content

  for (const [placeholder, value] of Object.entries(replacements)) {
    result = result.replaceAll(placeholder, value)
  }

  return result
}

/**
 * Replace placeholders in template files
 *
 * @param {string} targetDir - Target directory containing copied templates
 * @param {object} options - Replacement options
 * @param {string} options.projectName - Project name for [[APP_NAME]]
 * @param {'typescript' | 'javascript'} options.language - Language for [[MAIN]]
 */
export async function replacePlaceholders(targetDir, options) {
  const { projectName, language } = options

  const replacements = {
    [PLACEHOLDER_APP_NAME]: projectName,
    [PLACEHOLDER_MAIN]: getMainEntry(language),
  }

  for (const fileName of FILES_WITH_PLACEHOLDERS) {
    const filePath = path.join(targetDir, fileName)
    const content = await readFile(filePath)
    const updatedContent = replaceAllPlaceholders(content, replacements)
    await writeFile(filePath, updatedContent)
  }
}

export {
  PLACEHOLDER_APP_NAME,
  PLACEHOLDER_MAIN,
  FILES_WITH_PLACEHOLDERS,
  getMainEntry,
}
