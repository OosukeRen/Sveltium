#!/usr/bin/env node
'use strict'

/**
 * NW.js MCP Server
 *
 * Standalone Node.js server that:
 * 1. Communicates with Claude Code via stdio (MCP protocol)
 * 2. Accepts WebSocket connections from NW.js apps
 * 3. Proxies tool calls to connected NW.js apps
 * 4. Also accepts HTTP requests for stdio proxy mode
 *
 * Usage: node server/index.js [--port 3940]
 */

var http = require('http')
var MCPHandler = require('./mcp-handler')
var WebSocketBridge = require('./ws-bridge')

var DEFAULT_WS_PORT = 3940

// Parse command line args
var wsPort = DEFAULT_WS_PORT
var args = process.argv.slice(2)

// Handle 'init' command - creates .mcp.json in current directory
if (args[0] === 'init') {
  var fs = require('fs')
  var path = require('path')
  var cwd = process.cwd()
  var mcpJsonPath = path.join(cwd, '.mcp.json')
  var serverPath = path.join(__dirname, 'index.js')

  var mcpConfig = {}
  if (fs.existsSync(mcpJsonPath)) {
    try {
      mcpConfig = JSON.parse(fs.readFileSync(mcpJsonPath, 'utf8'))
    } catch (e) {
      mcpConfig = {}
    }
  }

  if (!mcpConfig.mcpServers) {
    mcpConfig.mcpServers = {}
  }

  mcpConfig.mcpServers.nwjs = {
    command: 'node',
    args: [serverPath]
  }

  fs.writeFileSync(mcpJsonPath, JSON.stringify(mcpConfig, null, 2) + '\n')
  console.log('[nwjs-mcp] Created .mcp.json at:', mcpJsonPath)
  console.log('[nwjs-mcp] Server path:', serverPath)
  process.exit(0)
}

for (var i = 0; i < args.length; i++) {
  if ((args[i] === '--port' || args[i] === '-p') && args[i + 1]) {
    wsPort = parseInt(args[i + 1], 10)
    i++
  } else if (args[i].indexOf('--port=') === 0) {
    wsPort = parseInt(args[i].split('=')[1], 10)
  }
}

var httpPort = wsPort + 1  // 3941 for HTTP

// Create WebSocket bridge for NW.js apps
var wsBridge = new WebSocketBridge(wsPort)

// Create MCP handler
var mcpHandler = new MCPHandler(wsBridge)

// Start WebSocket bridge
wsBridge.start()

// Start MCP stdio listener
mcpHandler.start()

// Also start HTTP server for proxy mode
var httpServer = http.createServer(function(req, res) {
  if (req.method === 'POST' && req.url === '/mcp') {
    var body = ''
    req.on('data', function(chunk) {
      body += chunk
    })
    req.on('end', function() {
      mcpHandler.handleMessage(body, function(response) {
        res.writeHead(200, { 'Content-Type': 'application/json' })
        res.end(response)
      })
    })
  } else {
    res.writeHead(404)
    res.end('Not found')
  }
})

httpServer.listen(httpPort, function() {
  process.stderr.write('[nwjs-mcp] HTTP server listening on port ' + httpPort + '\n')
})

// Handle shutdown
process.on('SIGINT', function() {
  wsBridge.stop()
  httpServer.close()
  process.exit(0)
})

process.on('SIGTERM', function() {
  wsBridge.stop()
  httpServer.close()
  process.exit(0)
})