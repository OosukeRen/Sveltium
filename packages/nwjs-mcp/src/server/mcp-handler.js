'use strict'

/**
 * MCP Protocol Handler (stdio transport)
 *
 * Handles JSON-RPC 2.0 messages from Claude Code via stdin/stdout
 */

var PROTOCOL_VERSION = '2024-11-05'
var SERVER_NAME = 'nwjs-mcp'
var SERVER_VERSION = '0.1.0'

// Default NW.js executable path - can be overridden via setNwPath()
var DEFAULT_NW_PATH = null

function MCPServer(wsBridge) {
  this.wsBridge = wsBridge
  this.buffer = ''
  this.running = false
  this.nwPath = DEFAULT_NW_PATH
  this.startedApps = []  // Track spawned app processes
}

/**
 * Set the default NW.js executable path
 * @param {string} path - Path to nw.exe
 */
MCPServer.prototype.setNwPath = function(path) {
  this.nwPath = path
}

MCPServer.prototype.start = function() {
  var self = this
  this.running = true

  process.stdin.setEncoding('utf8')

  process.stdin.on('data', function(chunk) {
    self._handleData(chunk)
  })

  process.stdin.on('end', function() {
    self.running = false
    process.exit(0)
  })

  // Log to stderr (stdout is for MCP protocol)
  process.stderr.write('[nwjs-mcp] Server started, waiting for MCP messages...\n')
}

MCPServer.prototype._handleData = function(chunk) {
  var self = this
  this.buffer += chunk

  var lines = this.buffer.split('\n')
  this.buffer = lines.pop()

  for (var i = 0; i < lines.length; i++) {
    var line = lines[i].trim()
    if (line) {
      self._processMessage(line)
    }
  }
}

/**
 * Handle a message and return response via callback (for HTTP mode)
 */
MCPServer.prototype.handleMessage = function(message, callback) {
  var self = this
  var request = null

  try {
    request = JSON.parse(message)
  } catch (e) {
    callback(JSON.stringify({
      jsonrpc: '2.0',
      id: null,
      error: { code: -32700, message: 'Parse error' }
    }))
    return
  }

  var id = request.id
  var method = request.method
  var params = request.params || {}

  this._handleMethod(id, method, params, function(result, error) {
    if (error) {
      callback(JSON.stringify({
        jsonrpc: '2.0',
        id: id,
        error: error
      }))
    } else {
      callback(JSON.stringify({
        jsonrpc: '2.0',
        id: id,
        result: result
      }))
    }
  })
}

MCPServer.prototype._processMessage = function(message) {
  var self = this
  var request = null

  try {
    request = JSON.parse(message)
  } catch (e) {
    this._sendError(null, -32700, 'Parse error')
    return
  }

  var id = request.id
  var method = request.method
  var params = request.params || {}

  this._handleMethod(id, method, params, function(result, error) {
    if (error) {
      self._sendError(id, error.code, error.message, error.data)
    } else {
      self._sendResult(id, result)
    }
  })
}

MCPServer.prototype._handleMethod = function(id, method, params, callback) {
  var self = this

  switch (method) {
    case 'initialize':
      callback({
        protocolVersion: PROTOCOL_VERSION,
        capabilities: { tools: {} },
        serverInfo: { name: SERVER_NAME, version: SERVER_VERSION }
      })
      break

    case 'initialized':
      callback({})
      break

    case 'tools/list':
      callback({ tools: this._getToolsList() })
      break

    case 'tools/call':
      this._handleToolCall(params, callback)
      break

    case 'ping':
      callback({})
      break

    default:
      callback(null, { code: -32601, message: 'Method not found: ' + method })
  }
}

MCPServer.prototype._getToolsList = function() {
  return [
    {
      name: 'browser_snapshot',
      description: 'Capture accessibility snapshot of the current page. Returns a tree structure with element refs for targeting interactions.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'browser_take_screenshot',
      description: 'Take a screenshot of the current page or a specific element.',
      inputSchema: {
        type: 'object',
        properties: {
          ref: { type: 'string', description: 'Element ref to screenshot (optional)' },
          fullPage: { type: 'boolean', description: 'Capture full scrollable page' }
        }
      }
    },
    {
      name: 'browser_click',
      description: 'Click on an element specified by ref.',
      inputSchema: {
        type: 'object',
        properties: {
          ref: { type: 'string', description: 'Element ref from snapshot' },
          element: { type: 'string', description: 'Human-readable element description' },
          button: { type: 'string', enum: ['left', 'right', 'middle'], description: 'Mouse button' },
          doubleClick: { type: 'boolean', description: 'Perform double click' }
        },
        required: ['ref', 'element']
      }
    },
    {
      name: 'browser_type',
      description: 'Type text into an editable element.',
      inputSchema: {
        type: 'object',
        properties: {
          ref: { type: 'string', description: 'Element ref from snapshot' },
          element: { type: 'string', description: 'Human-readable element description' },
          text: { type: 'string', description: 'Text to type' },
          slowly: { type: 'boolean', description: 'Type character by character' },
          submit: { type: 'boolean', description: 'Press Enter after typing' }
        },
        required: ['ref', 'element', 'text']
      }
    },
    {
      name: 'browser_evaluate',
      description: 'Evaluate JavaScript expression on the page.',
      inputSchema: {
        type: 'object',
        properties: {
          function: { type: 'string', description: 'JavaScript function to evaluate' },
          ref: { type: 'string', description: 'Element ref to pass to function (optional)' },
          element: { type: 'string', description: 'Human-readable element description' }
        },
        required: ['function']
      }
    },
    {
      name: 'browser_navigate',
      description: 'Navigate to a URL.',
      inputSchema: {
        type: 'object',
        properties: {
          url: { type: 'string', description: 'URL to navigate to' }
        },
        required: ['url']
      }
    },
    {
      name: 'browser_console_messages',
      description: 'Get console messages from the page.',
      inputSchema: {
        type: 'object',
        properties: {
          level: { type: 'string', enum: ['error', 'warning', 'info', 'debug'], description: 'Minimum log level' }
        }
      }
    },
    {
      name: 'browser_resize',
      description: 'Resize the browser window.',
      inputSchema: {
        type: 'object',
        properties: {
          width: { type: 'number', description: 'Window width in pixels' },
          height: { type: 'number', description: 'Window height in pixels' }
        },
        required: ['width', 'height']
      }
    },
    {
      name: 'browser_wait_for',
      description: 'Wait for text to appear or disappear, or wait for a specified time.',
      inputSchema: {
        type: 'object',
        properties: {
          text: { type: 'string', description: 'Text to wait for' },
          textGone: { type: 'string', description: 'Text to wait for to disappear' },
          time: { type: 'number', description: 'Time to wait in seconds' }
        }
      }
    },
    {
      name: 'browser_fill_form',
      description: 'Fill multiple form fields at once.',
      inputSchema: {
        type: 'object',
        properties: {
          fields: {
            type: 'array',
            description: 'Array of fields to fill',
            items: {
              type: 'object',
              properties: {
                ref: { type: 'string' },
                name: { type: 'string' },
                type: { type: 'string', enum: ['textbox', 'checkbox', 'radio', 'combobox', 'slider'] },
                value: { type: 'string' }
              },
              required: ['ref', 'name', 'type', 'value']
            }
          }
        },
        required: ['fields']
      }
    },
    {
      name: 'browser_press_key',
      description: 'Press a keyboard key.',
      inputSchema: {
        type: 'object',
        properties: {
          key: { type: 'string', description: 'Key to press (e.g., "Enter", "Tab", "a")' }
        },
        required: ['key']
      }
    },
    {
      name: 'nwjs_list_apps',
      description: 'List all connected NW.js applications.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_select_app',
      description: 'Select which NW.js app to target for browser commands.',
      inputSchema: {
        type: 'object',
        properties: {
          appId: { type: 'string', description: 'App ID to select' }
        },
        required: ['appId']
      }
    },
    {
      name: 'nwjs_reload',
      description: 'Reload the NW.js app window.',
      inputSchema: {
        type: 'object',
        properties: {
          ignoreCache: { type: 'boolean', description: 'Ignore cache when reloading (like Ctrl+Shift+R)' },
          relaunch: { type: 'boolean', description: 'Fully relaunch the app (restart the process) instead of just reloading the window' }
        }
      }
    },
    {
      name: 'nwjs_show_devtools',
      description: 'Show the developer tools for the NW.js app.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_close',
      description: 'Close the NW.js app window.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_start_app',
      description: 'Start an NW.js application. The app must include the MCP client library to connect.',
      inputSchema: {
        type: 'object',
        properties: {
          appPath: { type: 'string', description: 'Path to the NW.js app directory (containing package.json)' },
          nwPath: { type: 'string', description: 'Path to nw.exe (optional, uses configured default if not specified)' },
          args: { type: 'array', items: { type: 'string' }, description: 'Additional command line arguments' }
        },
        required: ['appPath']
      }
    },
    {
      name: 'nwjs_get_manifest',
      description: 'Get the app manifest (package.json) information.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_get_argv',
      description: 'Get command line arguments the app was started with.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_minimize',
      description: 'Minimize the NW.js app window.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_maximize',
      description: 'Maximize the NW.js app window.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_restore',
      description: 'Restore the NW.js app window from minimized/maximized state.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_focus',
      description: 'Bring the NW.js app window to the front.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_get_bounds',
      description: 'Get the window position and size.',
      inputSchema: {
        type: 'object',
        properties: {}
      }
    },
    {
      name: 'nwjs_set_bounds',
      description: 'Set the window position and size.',
      inputSchema: {
        type: 'object',
        properties: {
          x: { type: 'number', description: 'Window X position' },
          y: { type: 'number', description: 'Window Y position' },
          width: { type: 'number', description: 'Window width' },
          height: { type: 'number', description: 'Window height' }
        }
      }
    },
    {
      name: 'nwjs_zoom',
      description: 'Set the zoom level of the page.',
      inputSchema: {
        type: 'object',
        properties: {
          level: { type: 'number', description: 'Zoom level (1.0 = 100%, 1.5 = 150%, etc.)' }
        },
        required: ['level']
      }
    }
  ]
}

MCPServer.prototype._handleToolCall = function(params, callback) {
  var self = this
  var toolName = params.name
  var toolArgs = params.arguments || {}

  // Handle server-side tools
  if (toolName === 'nwjs_list_apps') {
    var apps = this.wsBridge.getConnectedApps()
    callback({
      content: [{
        type: 'text',
        text: apps.length === 0
          ? 'No NW.js apps connected. Start an NW.js app with the MCP client library.'
          : 'Connected apps:\n' + apps.map(function(app) {
              return '- ' + app.id + (app.name ? ' (' + app.name + ')' : '') + (app.active ? ' [active]' : '')
            }).join('\n')
      }]
    })
    return
  }

  if (toolName === 'nwjs_select_app') {
    var success = this.wsBridge.selectApp(toolArgs.appId)
    callback({
      content: [{
        type: 'text',
        text: success
          ? 'Selected app: ' + toolArgs.appId
          : 'App not found: ' + toolArgs.appId
      }],
      isError: !success
    })
    return
  }

  if (toolName === 'nwjs_start_app') {
    this._startApp(toolArgs, callback)
    return
  }

  // Proxy to NW.js app
  this.wsBridge.callTool(toolName, toolArgs, function(err, result) {
    if (err) {
      callback({
        content: [{ type: 'text', text: 'Error: ' + err.message }],
        isError: true
      })
    } else {
      callback(result)
    }
  })
}

MCPServer.prototype._sendResult = function(id, result) {
  var response = {
    jsonrpc: '2.0',
    id: id,
    result: result
  }
  process.stdout.write(JSON.stringify(response) + '\n')
}

MCPServer.prototype._sendError = function(id, code, message, data) {
  var response = {
    jsonrpc: '2.0',
    id: id,
    error: {
      code: code,
      message: message
    }
  }
  if (data !== undefined) {
    response.error.data = data
  }
  process.stdout.write(JSON.stringify(response) + '\n')
}

MCPServer.prototype._startApp = function(args, callback) {
  var self = this
  var spawn = require('child_process').spawn
  var path = require('path')
  var fs = require('fs')

  var appPath = args.appPath

  // Validate app path exists
  if (!fs.existsSync(appPath)) {
    callback({
      content: [{ type: 'text', text: 'App path does not exist: ' + appPath }],
      isError: true
    })
    return
  }

  // Check for package.json
  var packageJsonPath = path.join(appPath, 'package.json')
  if (!fs.existsSync(packageJsonPath)) {
    callback({
      content: [{ type: 'text', text: 'No package.json found in: ' + appPath }],
      isError: true
    })
    return
  }

  // Find nwPath from multiple sources
  var nwPath = this._findNwPath(args.nwPath, appPath)

  if (!nwPath) {
    callback({
      content: [{
        type: 'text',
        text: 'NW.js executable not found. Options:\n' +
              '1. Pass nwPath argument to this tool\n' +
              '2. Set NWJS_PATH in .env file in app directory\n' +
              '3. Set NWJS_PATH environment variable\n' +
              '4. Install "nw" package in your project: npm install nw'
      }],
      isError: true
    })
    return
  }

  // Check nw.exe exists
  if (!fs.existsSync(nwPath)) {
    callback({
      content: [{ type: 'text', text: 'NW.js executable not found at: ' + nwPath }],
      isError: true
    })
    return
  }

  // Build arguments
  var spawnArgs = [appPath]
  if (args.args && Array.isArray(args.args)) {
    spawnArgs = spawnArgs.concat(args.args)
  }

  // Spawn the app
  var child = spawn(nwPath, spawnArgs, {
    detached: true,
    stdio: 'ignore',
    cwd: appPath
  })

  child.unref()

  // Track spawned app
  self.startedApps.push({
    pid: child.pid,
    appPath: appPath,
    startedAt: Date.now()
  })

  callback({
    content: [{
      type: 'text',
      text: 'Started NW.js app: ' + appPath + '\nPID: ' + child.pid + '\nWaiting for app to connect...'
    }]
  })
}

/**
 * Parse a .env file and return key-value pairs
 * @param {string} envFilePath - Path to .env file
 * @returns {Object}
 */
MCPServer.prototype._parseEnvFile = function(envFilePath) {
  var fs = require('fs')
  var result = {}

  if (!fs.existsSync(envFilePath)) {
    return result
  }

  try {
    var content = fs.readFileSync(envFilePath, 'utf8')
    var lines = content.split('\n')

    for (var i = 0; i < lines.length; i++) {
      var line = lines[i].trim()

      // Skip empty lines and comments
      if (!line || line.charAt(0) === '#') {
        continue
      }

      var eqIndex = line.indexOf('=')
      if (eqIndex === -1) {
        continue
      }

      var key = line.substring(0, eqIndex).trim()
      var value = line.substring(eqIndex + 1).trim()

      // Remove surrounding quotes if present
      if ((value.charAt(0) === '"' && value.charAt(value.length - 1) === '"') ||
          (value.charAt(0) === "'" && value.charAt(value.length - 1) === "'")) {
        value = value.substring(1, value.length - 1)
      }

      result[key] = value
    }
  } catch (e) {
    // Failed to read/parse .env file
  }

  return result
}

/**
 * Find NW.js executable path from multiple sources
 * @param {string} explicitPath - Path passed as argument
 * @param {string} appPath - App directory to check for nw package
 * @returns {string|null}
 */
MCPServer.prototype._findNwPath = function(explicitPath, appPath) {
  var fs = require('fs')
  var path = require('path')

  // 1. Explicit path from argument
  if (explicitPath && fs.existsSync(explicitPath)) {
    return explicitPath
  }

  // 2. Server configured path
  if (this.nwPath && fs.existsSync(this.nwPath)) {
    return this.nwPath
  }

  // 3. Environment variable
  var envPath = process.env.NWJS_PATH
  if (envPath && fs.existsSync(envPath)) {
    return envPath
  }

  // 4. Check .env file in app directory
  var envFile = this._parseEnvFile(path.join(appPath, '.env'))
  if (envFile.NWJS_PATH && fs.existsSync(envFile.NWJS_PATH)) {
    return envFile.NWJS_PATH
  }

  // 5. Try to find from 'nw' package in app's node_modules
  var nwPackagePath = path.join(appPath, 'node_modules', 'nw')
  if (fs.existsSync(nwPackagePath)) {
    try {
      // The nw package has a findpath module
      var findpath = require(path.join(nwPackagePath, 'lib', 'findpath'))
      var foundPath = findpath()
      if (foundPath && fs.existsSync(foundPath)) {
        return foundPath
      }
    } catch (e) {
      // findpath module not available or failed
    }
  }

  // 6. Check common locations on Windows
  if (process.platform === 'win32') {
    var commonPaths = [
      path.join(process.env.LOCALAPPDATA || '', 'nw'),
      path.join(process.env.PROGRAMFILES || '', 'nw'),
      'C:\\nw'
    ]

    for (var i = 0; i < commonPaths.length; i++) {
      var nwExe = path.join(commonPaths[i], 'nw.exe')
      if (fs.existsSync(nwExe)) {
        return nwExe
      }
    }
  }

  return null
}

module.exports = MCPServer
