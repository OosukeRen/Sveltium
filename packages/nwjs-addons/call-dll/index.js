/**
 * Call-DLL - Dynamic DLL Loading and Function Calling
 *
 * Enables calling Windows API and third-party DLL functions
 * directly from JavaScript without writing C++ bindings.
 */

'use strict'

var native = require('../build/Release/nwjs_addons.node').calldll

/**
 * Load a DLL
 * @param {string} path - Path to DLL file (or system DLL name like 'user32.dll')
 * @returns {DLLHandle}
 */
function load(path) {
  return new DLLHandle(native.load(path))
}

/**
 * DLL Handle - represents a loaded DLL
 * @constructor
 * @param {Object} nativeHandle - Native handle object
 */
function DLLHandle(nativeHandle) {
  this._native = nativeHandle
}

/**
 * Get a function from the DLL
 * @param {string} name - Function name
 * @param {string} callConvention - 'cdecl' or 'stdcall'
 * @param {string} returnType - Return type (e.g., 'int32', 'uint32', 'void', 'string', 'pointer')
 * @param {string[]} argTypes - Array of argument types
 * @returns {Function} Callable function
 */
DLLHandle.prototype.getFunction = function(name, callConvention, returnType, argTypes) {
  callConvention = callConvention || 'cdecl'
  returnType = returnType || 'int32'
  argTypes = argTypes || []

  // Native API expects: (name, returnType, argTypes[], options?)
  var options = { callConvention: callConvention }
  var nativeFunc = this._native.getFunction(name, returnType, argTypes, options)

  // Return a wrapper function that can be called directly
  return function() {
    var args = Array.prototype.slice.call(arguments)
    return nativeFunc.call.apply(nativeFunc, args)
  }
}

/**
 * Get exported symbol address
 * @param {string} name - Symbol name
 * @returns {number|null} Symbol address or null
 */
DLLHandle.prototype.getSymbol = function(name) {
  return this._native.getSymbol(name)
}

/**
 * Get the path of the loaded DLL
 * @returns {string}
 */
DLLHandle.prototype.getPath = function() {
  return this._native.getPath()
}

/**
 * Close the DLL handle
 */
DLLHandle.prototype.close = function() {
  this._native.close()
}

/**
 * Get last error message
 * @returns {string|null}
 */
DLLHandle.prototype.getError = function() {
  return this._native.getError()
}

// Exports
module.exports = {
  load: load,
  DLLHandle: DLLHandle
}
