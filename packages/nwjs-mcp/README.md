# nwjs-mcp

MCP (Model Context Protocol) server for NW.js application testing and automation.

This package enables AI assistants (like Claude Code) to interact with and test NW.js applications, addressing the limitation that browser automation tools like Playwright don't work with NW.js.

## Installation

```bash
npm install nwjs-mcp
```

## Quick Start

Add to your NW.js app's `node-main` script:

```javascript
// main.js (node-main in package.json)
if (process.argv.includes('--mcp')) {
  require('nwjs-mcp').startServer({
    mode: 'stdio',
    native: true
  })
}
```

Or in your `package.json`:

```json
{
  "main": "index.html",
  "node-main": "mcp-bootstrap.js"
}
```

## Claude Code Integration

Add to your `.claude.json` or Claude Code settings:

```json
{
  "mcpServers": {
    "nwjs": {
      "command": "path/to/nw.exe",
      "args": ["path/to/your-app", "--mcp"],
      "env": {
        "NWJS_MCP_MODE": "stdio"
      }
    }
  }
}
```

## Available Tools

| Tool | Description |
|------|-------------|
| `browser_snapshot` | Get accessibility tree of the page with element refs |
| `browser_take_screenshot` | Capture page screenshot |
| `browser_click` | Click an element by ref |
| `browser_type` | Type text into an element |
| `browser_press_key` | Press a keyboard key |
| `browser_evaluate` | Execute JavaScript on the page |
| `browser_navigate` | Navigate to a URL |
| `browser_console_messages` | Get console log history |
| `browser_resize` | Resize the window |
| `browser_wait_for` | Wait for text or time |
| `browser_fill_form` | Fill multiple form fields |

## API

### startServer(options)

Start the MCP server.

```javascript
var server = require('nwjs-mcp').startServer({
  mode: 'stdio',     // 'stdio' or 'http'
  port: 3000,        // HTTP port (only for http mode)
  native: true       // Try to load native addon
})
```

### createServer(options)

Create server instance without starting.

```javascript
var server = require('nwjs-mcp').createServer({ mode: 'http' })
// Start later
server.start()
// Stop when done
server.stop()
```

## Server Modes

### stdio Mode (Recommended)

Uses stdin/stdout for communication. Best for integration with Claude Code.

```javascript
startServer({ mode: 'stdio' })
```

### HTTP Mode

Runs an HTTP server for network access.

```javascript
startServer({ mode: 'http', port: 3000 })
```

## Native Addon (Optional)

For enhanced features like system-level input (useful for testing native dialogs), install the native addon:

```bash
npm install @aspect/nwjs-addons
```

Native features:
- System-level mouse clicks (for native dialogs)
- System-level keyboard input
- Window management (focus, minimize)
- Screen capture

The server gracefully falls back to JS-only mode if native addon is unavailable.

## Example Usage with Claude Code

Once configured, you can ask Claude Code to:

```
"Test my NW.js app - click the login button and verify the form appears"
```

Claude Code will:
1. Use `browser_snapshot` to see the page structure
2. Use `browser_click` to click the login button
3. Use `browser_snapshot` again to verify the form
4. Report the results

## License

MIT
