/**
 * Clipboard addon - Full clipboard access for Windows
 * Supports text, files, and images
 */

'use strict'

var native = require('../build/Release/nwjs_addons.node')

/**
 * Get the current clipboard content type
 * @returns {'empty'|'text'|'files'|'image'|'unknown'}
 */
function getType() {
  return native.clipboardGetType()
}

/**
 * Check if clipboard contains text
 * @returns {boolean}
 */
function hasText() {
  return native.clipboardHasText()
}

/**
 * Check if clipboard contains files
 * @returns {boolean}
 */
function hasFiles() {
  return native.clipboardHasFiles()
}

/**
 * Check if clipboard contains an image
 * @returns {boolean}
 */
function hasImage() {
  return native.clipboardHasImage()
}

/**
 * Get text from clipboard
 * @returns {string|null} The text content, or null if no text
 */
function getText() {
  return native.clipboardGetText()
}

/**
 * Set/copy text to clipboard
 * @param {string} text - The text to copy
 * @returns {boolean} True if successful
 */
function setText(text) {
  if (typeof text !== 'string') {
    throw new TypeError('text must be a string')
  }
  return native.clipboardCopyText(text)
}

/**
 * Get file paths from clipboard
 * @returns {string[]} Array of file paths
 */
function getFiles() {
  return native.clipboardGetFiles()
}

/**
 * Copy files to clipboard (for paste operation)
 * @param {string[]} paths - Array of file paths
 * @returns {boolean} True if successful
 */
function copyFiles(paths) {
  if (!Array.isArray(paths)) {
    throw new TypeError('paths must be an array')
  }
  return native.clipboardCopyFiles(paths)
}

/**
 * Cut files to clipboard (for move operation)
 * @param {string[]} paths - Array of file paths
 * @returns {boolean} True if successful
 */
function cutFiles(paths) {
  if (!Array.isArray(paths)) {
    throw new TypeError('paths must be an array')
  }
  return native.clipboardCutFiles(paths)
}

/**
 * Paste files from clipboard to destination directory
 * @param {string} destDir - Destination directory path
 * @returns {string[]} Array of new file paths
 */
function pasteFiles(destDir) {
  if (typeof destDir !== 'string') {
    throw new TypeError('destDir must be a string')
  }
  return native.clipboardPasteFiles(destDir)
}

/**
 * Check if clipboard files are marked for cut (move) operation
 * @returns {boolean} True if cut operation, false if copy
 */
function isCutOperation() {
  return native.clipboardIsCutOperation()
}

/**
 * Get dimensions of image in clipboard
 * @returns {{width: number, height: number}|null}
 */
function getImageSize() {
  return native.clipboardGetImageSize()
}

/**
 * Save clipboard image to file
 * @param {string} filePath - Output file path (supports .png, .jpg, .bmp, .gif)
 * @returns {boolean} True if successful
 */
function saveImageToFile(filePath) {
  if (typeof filePath !== 'string') {
    throw new TypeError('filePath must be a string')
  }
  return native.clipboardSaveImageToFile(filePath)
}

/**
 * Get available clipboard formats
 * @returns {Object} Format availability info
 */
function getFormats() {
  return {
    text: hasText(),
    files: hasFiles(),
    image: hasImage()
  }
}

/**
 * Clear the clipboard
 * @returns {boolean} True if successful
 */
function clear() {
  return native.clipboardClear()
}

module.exports = {
  getType: getType,
  hasText: hasText,
  hasFiles: hasFiles,
  hasImage: hasImage,
  getText: getText,
  setText: setText,
  getFiles: getFiles,
  copyFiles: copyFiles,
  cutFiles: cutFiles,
  pasteFiles: pasteFiles,
  isCutOperation: isCutOperation,
  getImageSize: getImageSize,
  saveImageToFile: saveImageToFile,
  getFormats: getFormats,
  clear: clear
}
