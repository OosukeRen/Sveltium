import fs from 'fs-extra'
import path from 'node:path'

/**
 * Copy directory contents recursively, merging with existing files
 * @param {string} src - Source directory
 * @param {string} dest - Destination directory
 */
export async function copyDir(src, dest) {
  await fs.copy(src, dest, { overwrite: true })
}

/**
 * Rename _gitignore to .gitignore in target directory
 * (npm renames .gitignore to .npmignore on publish, so templates use _gitignore)
 * @param {string} dir - Target directory
 */
export async function renameGitignore(dir) {
  const gitignorePath = path.join(dir, '_gitignore')
  const targetPath = path.join(dir, '.gitignore')

  const exists = await fs.pathExists(gitignorePath)
  if (exists) {
    await fs.rename(gitignorePath, targetPath)
  }
}

/**
 * Ensure directory exists, create if not
 * @param {string} dir - Directory path
 */
export async function ensureDir(dir) {
  await fs.ensureDir(dir)
}

/**
 * Check if directory exists and is not empty
 * @param {string} dir - Directory path
 * @returns {Promise<boolean>}
 */
export async function dirHasContents(dir) {
  const exists = await fs.pathExists(dir)
  if (!exists) {
    return false
  }

  const contents = await fs.readdir(dir)
  return contents.length > 0
}

/**
 * Read file contents as string
 * @param {string} filePath - File path
 * @returns {Promise<string>}
 */
export async function readFile(filePath) {
  return fs.readFile(filePath, 'utf-8')
}

/**
 * Write string contents to file
 * @param {string} filePath - File path
 * @param {string} content - Content to write
 */
export async function writeFile(filePath, content) {
  await fs.writeFile(filePath, content, 'utf-8')
}

/**
 * Write JSON object to file with formatting
 * @param {string} filePath - File path
 * @param {object} data - JSON data
 */
export async function writeJson(filePath, data) {
  await fs.writeJson(filePath, data, { spaces: 2 })
}
