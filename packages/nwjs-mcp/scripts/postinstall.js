#!/usr/bin/env node
'use strict'

/**
 * Postinstall script that configures .mcp.json for Claude Code integration
 */

var fs = require('fs')
var path = require('path')

// Find the project root - use npm's INIT_CWD which is where npm was run from
function findProjectRoot() {
  // INIT_CWD is set by npm to the directory where `npm install` was run
  if (process.env.INIT_CWD) {
    return process.env.INIT_CWD
  }

  // Fallback: use current working directory
  return process.cwd()
}

// Get the path to the main server index.js in node_modules
function getServerPath(projectRoot) {
  // Check if we're installed in node_modules of the project (with scope)
  var nodeModulesPath = path.join(projectRoot, 'node_modules', '@sveltium', 'nwjs-mcp', 'src', 'server', 'index.js')

  // Also check without scope
  var nodeModulesPathNoScope = path.join(projectRoot, 'node_modules', 'nwjs-mcp', 'src', 'server', 'index.js')

  if (fs.existsSync(nodeModulesPath)) {
    return nodeModulesPath
  }

  if (fs.existsSync(nodeModulesPathNoScope)) {
    return nodeModulesPathNoScope
  }

  // Fallback: use relative to this script (for dev mode)
  var localPath = path.join(__dirname, '..', 'src', 'server', 'index.js')
  return path.resolve(localPath)
}

function main() {
  // Skip for global installs
  if (process.env.npm_config_global === 'true') {
    console.log('[nwjs-mcp] Global install detected. Run "nwjs-mcp init" in your project to configure .mcp.json')
    return
  }

  var projectRoot = findProjectRoot()
  if (!projectRoot) {
    console.log('[nwjs-mcp] Could not find project root, skipping .mcp.json configuration')
    return
  }

  var mcpJsonPath = path.join(projectRoot, '.mcp.json')
  var serverPath = getServerPath(projectRoot)

  var mcpConfig = {}

  // Read existing .mcp.json if it exists
  if (fs.existsSync(mcpJsonPath)) {
    try {
      mcpConfig = JSON.parse(fs.readFileSync(mcpJsonPath, 'utf8'))
    } catch (e) {
      console.log('[nwjs-mcp] Could not parse existing .mcp.json, creating new one')
      mcpConfig = {}
    }
  }

  // Ensure mcpServers object exists
  if (!mcpConfig.mcpServers) {
    mcpConfig.mcpServers = {}
  }

  // Add or update nwjs entry
  mcpConfig.mcpServers.nwjs = {
    command: 'node',
    args: [serverPath]
  }

  // Write .mcp.json
  try {
    fs.writeFileSync(mcpJsonPath, JSON.stringify(mcpConfig, null, 2) + '\n')
    console.log('[nwjs-mcp] Configured .mcp.json at:', mcpJsonPath)
    console.log('[nwjs-mcp] Server path:', serverPath)
    console.log('[nwjs-mcp] To use nwjs_start_app, either:')
    console.log('  - Install "nw" package: npm install nw')
    console.log('  - Or set NWJS_PATH in .env file')
    console.log('  - Or add NWJS_PATH env to .mcp.json')
  } catch (e) {
    console.log('[nwjs-mcp] Could not write .mcp.json:', e.message)
  }
}

main()
