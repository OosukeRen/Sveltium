'use strict'

/**
 * Console Capture
 *
 * Captures console messages for retrieval
 */

function ConsoleCapture(win) {
  this.win = win
  this.messages = []
  this.maxMessages = 1000
  this.originalConsole = {}
  this.started = false
}

/**
 * Start capturing console messages
 */
ConsoleCapture.prototype.start = function() {
  if (this.started) {
    return
  }

  var self = this
  var console = this.win.console

  // Store original methods
  this.originalConsole.log = console.log
  this.originalConsole.info = console.info
  this.originalConsole.warn = console.warn
  this.originalConsole.error = console.error
  this.originalConsole.debug = console.debug

  // Override console methods
  console.log = function() {
    self._capture('info', arguments)
    self.originalConsole.log.apply(console, arguments)
  }

  console.info = function() {
    self._capture('info', arguments)
    self.originalConsole.info.apply(console, arguments)
  }

  console.warn = function() {
    self._capture('warning', arguments)
    self.originalConsole.warn.apply(console, arguments)
  }

  console.error = function() {
    self._capture('error', arguments)
    self.originalConsole.error.apply(console, arguments)
  }

  console.debug = function() {
    self._capture('debug', arguments)
    self.originalConsole.debug.apply(console, arguments)
  }

  this.started = true
}

/**
 * Stop capturing console messages
 */
ConsoleCapture.prototype.stop = function() {
  if (!this.started) {
    return
  }

  var console = this.win.console

  // Restore original methods
  console.log = this.originalConsole.log
  console.info = this.originalConsole.info
  console.warn = this.originalConsole.warn
  console.error = this.originalConsole.error
  console.debug = this.originalConsole.debug

  this.started = false
}

/**
 * Capture a console message
 */
ConsoleCapture.prototype._capture = function(level, args) {
  var message = ''

  for (var i = 0; i < args.length; i++) {
    if (i > 0) message += ' '

    var arg = args[i]
    if (typeof arg === 'object') {
      try {
        message += JSON.stringify(arg)
      } catch (e) {
        message += String(arg)
      }
    } else {
      message += String(arg)
    }
  }

  this.messages.push({
    level: level,
    message: message,
    timestamp: Date.now()
  })

  // Trim old messages
  if (this.messages.length > this.maxMessages) {
    this.messages = this.messages.slice(-this.maxMessages)
  }
}

/**
 * Get messages at or above a given level
 */
ConsoleCapture.prototype.getMessages = function(minLevel) {
  var levels = ['debug', 'info', 'warning', 'error']
  var minIndex = levels.indexOf(minLevel || 'info')

  var result = []
  for (var i = 0; i < this.messages.length; i++) {
    var msg = this.messages[i]
    var msgIndex = levels.indexOf(msg.level)
    if (msgIndex >= minIndex) {
      result.push('[' + msg.level.toUpperCase() + '] ' + msg.message)
    }
  }

  return result
}

/**
 * Clear all captured messages
 */
ConsoleCapture.prototype.clear = function() {
  this.messages = []
}

module.exports = ConsoleCapture
