'use strict'

var native = require('../build/Release/nwjs_addons.node')

var DEFAULT_PARSE_OPTIONS = {
  delimiter: ',',
  quote: '"',
  escape: '"',
  headers: true,
  skipEmptyLines: true,
  trim: false,
  columns: null
}

var DEFAULT_STRINGIFY_OPTIONS = {
  delimiter: ',',
  quote: '"',
  quoteAll: false,
  headers: true,
  lineEnding: '\r\n'
}

function mergeOptions(defaults, provided) {
  var result = {}
  for (var key in defaults) {
    var hasProvided = provided && provided[key] !== undefined
    result[key] = hasProvided ? provided[key] : defaults[key]
  }
  return result
}

function validateDelimiter(value, name) {
  var isValid = typeof value === 'string' && value.length === 1
  if (!isValid) {
    throw new TypeError(name + ' must be a single character string')
  }
}

function rowsToObjects(rows, columns) {
  var result = []

  for (var i = 0; i < rows.length; i++) {
    var row = rows[i]
    var obj = {}

    for (var j = 0; j < columns.length; j++) {
      var hasValue = j < row.length
      obj[columns[j]] = hasValue ? row[j] : ''
    }

    result.push(obj)
  }

  return result
}

function objectsToRows(data, includeHeaders) {
  var rows = []
  var keys = Object.keys(data[0])

  if (includeHeaders) {
    rows.push(keys)
  }

  for (var i = 0; i < data.length; i++) {
    var row = []
    for (var j = 0; j < keys.length; j++) {
      var val = data[i][keys[j]]
      var isNullOrUndefined = val === null || val === undefined
      row.push(isNullOrUndefined ? '' : String(val))
    }
    rows.push(row)
  }

  return rows
}

/**
 * Parse CSV string
 * @param {string} csvString - CSV content to parse
 * @param {Object} [options] - Parse options
 * @returns {Array} Parsed data (array of arrays or array of objects)
 */
function parse(csvString, options) {
  if (typeof csvString !== 'string') {
    throw new TypeError('csvString must be a string')
  }

  var opts = mergeOptions(DEFAULT_PARSE_OPTIONS, options)
  validateDelimiter(opts.delimiter, 'delimiter')
  validateDelimiter(opts.quote, 'quote')
  validateDelimiter(opts.escape, 'escape')

  var nativeOpts = {
    delimiter: opts.delimiter,
    quote: opts.quote,
    escape: opts.escape,
    skipEmptyLines: opts.skipEmptyLines,
    trim: opts.trim
  }

  var rows = native.csvParse(csvString, nativeOpts)

  var useHeaders = opts.headers && rows.length > 0
  var columns = opts.columns || (useHeaders ? rows[0] : null)

  if (!columns) {
    return rows
  }

  var dataRows = useHeaders ? rows.slice(1) : rows
  return rowsToObjects(dataRows, columns)
}

/**
 * Parse CSV file
 * @param {string} filePath - Path to CSV file
 * @param {Object} [options] - Parse options
 * @returns {Array} Parsed data
 */
function parseFile(filePath, options) {
  if (typeof filePath !== 'string') {
    throw new TypeError('filePath must be a string')
  }

  var opts = mergeOptions(DEFAULT_PARSE_OPTIONS, options)
  validateDelimiter(opts.delimiter, 'delimiter')
  validateDelimiter(opts.quote, 'quote')
  validateDelimiter(opts.escape, 'escape')

  var nativeOpts = {
    delimiter: opts.delimiter,
    quote: opts.quote,
    escape: opts.escape,
    skipEmptyLines: opts.skipEmptyLines,
    trim: opts.trim
  }

  var rows = native.csvParseFile(filePath, nativeOpts)

  var useHeaders = opts.headers && rows.length > 0
  var columns = opts.columns || (useHeaders ? rows[0] : null)

  if (!columns) {
    return rows
  }

  var dataRows = useHeaders ? rows.slice(1) : rows
  return rowsToObjects(dataRows, columns)
}

/**
 * Convert array to CSV string
 * @param {Array} data - Array of arrays or array of objects
 * @param {Object} [options] - Stringify options
 * @returns {string} CSV string
 */
function stringify(data, options) {
  if (!Array.isArray(data)) {
    throw new TypeError('data must be an array')
  }

  if (data.length === 0) {
    return ''
  }

  var opts = mergeOptions(DEFAULT_STRINGIFY_OPTIONS, options)
  validateDelimiter(opts.delimiter, 'delimiter')
  validateDelimiter(opts.quote, 'quote')

  var firstItem = data[0]
  var isObjectArray = typeof firstItem === 'object' && !Array.isArray(firstItem)

  var rows = isObjectArray ? objectsToRows(data, opts.headers) : data

  var nativeOpts = {
    delimiter: opts.delimiter,
    quote: opts.quote,
    quoteAll: opts.quoteAll,
    lineEnding: opts.lineEnding
  }

  return native.csvStringify(rows, nativeOpts)
}

/**
 * Write array to CSV file
 * @param {string} filePath - Output file path
 * @param {Array} data - Array of arrays or array of objects
 * @param {Object} [options] - Stringify options
 * @returns {boolean} True if successful
 */
function writeFile(filePath, data, options) {
  if (typeof filePath !== 'string') {
    throw new TypeError('filePath must be a string')
  }

  var csvString = stringify(data, options)
  return native.csvWriteFile(filePath, csvString)
}

module.exports = {
  parse: parse,
  parseFile: parseFile,
  stringify: stringify,
  writeFile: writeFile
}
