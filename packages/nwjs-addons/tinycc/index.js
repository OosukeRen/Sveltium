/**
 * TinyCC Runtime Executor
 *
 * Provides runtime C compilation and execution using TinyCC.
 *
 * IMPORTANT: This module requires libtcc to be compiled and linked.
 * See tinycc/README.md for setup instructions.
 */

'use strict'

var path = require('path')
var native = null
var loadError = null

try {
  var addon = require('../build/Release/nwjs_addons.node')
  native = addon.tinycc || null
  if (!native) {
    loadError = new Error('tinycc module not found in native addon')
  }
} catch (err) {
  loadError = err
}

// Path to bundled TinyCC runtime
var runtimePath = path.join(__dirname, 'runtime')

/**
 * Check if tinycc addon is available
 * @returns {boolean}
 */
function isAvailable() {
  return native !== null
}

/**
 * Get the error that occurred during loading (if any)
 * @returns {Error|null}
 */
function getLoadError() {
  return loadError
}

/**
 * Create a new TinyCC compiler instance
 * @param {Object} [options] - Configuration options
 * @param {boolean} [options.useBuiltinRuntime=true] - Use bundled C runtime
 * @returns {Compiler}
 */
function create(options) {
  if (!native) {
    throw new Error('TinyCC addon not available: ' + (loadError ? loadError.message : 'unknown error'))
  }

  var opts = options || {}
  var useBuiltinRuntime = opts.useBuiltinRuntime !== false

  var compiler = new Compiler(native.create())

  // Automatically configure bundled runtime paths
  if (useBuiltinRuntime) {
    compiler.setLibPath(runtimePath)
    compiler.addIncludePath(path.join(runtimePath, 'include'))
    compiler.addLibraryPath(path.join(runtimePath, 'lib'))
  }

  // Always add the addon's include directory for jsbridge.h
  compiler.addIncludePath(path.join(__dirname, 'include'))

  return compiler
}

/**
 * Compiler class - wraps native TCC instance
 */
function Compiler(nativeCompiler) {
  this._native = nativeCompiler
  this._released = false
}

/**
 * Set the TinyCC library path (for runtime files)
 * @param {string} libPath - Path to TCC runtime directory
 * @returns {Compiler} this for chaining
 */
Compiler.prototype.setLibPath = function(libPath) {
  this._checkReleased()
  this._native.setLibPath(libPath)
  return this
}

/**
 * Add include path for header file resolution
 * @param {string} path - Directory path
 * @returns {Compiler} this for chaining
 */
Compiler.prototype.addIncludePath = function(path) {
  this._checkReleased()
  this._native.addIncludePath(path)
  return this
}

/**
 * Add library path for linking
 * @param {string} path - Directory path
 * @returns {Compiler} this for chaining
 */
Compiler.prototype.addLibraryPath = function(path) {
  this._checkReleased()
  this._native.addLibraryPath(path)
  return this
}

/**
 * Add library for linking
 * @param {string} name - Library name (without lib prefix or extension)
 * @returns {Compiler} this for chaining
 */
Compiler.prototype.addLibrary = function(name) {
  this._checkReleased()
  this._native.addLibrary(name)
  return this
}

/**
 * Define preprocessor macro
 * @param {string} name - Macro name
 * @param {string} [value] - Optional macro value
 * @returns {Compiler} this for chaining
 */
Compiler.prototype.define = function(name, value) {
  this._checkReleased()
  this._native.define(name, value || '')
  return this
}

/**
 * Undefine preprocessor macro
 * @param {string} name - Macro name
 * @returns {Compiler} this for chaining
 */
Compiler.prototype.undefine = function(name) {
  this._checkReleased()
  this._native.undefine(name)
  return this
}

/**
 * Compile C source code string
 * @param {string} code - C source code
 * @returns {boolean} true if compilation succeeded
 */
Compiler.prototype.compile = function(code) {
  this._checkReleased()
  return this._native.compile(code)
}

/**
 * Compile C source file
 * @param {string} path - Path to C source file
 * @returns {boolean} true if compilation succeeded
 */
Compiler.prototype.compileFile = function(path) {
  this._checkReleased()
  return this._native.compileFile(path)
}

/**
 * Get a function from compiled code
 *
 * Two calling conventions supported:
 *
 * 1. jsbridge mode (for functions using jsvalue types):
 *    getFunction(name, argCount)
 *    - Functions must use: jsvalue func(jscontext ctx, jsvalue arg1, ...)
 *
 * 2. Native types mode (for plain C functions):
 *    getFunction(name, returnType, argTypes)
 *    - Functions use plain C types: int func(int a, int b)
 *    - Types: 'void', 'int', 'int32', 'uint', 'uint32', 'int64', 'uint64',
 *             'float', 'double', 'string', 'pointer'
 *
 * @example
 * // jsbridge mode
 * var fn = compiler.getFunction('add', 2)
 *
 * @example
 * // Native types mode
 * var fn = compiler.getFunction('add', 'int', ['int', 'int'])
 *
 * @param {string} name - Function name
 * @param {number|string} argCountOrReturnType - Arg count (jsbridge) or return type string (native)
 * @param {string[]} [argTypes] - Array of argument type strings (native mode only)
 * @returns {NativeFunction} Callable function wrapper
 */
Compiler.prototype.getFunction = function(name, argCountOrReturnType, argTypes) {
  this._checkReleased()

  // Detect which mode based on second argument type
  if (typeof argCountOrReturnType === 'string') {
    // Native types mode: getFunction(name, returnType, argTypes[])
    return this._native.getFunction(name, argCountOrReturnType, argTypes || [])
  }

  // jsbridge mode: getFunction(name, argCount)
  return this._native.getFunction(name, argCountOrReturnType || 0)
}

/**
 * Get symbol address as number
 * @param {string} name - Symbol name
 * @returns {number|null} Symbol address or null if not found
 */
Compiler.prototype.getSymbol = function(name) {
  this._checkReleased()
  return this._native.getSymbol(name)
}

/**
 * Get last compilation error
 * @returns {string|null}
 */
Compiler.prototype.getError = function() {
  if (this._released) {
    return null
  }
  return this._native.getError()
}

/**
 * Release compiler resources
 */
Compiler.prototype.release = function() {
  if (!this._released) {
    this._native.release()
    this._released = true
  }
}

/**
 * Check if compiler has been released
 * @private
 */
Compiler.prototype._checkReleased = function() {
  if (this._released) {
    throw new Error('Compiler has been released')
  }
}

// Exports
module.exports = {
  isAvailable: isAvailable,
  getLoadError: getLoadError,
  create: create,
  Compiler: Compiler
}
