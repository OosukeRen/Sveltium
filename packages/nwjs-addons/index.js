/**
 * nwjs-addons - Native C++ addons for NW.js applications
 *
 * Built addons (always available):
 * - clipboard: Full clipboard access (text, files, images)
 * - folderDialog: Native folder selection dialog
 * - ipc: Inter-process communication via named pipes
 * - callDll: Dynamic DLL loading and function calling (FFI)
 *
 * Optional addons (require additional setup):
 * - tinycc: Runtime C compilation (requires libtcc)
 * - sqlite3: SQLite3 database (requires sqlite3 amalgamation)
 */

'use strict'

var clipboard = require('./clipboard')
var folderDialog = require('./folder-dialog')
var ipc = require('./ipc')
var callDll = require('./call-dll')
var csvParser = require('./csv-parser')

// Optional addons - may not be available
var tinycc = null
var sqlite3 = null

try {
  tinycc = require('./tinycc')
} catch (e) {
  // tinycc requires libtcc library
}

try {
  sqlite3 = require('./nw-sqlite3')
} catch (e) {
  // nw-sqlite3 requires SQLite amalgamation
}

module.exports = {
  clipboard: clipboard,
  folderDialog: folderDialog,
  ipc: ipc,
  callDll: callDll,
  tinycc: tinycc,
  sqlite3: sqlite3,
  csvParser: csvParser
}
