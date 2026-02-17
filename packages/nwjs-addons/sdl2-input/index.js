'use strict'

var native = require('../build/Release/nwjs_addons.node')

var initialized = false

function ensureInit() {
  if (!initialized) {
    native.sdl2Init()
    initialized = true
  }
}

/**
 * Joystick class for raw joystick access
 * @param {number} deviceIndex - Device index from getJoysticks()
 */
function Joystick(deviceIndex) {
  ensureInit()
  this._ptr = native.sdl2OpenJoystick(deviceIndex)
  if (!this._ptr) {
    throw new Error('Failed to open joystick at index ' + deviceIndex)
  }
}

/**
 * Close the joystick
 */
Joystick.prototype.close = function() {
  if (this._ptr) {
    native.sdl2CloseJoystick(this._ptr)
    this._ptr = null
  }
}

/**
 * Get current joystick state
 * @returns {Object} State with axes[], buttons[], hats[]
 */
Joystick.prototype.getState = function() {
  var result = null
  if (this._ptr) {
    result = native.sdl2GetJoystickState(this._ptr)
  }
  return result
}

/**
 * Trigger rumble/vibration
 * @param {number} lowFreq - Low frequency motor (0-65535)
 * @param {number} highFreq - High frequency motor (0-65535)
 * @param {number} durationMs - Duration in milliseconds
 * @returns {boolean} Success
 */
Joystick.prototype.rumble = function(lowFreq, highFreq, durationMs) {
  var result = false
  if (this._ptr) {
    result = native.sdl2RumbleJoystick(this._ptr, lowFreq, highFreq, durationMs)
  }
  return result
}

/**
 * GameController class for standardized Xbox-style controller access
 * @param {number} deviceIndex - Device index from getGameControllers()
 */
function GameController(deviceIndex) {
  ensureInit()
  this._ptr = native.sdl2OpenGameController(deviceIndex)
  if (!this._ptr) {
    throw new Error('Failed to open game controller at index ' + deviceIndex)
  }
}

/**
 * Close the game controller
 */
GameController.prototype.close = function() {
  if (this._ptr) {
    native.sdl2CloseGameController(this._ptr)
    this._ptr = null
  }
}

/**
 * Get current game controller state
 * @returns {Object} Standardized state with named buttons/axes
 */
GameController.prototype.getState = function() {
  var result = null
  if (this._ptr) {
    result = native.sdl2GetGameControllerState(this._ptr)
  }
  return result
}

/**
 * Trigger rumble/vibration
 * @param {number} lowFreq - Low frequency motor (0-65535)
 * @param {number} highFreq - High frequency motor (0-65535)
 * @param {number} durationMs - Duration in milliseconds
 * @returns {boolean} Success
 */
GameController.prototype.rumble = function(lowFreq, highFreq, durationMs) {
  var result = false
  if (this._ptr) {
    result = native.sdl2RumbleGameController(this._ptr, lowFreq, highFreq, durationMs)
  }
  return result
}

/**
 * Get controller name
 * @returns {string} Controller name
 */
GameController.prototype.getName = function() {
  var result = ''
  if (this._ptr) {
    result = native.sdl2GetGameControllerName(this._ptr)
  }
  return result
}

/**
 * Get list of all connected joysticks
 * @returns {Array} Array of joystick info objects
 */
function getJoysticks() {
  ensureInit()
  var count = native.sdl2GetNumJoysticks()
  var result = []
  for (var i = 0; i < count; i++) {
    result.push(native.sdl2GetJoystickInfo(i))
  }
  return result
}

/**
 * Get list of connected game controllers (devices with standardized mapping)
 * @returns {Array} Array of controller info objects
 */
function getGameControllers() {
  ensureInit()
  var count = native.sdl2GetNumJoysticks()
  var result = []
  for (var i = 0; i < count; i++) {
    var isController = native.sdl2IsGameController(i)
    if (isController) {
      result.push(native.sdl2GetJoystickInfo(i))
    }
  }
  return result
}

/**
 * Get mouse state relative to focused window
 * @returns {Object} Mouse state with x, y, left, middle, right, x1, x2
 */
function getMouseState() {
  ensureInit()
  return native.sdl2GetMouseState()
}

/**
 * Get global mouse state (screen coordinates)
 * @returns {Object} Mouse state with x, y, left, middle, right, x1, x2
 */
function getGlobalMouseState() {
  ensureInit()
  return native.sdl2GetGlobalMouseState()
}

/**
 * Get relative mouse movement since last call
 * @returns {Object} Mouse delta with x, y, left, middle, right, x1, x2
 */
function getRelativeMouseState() {
  ensureInit()
  return native.sdl2GetRelativeMouseState()
}

/**
 * Poll for events and update joystick/controller states
 * Call this once per frame in your game loop
 */
function update() {
  ensureInit()
  native.sdl2Update()
}

/**
 * Shutdown SDL2 input subsystem
 */
function quit() {
  if (initialized) {
    native.sdl2Quit()
    initialized = false
  }
}

module.exports = {
  Joystick: Joystick,
  GameController: GameController,
  getJoysticks: getJoysticks,
  getGameControllers: getGameControllers,
  getMouseState: getMouseState,
  getGlobalMouseState: getGlobalMouseState,
  getRelativeMouseState: getRelativeMouseState,
  update: update,
  quit: quit
}
