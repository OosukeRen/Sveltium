'use strict'

/**
 * Tool Executor
 *
 * Executes MCP tools in the NW.js browser context
 */

var SnapshotBuilder = require('./dom/snapshot-builder')
var ElementFinder = require('./dom/element-finder')
var EventDispatcher = require('./dom/event-dispatcher')
var ConsoleCapture = require('./dom/console-capture')

function ToolExecutor(win) {
  this.win = win
  this.doc = win.document

  // Initialize DOM utilities
  this.snapshot = new SnapshotBuilder(win)
  this.finder = new ElementFinder(win)
  this.events = new EventDispatcher(win)
  this.console = new ConsoleCapture(win)

  // Start console capture
  this.console.start()
}

ToolExecutor.prototype.execute = function(tool, args, callback) {
  var self = this

  try {
    switch (tool) {
      case 'browser_snapshot':
        this._snapshot(args, callback)
        break

      case 'browser_take_screenshot':
        this._screenshot(args, callback)
        break

      case 'browser_click':
        this._click(args, callback)
        break

      case 'browser_type':
        this._type(args, callback)
        break

      case 'browser_evaluate':
        this._evaluate(args, callback)
        break

      case 'browser_navigate':
        this._navigate(args, callback)
        break

      case 'browser_console_messages':
        this._consoleMessages(args, callback)
        break

      case 'browser_resize':
        this._resize(args, callback)
        break

      case 'browser_wait_for':
        this._waitFor(args, callback)
        break

      case 'browser_fill_form':
        this._fillForm(args, callback)
        break

      case 'browser_press_key':
        this._pressKey(args, callback)
        break

      case 'nwjs_reload':
        this._reload(args, callback)
        break

      case 'nwjs_show_devtools':
        this._showDevtools(args, callback)
        break

      case 'nwjs_close':
        this._close(args, callback)
        break

      case 'nwjs_get_manifest':
        this._getManifest(args, callback)
        break

      case 'nwjs_get_argv':
        this._getArgv(args, callback)
        break

      case 'nwjs_minimize':
        this._minimize(args, callback)
        break

      case 'nwjs_maximize':
        this._maximize(args, callback)
        break

      case 'nwjs_restore':
        this._restore(args, callback)
        break

      case 'nwjs_focus':
        this._focus(args, callback)
        break

      case 'nwjs_get_bounds':
        this._getBounds(args, callback)
        break

      case 'nwjs_set_bounds':
        this._setBounds(args, callback)
        break

      case 'nwjs_zoom':
        this._zoom(args, callback)
        break

      default:
        callback(new Error('Unknown tool: ' + tool))
    }
  } catch (err) {
    callback(err)
  }
}

ToolExecutor.prototype._snapshot = function(args, callback) {
  var tree = this.snapshot.build()
  callback(null, {
    content: [{ type: 'text', text: tree }]
  })
}

ToolExecutor.prototype._screenshot = function(args, callback) {
  var self = this

  // Try to get nw.gui
  var nwGui = null
  try {
    nwGui = this.win.require('nw.gui')
  } catch (e) {
    nwGui = this.win.nw
  }

  if (!nwGui || !nwGui.Window) {
    callback(new Error('NW.js window API not available'))
    return
  }

  var nwWin = nwGui.Window.get()

  nwWin.capturePage(function(buffer) {
    var base64 = buffer.toString('base64')
    callback(null, {
      content: [{ type: 'image', data: base64, mimeType: 'image/png' }]
    })
  }, { format: 'png', datatype: 'buffer' })
}

ToolExecutor.prototype._click = function(args, callback) {
  var element = this.finder.findByRef(args.ref)
  if (!element) {
    callback(new Error('Element not found: ' + args.ref))
    return
  }

  var button = args.button || 'left'
  var doubleClick = args.doubleClick || false

  this.events.click(element, button, doubleClick)

  callback(null, {
    content: [{ type: 'text', text: 'Clicked: ' + (args.element || args.ref) }]
  })
}

ToolExecutor.prototype._type = function(args, callback) {
  var element = this.finder.findByRef(args.ref)
  if (!element) {
    callback(new Error('Element not found: ' + args.ref))
    return
  }

  var text = args.text || ''
  var slowly = args.slowly || false
  var submit = args.submit || false

  this.events.type(element, text, slowly, submit)

  callback(null, {
    content: [{ type: 'text', text: 'Typed: ' + text }]
  })
}

ToolExecutor.prototype._evaluate = function(args, callback) {
  var code = args.function
  var element = null

  if (args.ref) {
    element = this.finder.findByRef(args.ref)
    if (!element) {
      callback(new Error('Element not found: ' + args.ref))
      return
    }
  }

  var fn = null
  try {
    fn = this.win.eval('(' + code + ')')
  } catch (e) {
    callback(new Error('Failed to parse function: ' + e.message))
    return
  }

  var result = null
  try {
    if (element) {
      result = fn(element)
    } else {
      result = fn()
    }
  } catch (e) {
    callback(new Error('Function execution failed: ' + e.message))
    return
  }

  // Format result
  var formatted = 'undefined'
  if (result !== undefined) {
    if (result === null) {
      formatted = 'null'
    } else if (typeof result === 'object') {
      try {
        formatted = JSON.stringify(result, null, 2)
      } catch (e) {
        formatted = String(result)
      }
    } else {
      formatted = String(result)
    }
  }

  callback(null, {
    content: [{ type: 'text', text: formatted }]
  })
}

ToolExecutor.prototype._navigate = function(args, callback) {
  this.win.location.href = args.url
  callback(null, {
    content: [{ type: 'text', text: 'Navigating to: ' + args.url }]
  })
}

ToolExecutor.prototype._consoleMessages = function(args, callback) {
  var level = args.level || 'info'
  var messages = this.console.getMessages(level)
  callback(null, {
    content: [{ type: 'text', text: messages.join('\n') || '(no messages)' }]
  })
}

ToolExecutor.prototype._resize = function(args, callback) {
  var nwGui = null
  try {
    nwGui = this.win.require('nw.gui')
  } catch (e) {
    nwGui = this.win.nw
  }

  if (!nwGui || !nwGui.Window) {
    callback(new Error('NW.js window API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  nwWin.resizeTo(args.width, args.height)

  callback(null, {
    content: [{ type: 'text', text: 'Resized to ' + args.width + 'x' + args.height }]
  })
}

ToolExecutor.prototype._waitFor = function(args, callback) {
  var self = this

  if (args.time) {
    setTimeout(function() {
      callback(null, {
        content: [{ type: 'text', text: 'Waited ' + args.time + ' seconds' }]
      })
    }, args.time * 1000)
    return
  }

  var text = args.text
  var textGone = args.textGone
  var timeout = 10000
  var interval = 100
  var elapsed = 0

  var check = function() {
    var bodyText = self.doc.body ? self.doc.body.innerText : ''

    if (text) {
      if (bodyText.indexOf(text) !== -1) {
        callback(null, {
          content: [{ type: 'text', text: 'Found: ' + text }]
        })
        return
      }
    }

    if (textGone) {
      if (bodyText.indexOf(textGone) === -1) {
        callback(null, {
          content: [{ type: 'text', text: 'Gone: ' + textGone }]
        })
        return
      }
    }

    elapsed += interval
    if (elapsed >= timeout) {
      callback(new Error('Timeout waiting for ' + (text || textGone)))
      return
    }

    setTimeout(check, interval)
  }

  check()
}

ToolExecutor.prototype._fillForm = function(args, callback) {
  var self = this
  var fields = args.fields || []
  var filled = []

  for (var i = 0; i < fields.length; i++) {
    var field = fields[i]
    var element = this.finder.findByRef(field.ref)

    if (!element) {
      callback(new Error('Field not found: ' + field.ref))
      return
    }

    if (field.type === 'checkbox' || field.type === 'radio') {
      element.checked = field.value === 'true'
    } else if (field.type === 'combobox') {
      element.value = field.value
      this.events.dispatchInput(element)
    } else {
      element.value = field.value
      this.events.dispatchInput(element)
    }

    filled.push(field.name)
  }

  callback(null, {
    content: [{ type: 'text', text: 'Filled fields: ' + filled.join(', ') }]
  })
}

ToolExecutor.prototype._pressKey = function(args, callback) {
  this.events.pressKey(args.key)
  callback(null, {
    content: [{ type: 'text', text: 'Pressed: ' + args.key }]
  })
}

ToolExecutor.prototype._reload = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()

  // Check if full relaunch is requested (restart the whole app)
  if (args.relaunch) {
    callback(null, {
      content: [{ type: 'text', text: 'Relaunching app...' }]
    })

    // Relaunch using child_process.exec like XPCode does
    setTimeout(function() {
      var exec = require('child_process').exec
      var execPath = process.execPath

      // Quote the path and use "." for current app directory
      var safeExecPath = '"' + execPath + '"'
      var command = safeExecPath + ' .'

      exec(command, { stdio: 'ignore', detached: true }, function() {})

      // Close current window after short delay
      setTimeout(function() {
        nwWin.close()
      }, 30)
    }, 100)
    return
  }

  // Just reload the window
  if (args.ignoreCache) {
    nwWin.reloadDev()
  } else {
    nwWin.reload()
  }

  callback(null, {
    content: [{ type: 'text', text: 'Reloading app' + (args.ignoreCache ? ' (ignoring cache)' : '') }]
  })
}

ToolExecutor.prototype._showDevtools = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  nwWin.showDevTools()

  callback(null, {
    content: [{ type: 'text', text: 'DevTools opened' }]
  })
}

ToolExecutor.prototype._close = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  callback(null, {
    content: [{ type: 'text', text: 'Closing app' }]
  })

  // Close after sending response
  var nwWin = nwGui.Window.get()
  setTimeout(function() {
    nwWin.close()
  }, 100)
}

ToolExecutor.prototype._getNwGui = function() {
  try {
    return this.win.require('nw.gui')
  } catch (e) {
    return this.win.nw
  }
}

ToolExecutor.prototype._getManifest = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui || !nwGui.App) {
    callback(new Error('NW.js API not available'))
    return
  }

  var manifest = nwGui.App.manifest || {}
  var formatted = JSON.stringify(manifest, null, 2)

  callback(null, {
    content: [{ type: 'text', text: formatted }]
  })
}

ToolExecutor.prototype._getArgv = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui || !nwGui.App) {
    callback(new Error('NW.js API not available'))
    return
  }

  var result = {
    argv: nwGui.App.argv || [],
    fullArgv: nwGui.App.fullArgv || [],
    dataPath: nwGui.App.dataPath || ''
  }

  callback(null, {
    content: [{ type: 'text', text: JSON.stringify(result, null, 2) }]
  })
}

ToolExecutor.prototype._minimize = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  nwWin.minimize()

  callback(null, {
    content: [{ type: 'text', text: 'Window minimized' }]
  })
}

ToolExecutor.prototype._maximize = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  nwWin.maximize()

  callback(null, {
    content: [{ type: 'text', text: 'Window maximized' }]
  })
}

ToolExecutor.prototype._restore = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  nwWin.restore()

  callback(null, {
    content: [{ type: 'text', text: 'Window restored' }]
  })
}

ToolExecutor.prototype._focus = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  nwWin.focus()

  callback(null, {
    content: [{ type: 'text', text: 'Window focused' }]
  })
}

ToolExecutor.prototype._getBounds = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  var bounds = {
    x: nwWin.x,
    y: nwWin.y,
    width: nwWin.width,
    height: nwWin.height
  }

  callback(null, {
    content: [{ type: 'text', text: JSON.stringify(bounds, null, 2) }]
  })
}

ToolExecutor.prototype._setBounds = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()

  if (args.x !== undefined && args.y !== undefined) {
    nwWin.moveTo(args.x, args.y)
  }

  if (args.width !== undefined && args.height !== undefined) {
    nwWin.resizeTo(args.width, args.height)
  }

  var newBounds = {
    x: nwWin.x,
    y: nwWin.y,
    width: nwWin.width,
    height: nwWin.height
  }

  callback(null, {
    content: [{ type: 'text', text: 'Bounds updated: ' + JSON.stringify(newBounds) }]
  })
}

ToolExecutor.prototype._zoom = function(args, callback) {
  var nwGui = this._getNwGui()
  if (!nwGui) {
    callback(new Error('NW.js API not available'))
    return
  }

  var nwWin = nwGui.Window.get()
  var level = args.level || 1.0

  // NW.js uses zoomLevel which is logarithmic (0 = 100%, 1 = 120%, -1 = ~83%)
  // Convert linear scale to NW.js zoom level
  // Formula: zoomLevel = log(scale) / log(1.2)
  var zoomLevel = Math.log(level) / Math.log(1.2)
  nwWin.zoomLevel = zoomLevel

  callback(null, {
    content: [{ type: 'text', text: 'Zoom set to ' + (level * 100) + '%' }]
  })
}

module.exports = ToolExecutor
