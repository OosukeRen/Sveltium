'use strict'

/**
 * DOM Snapshot Builder
 *
 * Creates accessibility tree snapshot with element refs
 */

var ElementFinder = require('./element-finder')

function SnapshotBuilder(win) {
  this.win = win
  this.doc = win.document
  this.refCounter = 0
  this.refMap = {}
}

SnapshotBuilder.prototype.build = function() {
  this.refCounter = 0
  this.refMap = {}

  var lines = []
  var title = this.doc.title || 'Untitled'
  lines.push('- page "' + title + '" [ref=page]')

  if (this.doc.body) {
    this._buildNode(this.doc.body, 1, lines)
  }

  // Update global ref map for ElementFinder
  ElementFinder.setRefMap(this.refMap)

  return lines.join('\n')
}

SnapshotBuilder.prototype.getRefMap = function() {
  return this.refMap
}

SnapshotBuilder.prototype._buildNode = function(node, depth, lines) {
  if (node.nodeType !== 1) {
    return
  }

  // Skip hidden elements
  if (!this._isVisible(node)) {
    return
  }

  // Skip script, style, etc.
  var tagName = node.tagName.toLowerCase()
  if (tagName === 'script' || tagName === 'style' || tagName === 'noscript') {
    return
  }

  var role = this._getRole(node)
  var name = this._getName(node)
  var ref = 'e' + (++this.refCounter)

  this.refMap[ref] = node

  var indent = ''
  for (var i = 0; i < depth; i++) {
    indent += '  '
  }

  var line = indent + '- ' + role
  if (name) {
    line += ' "' + name + '"'
  }

  // Add attributes
  var attrs = this._getAttributes(node)
  if (attrs) {
    line += ' ' + attrs
  }

  line += ' [ref=' + ref + ']'
  lines.push(line)

  // Process children
  var children = node.children
  for (var j = 0; j < children.length; j++) {
    this._buildNode(children[j], depth + 1, lines)
  }
}

SnapshotBuilder.prototype._isVisible = function(node) {
  var style = this.win.getComputedStyle(node)
  if (style.display === 'none' || style.visibility === 'hidden') {
    return false
  }
  return true
}

SnapshotBuilder.prototype._getRole = function(node) {
  var tagName = node.tagName.toLowerCase()
  var role = node.getAttribute('role')

  if (role) {
    return role
  }

  var roleMap = {
    'button': 'button',
    'a': 'link',
    'input': this._getInputRole(node),
    'textarea': 'textbox',
    'select': 'combobox',
    'option': 'option',
    'h1': 'heading',
    'h2': 'heading',
    'h3': 'heading',
    'h4': 'heading',
    'h5': 'heading',
    'h6': 'heading',
    'ul': 'list',
    'ol': 'list',
    'li': 'listitem',
    'table': 'table',
    'tr': 'row',
    'td': 'cell',
    'th': 'columnheader',
    'img': 'image',
    'nav': 'navigation',
    'main': 'main',
    'header': 'banner',
    'footer': 'contentinfo',
    'aside': 'complementary',
    'form': 'form',
    'section': 'region',
    'article': 'article'
  }

  return roleMap[tagName] || 'generic'
}

SnapshotBuilder.prototype._getInputRole = function(node) {
  var type = (node.getAttribute('type') || 'text').toLowerCase()

  var typeRoleMap = {
    'text': 'textbox',
    'password': 'textbox',
    'email': 'textbox',
    'number': 'spinbutton',
    'checkbox': 'checkbox',
    'radio': 'radio',
    'button': 'button',
    'submit': 'button',
    'reset': 'button',
    'range': 'slider',
    'search': 'searchbox'
  }

  return typeRoleMap[type] || 'textbox'
}

SnapshotBuilder.prototype._getName = function(node) {
  // aria-label
  var ariaLabel = node.getAttribute('aria-label')
  if (ariaLabel) {
    return ariaLabel
  }

  // For inputs, use label or placeholder
  var tagName = node.tagName.toLowerCase()
  if (tagName === 'input' || tagName === 'textarea' || tagName === 'select') {
    var id = node.getAttribute('id')
    if (id) {
      var label = this.doc.querySelector('label[for="' + id + '"]')
      if (label) {
        return label.textContent.trim()
      }
    }
    var placeholder = node.getAttribute('placeholder')
    if (placeholder) {
      return placeholder
    }
  }

  // For buttons, links, headings - use text content
  if (tagName === 'button' || tagName === 'a' || tagName.match(/^h[1-6]$/)) {
    return node.textContent.trim().substring(0, 50)
  }

  // For images
  if (tagName === 'img') {
    return node.getAttribute('alt') || ''
  }

  // For list items with simple text
  if (tagName === 'li') {
    var text = node.textContent.trim()
    if (text.length < 50 && node.children.length === 0) {
      return text
    }
  }

  return ''
}

SnapshotBuilder.prototype._getAttributes = function(node) {
  var attrs = []
  var tagName = node.tagName.toLowerCase()

  if (tagName === 'input') {
    var placeholder = node.getAttribute('placeholder')
    if (placeholder) {
      attrs.push('[placeholder="' + placeholder + '"]')
    }
    if (node.disabled) {
      attrs.push('[disabled]')
    }
    if (node.checked) {
      attrs.push('[checked]')
    }
  }

  if (tagName === 'a') {
    var href = node.getAttribute('href')
    if (href && href.length < 50) {
      attrs.push('[href="' + href + '"]')
    }
  }

  return attrs.join(' ')
}

module.exports = SnapshotBuilder
