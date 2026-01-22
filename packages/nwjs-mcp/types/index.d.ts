/**
 * NW.js MCP Server Type Definitions
 */

export interface MCPServerOptions {
  /** Server mode: stdio for command-line integration, http for network access */
  mode?: 'stdio' | 'http'
  /** HTTP port (only used in http mode) */
  port?: number
  /** Whether to try loading native addon for enhanced features */
  native?: boolean
}

export interface MCPServer {
  /** Start the server */
  start(): void
  /** Stop the server */
  stop(): void
  /** Whether the server is running */
  running: boolean
}

/**
 * Start the MCP server
 * @param options Server configuration options
 * @returns The server instance
 */
export function startServer(options?: MCPServerOptions): MCPServer

/**
 * Create MCP server instance without starting
 * @param options Server configuration options
 * @returns The server instance
 */
export function createServer(options?: MCPServerOptions): MCPServer

/**
 * MCP Server class
 */
export { MCPServer }

// Tool types for reference
export interface MCPToolResult {
  content: Array<{
    type: 'text' | 'image'
    text?: string
    data?: string
    mimeType?: string
  }>
  isError?: boolean
}

export interface MCPTool {
  name: string
  description: string
  inputSchema: object
  execute(args: object, callback: (err: Error | null, result?: MCPToolResult) => void): void
}
