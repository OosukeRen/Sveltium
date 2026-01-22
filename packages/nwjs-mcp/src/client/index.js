'use strict'

/**
 * NW.js MCP Client
 *
 * Include this in your NW.js app to enable MCP testing.
 * The MCP server is started by Claude Code via .mcp.json configuration.
 *
 * Usage:
 *   var mcp = require('@sveltium/nwjs-mcp')
 *   mcp.init({
 *     name: 'My App',
 *     port: 3940
 *   })
 */

var ToolExecutor = require('./tool-executor')

var DEFAULT_PORT = 3940
var RECONNECT_DELAY = 2000

var _ws = null
var _appId = null
var _config = {}
var _executor = null
var _reconnectTimer = null
var _connected = false
var _win = null

/**
 * Initialize MCP client - connects to the MCP server
 * @param {Object} options
 * @param {string} options.name - App name (for identification)
 * @param {string} options.appId - Unique app ID (auto-generated if not provided)
 * @param {number} options.port - WebSocket port (default: 3940)
 * @param {Window} options.window - Browser window reference (auto-detected if not provided)
 * @param {boolean} options.autoReconnect - Auto-reconnect on disconnect (default: true)
 * @param {boolean} options.enabled - Enable MCP (default: true, use false or --no-mcp to disable)
 */
function init(options) {
  _config = options || {}
  _config.port = _config.port || DEFAULT_PORT
  _config.autoReconnect = _config.autoReconnect !== false
  _config.enabled = _config.enabled !== false

  // Check for --no-mcp flag to disable
  var nwGui = _getNwGui()
  if (nwGui && nwGui.App && nwGui.App.argv) {
    var argv = nwGui.App.argv
    for (var i = 0; i < argv.length; i++) {
      if (argv[i] === '--no-mcp') {
        _config.enabled = false
        break
      }
    }
  }

  if (!_config.enabled) {
    console.log('[nwjs-mcp] Disabled via config or --no-mcp flag')
    return
  }

  // Get window reference
  _win = _config.window || (typeof window !== 'undefined' ? window : null)
  if (!_win) {
    console.error('[nwjs-mcp] No window reference available. Pass { window: window } to init().')
    return
  }

  // Initialize tool executor
  _executor = new ToolExecutor(_win)

  // Connect to server
  _connect()
}

function _connect() {
  var WS = _win.WebSocket
  if (_ws && (_ws.readyState === WS.CONNECTING || _ws.readyState === WS.OPEN)) {
    return
  }

  var url = 'ws://localhost:' + _config.port

  try {
    _ws = new WS(url)
  } catch (e) {
    console.error('[nwjs-mcp] Failed to create WebSocket:', e.message)
    _scheduleReconnect()
    return
  }

  _ws.onopen = function() {
    console.log('[nwjs-mcp] Connected to MCP server')
    _connected = true

    // Register app
    _ws.send(JSON.stringify({
      type: 'register',
      appId: _config.appId || _appId,
      name: _config.name || ''
    }))
  }

  _ws.onmessage = function(event) {
    var message = null
    try {
      message = JSON.parse(event.data)
    } catch (e) {
      console.error('[nwjs-mcp] Invalid message:', e.message)
      return
    }

    _handleMessage(message)
  }

  _ws.onclose = function() {
    console.log('[nwjs-mcp] Disconnected from MCP server')
    _connected = false
    _ws = null
    _scheduleReconnect()
  }

  _ws.onerror = function() {
    // Error will be followed by close, reconnect handled there
  }
}

function _scheduleReconnect() {
  if (!_config.autoReconnect) {
    return
  }

  if (_reconnectTimer) {
    clearTimeout(_reconnectTimer)
  }

  _reconnectTimer = setTimeout(function() {
    _reconnectTimer = null
    console.log('[nwjs-mcp] Attempting to reconnect...')
    _connect()
  }, RECONNECT_DELAY)
}

function _handleMessage(message) {
  if (message.type === 'registered') {
    _appId = message.appId
    console.log('[nwjs-mcp] Registered as:', _appId)
    return
  }

  if (message.type === 'toolCall') {
    _handleToolCall(message)
    return
  }
}

function _handleToolCall(message) {
  var callId = message.callId
  var tool = message.tool
  var args = message.args

  _executor.execute(tool, args, function(err, result) {
    var WS = _win.WebSocket
    if (_ws && _ws.readyState === WS.OPEN) {
      if (err) {
        _ws.send(JSON.stringify({
          type: 'toolResult',
          callId: callId,
          error: err.message
        }))
      } else {
        _ws.send(JSON.stringify({
          type: 'toolResult',
          callId: callId,
          result: result
        }))
      }
    }
  })
}

function _getNwGui() {
  try {
    return require('nw.gui')
  } catch (e) {
    return typeof nw !== 'undefined' ? nw : null
  }
}

/**
 * Disconnect from the MCP server
 */
function disconnect() {
  _config.autoReconnect = false

  if (_reconnectTimer) {
    clearTimeout(_reconnectTimer)
    _reconnectTimer = null
  }

  if (_ws) {
    _ws.close()
    _ws = null
  }

  _connected = false
}

/**
 * Check if connected
 * @returns {boolean}
 */
function isConnected() {
  return _connected
}

/**
 * Get the app ID
 * @returns {string|null}
 */
function getAppId() {
  return _appId
}

module.exports = {
  init: init,
  connect: init,  // Alias for backwards compatibility
  disconnect: disconnect,
  isConnected: isConnected,
  getAppId: getAppId
}
