/**
 * IPC addon - Inter-process communication via named pipes
 * Supports process monitoring and channel-based messaging
 */

'use strict'

var EventEmitter = require('events').EventEmitter
var util = require('util')
var native = require('../build/Release/nwjs_addons.node')

/**
 * Named pipe channel for IPC communication
 * @constructor
 * @extends EventEmitter
 * @param {string} name - Channel name
 * @param {boolean} isServer - True to create server, false for client
 */
function Channel(name, isServer) {
  EventEmitter.call(this)

  if (typeof name !== 'string' || name.length === 0) {
    throw new TypeError('name must be a non-empty string')
  }

  this._id = native.ipcCreateChannel(name, isServer)
  this._name = name
  this._isServer = isServer
}

util.inherits(Channel, EventEmitter)

Object.defineProperty(Channel.prototype, 'name', {
  get: function() {
    return this._name
  }
})

Object.defineProperty(Channel.prototype, 'isServer', {
  get: function() {
    return native.ipcChannelIsServer(this._id)
  }
})

Object.defineProperty(Channel.prototype, 'connected', {
  get: function() {
    return native.ipcChannelIsConnected(this._id)
  }
})

/**
 * Connect the channel (blocking for server until client connects)
 * @returns {boolean} True if connected successfully
 */
Channel.prototype.connect = function() {
  var success = native.ipcChannelConnect(this._id)
  if (success) {
    this.emit('connect')
  }
  return success
}

/**
 * Send data through the channel
 * @param {string|Buffer} data - Data to send
 * @returns {boolean} True if sent successfully
 */
Channel.prototype.send = function(data) {
  if (typeof data !== 'string' && !Buffer.isBuffer(data)) {
    throw new TypeError('data must be string or Buffer')
  }
  return native.ipcChannelSend(this._id, data)
}

/**
 * Receive data from the channel (blocking)
 * @returns {Buffer|null} Received data or null
 */
Channel.prototype.receive = function() {
  return native.ipcChannelReceive(this._id)
}

/**
 * Receive data as string
 * @param {string} [encoding] - String encoding (default: 'utf8')
 * @returns {string|null}
 */
Channel.prototype.receiveString = function(encoding) {
  encoding = encoding || 'utf8'
  var data = this.receive()
  if (data === null) {
    return null
  }
  return data.toString(encoding)
}

/**
 * Close the channel
 */
Channel.prototype.close = function() {
  native.ipcChannelClose(this._id)
  this.emit('disconnect')
}

/**
 * Process monitor for tracking external processes
 * @constructor
 * @extends EventEmitter
 * @param {number} pid - Process ID to monitor
 */
function ProcessMonitor(pid) {
  EventEmitter.call(this)

  if (typeof pid !== 'number' || pid <= 0) {
    throw new TypeError('pid must be a positive number')
  }

  this._pid = pid
  this._interval = null
  this._running = false
}

util.inherits(ProcessMonitor, EventEmitter)

Object.defineProperty(ProcessMonitor.prototype, 'pid', {
  get: function() {
    return this._pid
  }
})

Object.defineProperty(ProcessMonitor.prototype, 'running', {
  get: function() {
    return isProcessRunning(this._pid)
  }
})

/**
 * Start monitoring the process
 * @param {number} [pollInterval] - Polling interval in milliseconds (default: 1000)
 * @returns {ProcessMonitor} this
 */
ProcessMonitor.prototype.start = function(pollInterval) {
  var self = this
  pollInterval = pollInterval || 1000

  if (this._interval) {
    return this
  }

  this._running = true
  this._interval = setInterval(function() {
    if (!isProcessRunning(self._pid)) {
      self.stop()
      self.emit('exit', 0)
    }
  }, pollInterval)

  return this
}

/**
 * Stop monitoring
 */
ProcessMonitor.prototype.stop = function() {
  this._running = false
  if (this._interval) {
    clearInterval(this._interval)
    this._interval = null
  }
}

/**
 * Check if a process is running
 * @param {number} pid - Process ID
 * @returns {boolean}
 */
function isProcessRunning(pid) {
  if (typeof pid !== 'number' || pid <= 0) {
    throw new TypeError('pid must be a positive number')
  }
  return native.ipcIsProcessRunning(pid)
}

/**
 * Generate a unique channel name (UUID-based)
 * @returns {string}
 */
function generateChannelName() {
  return native.ipcGenerateChannelName()
}

/**
 * Create a named pipe server
 * @param {string} name - Channel name
 * @returns {Channel}
 */
function createServer(name) {
  return new Channel(name, true)
}

/**
 * Connect to a named pipe server
 * @param {string} name - Channel name
 * @returns {Channel}
 */
function connect(name) {
  return new Channel(name, false)
}

/**
 * Create a process monitor
 * @param {number} pid - Process ID
 * @returns {ProcessMonitor}
 */
function monitorProcess(pid) {
  return new ProcessMonitor(pid)
}

module.exports = {
  Channel: Channel,
  ProcessMonitor: ProcessMonitor,
  isProcessRunning: isProcessRunning,
  generateChannelName: generateChannelName,
  createServer: createServer,
  connect: connect,
  monitorProcess: monitorProcess
}
