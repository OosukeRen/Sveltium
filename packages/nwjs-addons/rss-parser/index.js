'use strict'

var native = require('../build/Release/nwjs_addons.node')

/**
 * Parse RSS/Atom feed XML string
 * @param {string} xmlString - RSS or Atom XML content
 * @returns {Object} Feed object with title, description, link, language, lastBuildDate, items
 */
function parse(xmlString) {
  if (typeof xmlString !== 'string') {
    throw new TypeError('xmlString must be a string')
  }

  return native.rssParse(xmlString)
}

/**
 * Parse RSS/Atom feed from file
 * @param {string} filePath - Path to RSS/Atom XML file
 * @returns {Object} Feed object with title, description, link, language, lastBuildDate, items
 */
function parseFile(filePath) {
  if (typeof filePath !== 'string') {
    throw new TypeError('filePath must be a string')
  }

  return native.rssParseFile(filePath)
}

module.exports = {
  parse: parse,
  parseFile: parseFile
}
