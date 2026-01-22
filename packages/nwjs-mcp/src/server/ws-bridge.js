'use strict'

/**
 * WebSocket Bridge for NW.js Apps
 *
 * Manages WebSocket connections from NW.js apps and routes tool calls to them.
 */

var WebSocket = require('ws')

function WebSocketBridge(port) {
  this.port = port
  this.server = null
  this.apps = {}  // appId -> { ws, name, active }
  this.activeAppId = null
  this.pendingCalls = {}  // callId -> { callback, timeout }
  this.callIdCounter = 0
}

WebSocketBridge.prototype.start = function() {
  var self = this

  this.server = new WebSocket.Server({ port: this.port })

  this.server.on('listening', function() {
    process.stderr.write('[nwjs-mcp] WebSocket server listening on port ' + self.port + '\n')
  })

  this.server.on('connection', function(ws) {
    self._handleConnection(ws)
  })

  this.server.on('error', function(err) {
    process.stderr.write('[nwjs-mcp] WebSocket server error: ' + err.message + '\n')
  })
}

WebSocketBridge.prototype.stop = function() {
  if (this.server) {
    this.server.close()
    this.server = null
  }
}

WebSocketBridge.prototype._handleConnection = function(ws) {
  var self = this
  var appId = null

  ws.on('message', function(data) {
    var message = null
    try {
      message = JSON.parse(data)
    } catch (e) {
      process.stderr.write('[nwjs-mcp] Invalid message from app: ' + e.message + '\n')
      return
    }

    self._handleMessage(ws, message, function(assignedId) {
      if (assignedId) {
        appId = assignedId
      }
    })
  })

  ws.on('close', function() {
    if (appId && self.apps[appId]) {
      process.stderr.write('[nwjs-mcp] App disconnected: ' + appId + '\n')
      delete self.apps[appId]

      // If this was the active app, clear selection
      if (self.activeAppId === appId) {
        self.activeAppId = null
        // Auto-select another app if available
        var appIds = Object.keys(self.apps)
        if (appIds.length > 0) {
          self.activeAppId = appIds[0]
          self.apps[self.activeAppId].active = true
        }
      }
    }
  })

  ws.on('error', function(err) {
    process.stderr.write('[nwjs-mcp] WebSocket error: ' + err.message + '\n')
  })
}

WebSocketBridge.prototype._handleMessage = function(ws, message, onRegister) {
  var self = this

  // Handle registration
  if (message.type === 'register') {
    var appId = message.appId || 'app-' + Date.now()
    var appName = message.name || ''

    // If app already exists (reconnect), update the WebSocket
    if (this.apps[appId]) {
      this.apps[appId].ws = ws
      process.stderr.write('[nwjs-mcp] App reconnected: ' + appId + '\n')
    } else {
      this.apps[appId] = {
        ws: ws,
        name: appName,
        active: false
      }
      process.stderr.write('[nwjs-mcp] App registered: ' + appId + (appName ? ' (' + appName + ')' : '') + '\n')
    }

    // Auto-select first app
    if (!this.activeAppId) {
      this.activeAppId = appId
      this.apps[appId].active = true
    }

    // Send confirmation
    ws.send(JSON.stringify({
      type: 'registered',
      appId: appId
    }))

    onRegister(appId)
    return
  }

  // Handle tool response
  if (message.type === 'toolResult') {
    var callId = message.callId
    var pending = this.pendingCalls[callId]

    if (pending) {
      clearTimeout(pending.timeout)
      delete this.pendingCalls[callId]

      if (message.error) {
        pending.callback(new Error(message.error))
      } else {
        pending.callback(null, message.result)
      }
    }
    return
  }
}

WebSocketBridge.prototype.getConnectedApps = function() {
  var self = this
  return Object.keys(this.apps).map(function(appId) {
    return {
      id: appId,
      name: self.apps[appId].name,
      active: appId === self.activeAppId
    }
  })
}

WebSocketBridge.prototype.selectApp = function(appId) {
  if (!this.apps[appId]) {
    return false
  }

  // Deactivate current
  if (this.activeAppId && this.apps[this.activeAppId]) {
    this.apps[this.activeAppId].active = false
  }

  // Activate new
  this.activeAppId = appId
  this.apps[appId].active = true
  return true
}

WebSocketBridge.prototype.callTool = function(toolName, toolArgs, callback) {
  var self = this

  // Get active app
  if (!this.activeAppId || !this.apps[this.activeAppId]) {
    callback(new Error('No NW.js app connected. Start an NW.js app with the MCP client library.'))
    return
  }

  var app = this.apps[this.activeAppId]
  var ws = app.ws

  if (ws.readyState !== WebSocket.OPEN) {
    callback(new Error('App connection not ready'))
    return
  }

  // Generate call ID
  var callId = 'call-' + (++this.callIdCounter)

  // Set timeout
  var timeout = setTimeout(function() {
    delete self.pendingCalls[callId]
    callback(new Error('Tool call timed out'))
  }, 30000)

  // Store pending call
  this.pendingCalls[callId] = {
    callback: callback,
    timeout: timeout
  }

  // Send to app
  ws.send(JSON.stringify({
    type: 'toolCall',
    callId: callId,
    tool: toolName,
    args: toolArgs
  }))
}

module.exports = WebSocketBridge
