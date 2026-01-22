'use strict'

/**
 * Event Dispatcher
 *
 * Dispatches DOM events to elements
 */

function EventDispatcher(win) {
  this.win = win
  this.doc = win.document
}

/**
 * Click an element
 */
EventDispatcher.prototype.click = function(element, button, doubleClick) {
  var buttonCode = 0
  if (button === 'right') buttonCode = 2
  if (button === 'middle') buttonCode = 1

  // Focus the element if focusable
  if (element.focus) {
    element.focus()
  }

  // Get element center
  var rect = element.getBoundingClientRect()
  var x = rect.left + rect.width / 2
  var y = rect.top + rect.height / 2

  // Dispatch mouse events
  this._dispatchMouse(element, 'mousedown', x, y, buttonCode)
  this._dispatchMouse(element, 'mouseup', x, y, buttonCode)
  this._dispatchMouse(element, 'click', x, y, buttonCode)

  if (doubleClick) {
    this._dispatchMouse(element, 'mousedown', x, y, buttonCode)
    this._dispatchMouse(element, 'mouseup', x, y, buttonCode)
    this._dispatchMouse(element, 'click', x, y, buttonCode)
    this._dispatchMouse(element, 'dblclick', x, y, buttonCode)
  }
}

/**
 * Type text into an element
 */
EventDispatcher.prototype.type = function(element, text, slowly, submit) {
  // Focus the element
  if (element.focus) {
    element.focus()
  }

  if (slowly) {
    // Type character by character
    for (var i = 0; i < text.length; i++) {
      var char = text[i]
      this._dispatchKey(element, 'keydown', char)
      this._dispatchKey(element, 'keypress', char)

      // Update value
      if (element.value !== undefined) {
        element.value += char
      }

      this._dispatchKey(element, 'keyup', char)
      this.dispatchInput(element)
    }
  } else {
    // Set value directly
    if (element.value !== undefined) {
      element.value = text
    } else if (element.contentEditable === 'true') {
      element.textContent = text
    }
    this.dispatchInput(element)
  }

  if (submit) {
    this.pressKey('Enter', element)
  }
}

/**
 * Press a key
 */
EventDispatcher.prototype.pressKey = function(key, target) {
  target = target || this.doc.activeElement || this.doc.body

  this._dispatchKey(target, 'keydown', key)
  this._dispatchKey(target, 'keypress', key)
  this._dispatchKey(target, 'keyup', key)

  // Handle Enter key submitting forms
  if (key === 'Enter') {
    var form = target.form || target.closest('form')
    if (form) {
      var submitEvent = new this.win.Event('submit', {
        bubbles: true,
        cancelable: true
      })
      form.dispatchEvent(submitEvent)
    }
  }
}

/**
 * Dispatch input event
 */
EventDispatcher.prototype.dispatchInput = function(element) {
  var inputEvent = new this.win.Event('input', {
    bubbles: true,
    cancelable: true
  })
  element.dispatchEvent(inputEvent)

  var changeEvent = new this.win.Event('change', {
    bubbles: true,
    cancelable: true
  })
  element.dispatchEvent(changeEvent)
}

/**
 * Dispatch mouse event
 */
EventDispatcher.prototype._dispatchMouse = function(element, type, x, y, button) {
  var event = new this.win.MouseEvent(type, {
    view: this.win,
    bubbles: true,
    cancelable: true,
    clientX: x,
    clientY: y,
    button: button
  })
  element.dispatchEvent(event)
}

/**
 * Dispatch keyboard event
 */
EventDispatcher.prototype._dispatchKey = function(element, type, key) {
  var keyCode = key.length === 1 ? key.charCodeAt(0) : this._getKeyCode(key)

  var event = new this.win.KeyboardEvent(type, {
    view: this.win,
    bubbles: true,
    cancelable: true,
    key: key,
    code: key.length === 1 ? 'Key' + key.toUpperCase() : key,
    keyCode: keyCode,
    which: keyCode
  })
  element.dispatchEvent(event)
}

/**
 * Get key code for special keys
 */
EventDispatcher.prototype._getKeyCode = function(key) {
  var codes = {
    'Enter': 13,
    'Tab': 9,
    'Escape': 27,
    'Backspace': 8,
    'Delete': 46,
    'ArrowUp': 38,
    'ArrowDown': 40,
    'ArrowLeft': 37,
    'ArrowRight': 39,
    'Home': 36,
    'End': 35,
    'PageUp': 33,
    'PageDown': 34,
    'Space': 32
  }
  return codes[key] || 0
}

module.exports = EventDispatcher
