/**
 * Folder/File Dialog addon - Native Windows dialogs
 * Supports both modern (Vista+) and legacy (XP) dialogs
 */

'use strict'

var native = require('../build/Release/nwjs_addons.node')

/**
 * Open native folder selection dialog
 * @param {Object} [options] - Dialog options
 * @param {string} [options.title] - Dialog window title
 * @param {string} [options.initialPath] - Starting directory path
 * @returns {string|null} Selected folder path, or null if cancelled
 */
function open(options) {
  options = options || {}

  var opts = {
    title: options.title || '',
    initialPath: options.initialPath || ''
  }

  if (opts.title && typeof opts.title !== 'string') {
    throw new TypeError('title must be a string')
  }

  if (opts.initialPath && typeof opts.initialPath !== 'string') {
    throw new TypeError('initialPath must be a string')
  }

  return native.folderDialogOpen(opts)
}

/**
 * Open folder dialog with promise (same as open, for consistency)
 * Note: The dialog is blocking, so this doesn't provide true async behavior
 * @param {Object} [options] - Dialog options
 * @returns {Promise<string|null>}
 */
function openAsync(options) {
  return new Promise(function(resolve) {
    var result = open(options)
    resolve(result)
  })
}

/**
 * Open native file selection dialog
 * @param {Object} [options] - Dialog options
 * @param {string} [options.title] - Dialog window title
 * @param {string} [options.initialPath] - Starting directory path
 * @param {string[]} [options.filters] - File filters as pairs: ['Text Files', '*.txt', 'All Files', '*.*']
 * @param {boolean} [options.multiSelect] - Allow selecting multiple files
 * @returns {string|string[]|null} Selected file path(s), or null if cancelled
 */
function openFile(options) {
  options = options || {}

  var opts = {
    title: options.title || '',
    initialPath: options.initialPath || '',
    filters: options.filters || [],
    multiSelect: options.multiSelect || false
  }

  return native.fileDialogOpen(opts)
}

/**
 * Open file dialog with promise
 * @param {Object} [options] - Dialog options
 * @returns {Promise<string|string[]|null>}
 */
function openFileAsync(options) {
  return new Promise(function(resolve) {
    var result = openFile(options)
    resolve(result)
  })
}

/**
 * Open native file save dialog
 * @param {Object} [options] - Dialog options
 * @param {string} [options.title] - Dialog window title
 * @param {string} [options.initialPath] - Starting directory path
 * @param {string} [options.defaultName] - Default filename
 * @param {string[]} [options.filters] - File filters as pairs: ['Text Files', '*.txt', 'All Files', '*.*']
 * @returns {string|null} Selected file path, or null if cancelled
 */
function saveFile(options) {
  options = options || {}

  var opts = {
    title: options.title || '',
    initialPath: options.initialPath || '',
    defaultName: options.defaultName || '',
    filters: options.filters || []
  }

  return native.fileDialogSave(opts)
}

/**
 * Save file dialog with promise
 * @param {Object} [options] - Dialog options
 * @returns {Promise<string|null>}
 */
function saveFileAsync(options) {
  return new Promise(function(resolve) {
    var result = saveFile(options)
    resolve(result)
  })
}

module.exports = {
  open: open,
  openAsync: openAsync,
  openFile: openFile,
  openFileAsync: openFileAsync,
  saveFile: saveFile,
  saveFileAsync: saveFileAsync
}
