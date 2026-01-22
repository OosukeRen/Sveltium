'use strict'

/**
 * NW.js MCP - Model Context Protocol for NW.js apps
 *
 * Usage in your NW.js app:
 *   var mcp = require('@sveltium/nwjs-mcp')
 *   mcp.init({ name: 'My App' })
 *
 * The server auto-starts on first app launch and persists across app restarts.
 */

module.exports = require('./src/client')
