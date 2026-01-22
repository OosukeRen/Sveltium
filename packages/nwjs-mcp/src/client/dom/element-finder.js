'use strict'

/**
 * Element Finder
 *
 * Finds elements by ref (populated by SnapshotBuilder)
 */

// Global ref map (shared with SnapshotBuilder instances)
var _globalRefMap = {}

function ElementFinder(win) {
  this.win = win
  this.doc = win.document
}

/**
 * Set the global ref map (called by SnapshotBuilder after building)
 */
ElementFinder.setRefMap = function(refMap) {
  _globalRefMap = refMap
}

/**
 * Find element by ref
 */
ElementFinder.prototype.findByRef = function(ref) {
  if (ref === 'page') {
    return this.doc.body
  }
  return _globalRefMap[ref] || null
}

/**
 * Get bounding box of element
 */
ElementFinder.prototype.getBounds = function(element) {
  var rect = element.getBoundingClientRect()
  return {
    x: rect.left,
    y: rect.top,
    width: rect.width,
    height: rect.height
  }
}

/**
 * Check if element is visible
 */
ElementFinder.prototype.isVisible = function(element) {
  var style = this.win.getComputedStyle(element)
  if (style.display === 'none' || style.visibility === 'hidden') {
    return false
  }
  var rect = element.getBoundingClientRect()
  return rect.width > 0 && rect.height > 0
}

module.exports = ElementFinder
