/**
 * nw-sqlite3 - Synchronous SQLite3 Database
 *
 * A clean, synchronous SQLite3 API for NW.js applications.
 * Inspired by better-sqlite3 but designed for Windows XP compatibility.
 *
 * IMPORTANT: This module requires SQLite3 amalgamation.
 * Download from: https://sqlite.org/download.html
 * See nw-sqlite3/src/sqlite3.h for setup instructions.
 */

'use strict'

var native = null
var loadError = null

try {
  native = require('../build/Release/nwjs_addons.node').sqlite3
} catch (err) {
  loadError = err
}

/**
 * Check if sqlite3 addon is available
 * @returns {boolean}
 */
function isAvailable() {
  return native !== null && native.Database !== undefined
}

/**
 * Get the error that occurred during loading (if any)
 * @returns {Error|null}
 */
function getLoadError() {
  return loadError
}

/**
 * Database class
 * @param {string} path - Path to database file (or ':memory:' for in-memory)
 * @param {Object} [options] - Options
 * @param {boolean} [options.readonly=false] - Open in read-only mode
 */
function Database(path, options) {
  if (!native || !native.Database) {
    throw new Error('SQLite3 addon not available: ' + (loadError ? loadError.message : 'unknown error'))
  }

  options = options || {}
  this._native = new native.Database(path, options)
}

/**
 * Check if database is open
 * @returns {boolean}
 */
Object.defineProperty(Database.prototype, 'open', {
  get: function() {
    return this._native.open
  }
})

/**
 * Get database path
 * @returns {string}
 */
Object.defineProperty(Database.prototype, 'path', {
  get: function() {
    return this._native.path
  }
})

/**
 * Check if currently in a transaction
 * @returns {boolean}
 */
Object.defineProperty(Database.prototype, 'inTransaction', {
  get: function() {
    return this._native.inTransaction
  }
})

/**
 * Execute SQL that doesn't return results
 * @param {string} sql - SQL statement(s)
 * @returns {Database} this for chaining
 */
Database.prototype.exec = function(sql) {
  this._native.exec(sql)
  return this
}

/**
 * Prepare a SQL statement
 * @param {string} sql - SQL statement
 * @returns {Statement}
 */
Database.prototype.prepare = function(sql) {
  return new Statement(this._native.prepare(sql))
}

/**
 * Execute PRAGMA statement
 * @param {string} pragma - Pragma command (without 'PRAGMA' prefix)
 * @param {Object} [options] - Options
 * @param {boolean} [options.simple=false] - Return single value instead of array
 * @returns {*}
 */
Database.prototype.pragma = function(pragma, options) {
  var stmt = this.prepare('PRAGMA ' + pragma)
  options = options || {}

  if (options.simple) {
    var row = stmt.get()
    if (row) {
      // Return first column value
      var keys = Object.keys(row)
      return keys.length > 0 ? row[keys[0]] : undefined
    }
    return undefined
  }

  return stmt.all()
}

/**
 * Create a transaction wrapper function
 * @param {Function} fn - Function to wrap in transaction
 * @returns {Function} Wrapped function that runs in transaction
 */
Database.prototype.transaction = function(fn) {
  var db = this

  return function transactionWrapper() {
    var args = arguments

    db.exec('BEGIN TRANSACTION')
    try {
      var result = fn.apply(null, args)
      db.exec('COMMIT')
      return result
    } catch (err) {
      db.exec('ROLLBACK')
      throw err
    }
  }
}

/**
 * Close the database
 */
Database.prototype.close = function() {
  this._native.close()
}

/**
 * Statement class
 */
function Statement(nativeStatement) {
  this._native = nativeStatement
}

/**
 * Get SQL source
 * @returns {string}
 */
Object.defineProperty(Statement.prototype, 'source', {
  get: function() {
    return this._native.source
  }
})

/**
 * Check if statement returns rows
 * @returns {boolean}
 */
Object.defineProperty(Statement.prototype, 'reader', {
  get: function() {
    return this._native.reader
  }
})

/**
 * Execute statement (INSERT, UPDATE, DELETE)
 * @param {...*} params - Bind parameters
 * @returns {{changes: number, lastInsertRowid: number}}
 */
Statement.prototype.run = function() {
  return this._native.run.apply(this._native, arguments)
}

/**
 * Get first row
 * @param {...*} params - Bind parameters
 * @returns {Object|undefined}
 */
Statement.prototype.get = function() {
  return this._native.get.apply(this._native, arguments)
}

/**
 * Get all rows
 * @param {...*} params - Bind parameters
 * @returns {Array<Object>}
 */
Statement.prototype.all = function() {
  return this._native.all.apply(this._native, arguments)
}

/**
 * Reset statement for re-execution
 * @returns {Statement}
 */
Statement.prototype.reset = function() {
  this._native.reset()
  return this
}

/**
 * Finalize statement (release resources)
 */
Statement.prototype.finalize = function() {
  this._native.finalize()
}

// Exports
module.exports = Database
module.exports.Database = Database
module.exports.Statement = Statement
module.exports.isAvailable = isAvailable
module.exports.getLoadError = getLoadError
