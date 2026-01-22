/**
 * NW.js Addons Test Suite
 * For NW.js 0.12.3
 */

'use strict';

// Load addons
var addons = null;
var loadError = null;

try {
  addons = require('../');
} catch (err) {
  loadError = err;
}

// State
var currentDb = null;
var currentCompiler = null;
var ipcServer = null;
var ipcClient = null;

// ============================================
// Console Logging
// ============================================

var consoleEl = null;

function log(message, type) {
  type = type || 'log';
  if (!consoleEl) {
    consoleEl = document.getElementById('console-output');
  }

  var timestamp = new Date().toLocaleTimeString();
  var line = document.createElement('div');
  line.className = type;
  line.textContent = '[' + timestamp + '] ' + message;
  consoleEl.appendChild(line);
  consoleEl.scrollTop = consoleEl.scrollHeight;

  // Also log to real console
  console[type === 'success' ? 'log' : type](message);
}

function clearConsole() {
  if (consoleEl) {
    consoleEl.innerHTML = '';
  }
}

function setOutput(id, text) {
  var el = document.getElementById(id);
  if (el) {
    el.textContent = text;
  }
}

// ============================================
// Initialization
// ============================================

function init() {
  log('Initializing addon tests...', 'info');

  if (loadError) {
    log('Failed to load addons: ' + loadError.message, 'error');
    document.getElementById('addon-status').textContent = 'Error: ' + loadError.message;
    return;
  }

  // Check addon availability
  var addonList = document.getElementById('addon-list');
  var addonInfo = [
    { name: 'clipboard', obj: addons.clipboard },
    { name: 'folderDialog', obj: addons.folderDialog },
    { name: 'ipc', obj: addons.ipc },
    { name: 'callDll', obj: addons.callDll },
    { name: 'csvParser', obj: addons.csvParser },
    { name: 'tinycc', obj: addons.tinycc, check: function() { return addons.tinycc && addons.tinycc.isAvailable(); } },
    { name: 'sqlite3', obj: addons.sqlite3, check: function() { return addons.sqlite3 && addons.sqlite3.isAvailable(); } }
  ];

  var availableCount = 0;

  addonInfo.forEach(function(addon) {
    var available = addon.check ? addon.check() : (addon.obj != null);
    if (available) availableCount++;

    var div = document.createElement('div');
    div.className = 'addon-item ' + (available ? 'available' : 'unavailable');
    div.innerHTML = '<span class="addon-name">' + addon.name + '</span>' +
                    '<span>' + (available ? 'Available' : 'Not loaded') + '</span>';
    addonList.appendChild(div);

    log(addon.name + ': ' + (available ? 'loaded' : 'NOT AVAILABLE'), available ? 'success' : 'warn');
  });

  document.getElementById('addon-status').textContent =
    availableCount + '/' + addonInfo.length + ' addons loaded successfully';

  log('Initialization complete', 'success');
}

// ============================================
// Clipboard Tests
// ============================================

function clipboardCopyText() {
  if (!addons.clipboard) {
    log('Clipboard addon not available', 'error');
    return;
  }

  var text = document.getElementById('clipboard-text').value;
  try {
    addons.clipboard.setText(text);
    log('Copied text to clipboard: "' + text + '"', 'success');
    setOutput('clipboard-output', 'Copied: "' + text + '"');
  } catch (err) {
    log('Clipboard copy failed: ' + err.message, 'error');
    setOutput('clipboard-output', 'Error: ' + err.message);
  }
}

function clipboardPasteText() {
  if (!addons.clipboard) {
    log('Clipboard addon not available', 'error');
    return;
  }

  try {
    var text = addons.clipboard.getText();
    log('Pasted from clipboard: "' + text + '"', 'success');
    setOutput('clipboard-output', 'Pasted: "' + text + '"');
    document.getElementById('clipboard-text').value = text;
  } catch (err) {
    log('Clipboard paste failed: ' + err.message, 'error');
    setOutput('clipboard-output', 'Error: ' + err.message);
  }
}

function clipboardGetFormats() {
  if (!addons.clipboard) {
    log('Clipboard addon not available', 'error');
    return;
  }

  try {
    var formats = addons.clipboard.getFormats();
    var result = 'Available formats:\n' + JSON.stringify(formats, null, 2);
    log('Got clipboard formats', 'success');
    setOutput('clipboard-output', result);
  } catch (err) {
    log('Failed to get formats: ' + err.message, 'error');
    setOutput('clipboard-output', 'Error: ' + err.message);
  }
}

function clipboardClear() {
  if (!addons.clipboard) {
    log('Clipboard addon not available', 'error');
    return;
  }

  try {
    addons.clipboard.clear();
    log('Clipboard cleared', 'success');
    setOutput('clipboard-output', 'Clipboard cleared');
  } catch (err) {
    log('Clipboard clear failed: ' + err.message, 'error');
    setOutput('clipboard-output', 'Error: ' + err.message);
  }
}

// ============================================
// Folder Dialog Tests
// ============================================

function folderDialogOpen() {
  if (!addons.folderDialog) {
    log('Folder dialog addon not available', 'error');
    return;
  }

  try {
    var result = addons.folderDialog.open();
    if (result) {
      log('Selected folder: ' + result, 'success');
      setOutput('folder-output', 'Selected: ' + result);
    } else {
      log('Folder selection cancelled', 'info');
      setOutput('folder-output', 'Cancelled');
    }
  } catch (err) {
    log('Folder dialog failed: ' + err.message, 'error');
    setOutput('folder-output', 'Error: ' + err.message);
  }
}

function folderDialogOpenWithTitle() {
  if (!addons.folderDialog) {
    log('Folder dialog addon not available', 'error');
    return;
  }

  try {
    var result = addons.folderDialog.open({ title: 'Select a folder for testing' });
    if (result) {
      log('Selected folder: ' + result, 'success');
      setOutput('folder-output', 'Selected: ' + result);
    } else {
      log('Folder selection cancelled', 'info');
      setOutput('folder-output', 'Cancelled');
    }
  } catch (err) {
    log('Folder dialog failed: ' + err.message, 'error');
    setOutput('folder-output', 'Error: ' + err.message);
  }
}

// ============================================
// IPC Tests
// ============================================

function ipcCreateServer() {
  if (!addons.ipc) {
    log('IPC addon not available', 'error');
    return;
  }

  var pipeName = document.getElementById('ipc-pipe-name').value;

  try {
    if (ipcServer) {
      ipcServer.close();
    }

    ipcServer = addons.ipc.createServer(pipeName);

    ipcServer.on('connection', function() {
      log('IPC: Client connected', 'success');
      setOutput('ipc-output', 'Client connected to server');
    });

    ipcServer.on('message', function(msg) {
      log('IPC Server received: ' + msg, 'info');
      setOutput('ipc-output', 'Server received: ' + msg);
    });

    ipcServer.on('error', function(err) {
      log('IPC Server error: ' + err, 'error');
    });

    log('IPC Server created on pipe: ' + pipeName, 'success');
    setOutput('ipc-output', 'Server listening on: \\\\.\\pipe\\' + pipeName);
  } catch (err) {
    log('IPC Server creation failed: ' + err.message, 'error');
    setOutput('ipc-output', 'Error: ' + err.message);
  }
}

function ipcConnect() {
  if (!addons.ipc) {
    log('IPC addon not available', 'error');
    return;
  }

  var pipeName = document.getElementById('ipc-pipe-name').value;

  try {
    if (ipcClient) {
      ipcClient.close();
    }

    ipcClient = addons.ipc.connect(pipeName);

    ipcClient.on('message', function(msg) {
      log('IPC Client received: ' + msg, 'info');
      setOutput('ipc-output', 'Client received: ' + msg);
    });

    ipcClient.on('error', function(err) {
      log('IPC Client error: ' + err, 'error');
    });

    log('IPC Client connected to pipe: ' + pipeName, 'success');
    setOutput('ipc-output', 'Connected to: \\\\.\\pipe\\' + pipeName);
  } catch (err) {
    log('IPC Client connection failed: ' + err.message, 'error');
    setOutput('ipc-output', 'Error: ' + err.message);
  }
}

function ipcSend() {
  var message = document.getElementById('ipc-message').value;

  if (ipcClient) {
    try {
      ipcClient.send(message);
      log('IPC Client sent: ' + message, 'success');
    } catch (err) {
      log('IPC Send failed: ' + err.message, 'error');
    }
  } else if (ipcServer) {
    try {
      ipcServer.broadcast(message);
      log('IPC Server broadcast: ' + message, 'success');
    } catch (err) {
      log('IPC Broadcast failed: ' + err.message, 'error');
    }
  } else {
    log('No IPC connection active', 'warn');
  }
}

function ipcClose() {
  if (ipcClient) {
    ipcClient.close();
    ipcClient = null;
    log('IPC Client closed', 'info');
  }
  if (ipcServer) {
    ipcServer.close();
    ipcServer = null;
    log('IPC Server closed', 'info');
  }
  setOutput('ipc-output', 'IPC connections closed');
}

// ============================================
// Call-DLL Tests
// ============================================

function callDllMessageBox() {
  if (!addons.callDll) {
    log('Call-DLL addon not available', 'error');
    return;
  }

  try {
    var user32 = addons.callDll.load('user32.dll');
    var messageBoxA = user32.getFunction('MessageBoxA', 'stdcall', 'int32', ['int32', 'string', 'string', 'int32']);

    log('Calling MessageBoxA...', 'info');
    var result = messageBoxA(0, 'Hello from nwjs-addons!', 'Call-DLL Test', 0x40); // MB_ICONINFORMATION

    log('MessageBoxA returned: ' + result, 'success');
    setOutput('calldll-output', 'MessageBoxA returned: ' + result);
  } catch (err) {
    log('MessageBox failed: ' + err.message, 'error');
    setOutput('calldll-output', 'Error: ' + err.message);
  }
}

function callDllGetTickCount() {
  if (!addons.callDll) {
    log('Call-DLL addon not available', 'error');
    return;
  }

  try {
    var kernel32 = addons.callDll.load('kernel32.dll');
    var getTickCount = kernel32.getFunction('GetTickCount', 'stdcall', 'uint32', []);

    var ticks = getTickCount();
    var seconds = Math.floor(ticks / 1000);
    var minutes = Math.floor(seconds / 60);
    var hours = Math.floor(minutes / 60);

    var uptime = hours + 'h ' + (minutes % 60) + 'm ' + (seconds % 60) + 's';

    log('GetTickCount: ' + ticks + ' (' + uptime + ')', 'success');
    setOutput('calldll-output', 'System uptime: ' + ticks + ' ms\n(' + uptime + ')');
  } catch (err) {
    log('GetTickCount failed: ' + err.message, 'error');
    setOutput('calldll-output', 'Error: ' + err.message);
  }
}

function callDllBeep() {
  if (!addons.callDll) {
    log('Call-DLL addon not available', 'error');
    return;
  }

  try {
    var kernel32 = addons.callDll.load('kernel32.dll');
    var beep = kernel32.getFunction('Beep', 'stdcall', 'int32', ['uint32', 'uint32']);

    log('Playing beep...', 'info');
    var result = beep(800, 200); // 800 Hz for 200ms

    log('Beep returned: ' + result, 'success');
    setOutput('calldll-output', 'Beep played (800Hz, 200ms)\nReturned: ' + result);
  } catch (err) {
    log('Beep failed: ' + err.message, 'error');
    setOutput('calldll-output', 'Error: ' + err.message);
  }
}

// ============================================
// TinyCC Tests
// NOTE: TinyCC functions must use the jsbridge interface:
//   jsvalue func(jscontext ctx, jsvalue arg1, ...)
// Plain C functions like 'int add(int, int)' won't work directly.
// The jsbridge converts between JS values and C types.
// ============================================

function tinyccSimple() {
  if (!addons.tinycc || !addons.tinycc.isAvailable()) {
    log('TinyCC addon not available', 'error');
    return;
  }

  try {
    if (currentCompiler) {
      currentCompiler.release();
    }

    currentCompiler = addons.tinycc.create();

    // Use jsbridge-compatible function signature
    var code = [
      '#include "jsbridge.h"',
      '',
      'jsvalue add(jscontext ctx, jsvalue a, jsvalue b) {',
      '  int32 a_int = jsvalue_to_int32(ctx, a);',
      '  int32 b_int = jsvalue_to_int32(ctx, b);',
      '  return int32_to_jsvalue(ctx, a_int + b_int);',
      '}'
    ].join('\n');

    var success = currentCompiler.compile(code);

    if (success) {
      var addFn = currentCompiler.getFunction('add', 2);
      var result = addFn.call(10, 20);

      log('TinyCC: add(10, 20) = ' + result, 'success');
      setOutput('tinycc-output', 'Compiled simple function\nadd(10, 20) = ' + result);
    } else {
      var error = currentCompiler.getError();
      log('TinyCC compile failed: ' + error, 'error');
      setOutput('tinycc-output', 'Compile error: ' + error);
    }
  } catch (err) {
    log('TinyCC error: ' + err.message, 'error');
    setOutput('tinycc-output', 'Error: ' + err.message);
  }
}

function tinyccMath() {
  if (!addons.tinycc || !addons.tinycc.isAvailable()) {
    log('TinyCC addon not available', 'error');
    return;
  }

  try {
    if (currentCompiler) {
      currentCompiler.release();
    }

    currentCompiler = addons.tinycc.create();

    // Use jsbridge-compatible function signatures
    var code = [
      '#include "jsbridge.h"',
      '',
      '// Internal helpers (plain C)',
      'static int factorial_impl(int n) {',
      '  if (n <= 1) return 1;',
      '  return n * factorial_impl(n - 1);',
      '}',
      '',
      'static int fibonacci_impl(int n) {',
      '  if (n <= 1) return n;',
      '  int a = 0, b = 1, c;',
      '  int i;',
      '  for (i = 2; i <= n; i++) {',
      '    c = a + b;',
      '    a = b;',
      '    b = c;',
      '  }',
      '  return b;',
      '}',
      '',
      '// jsbridge-compatible wrappers',
      'jsvalue factorial(jscontext ctx, jsvalue n) {',
      '  int32 val = jsvalue_to_int32(ctx, n);',
      '  return int32_to_jsvalue(ctx, factorial_impl(val));',
      '}',
      '',
      'jsvalue fibonacci(jscontext ctx, jsvalue n) {',
      '  int32 val = jsvalue_to_int32(ctx, n);',
      '  return int32_to_jsvalue(ctx, fibonacci_impl(val));',
      '}'
    ].join('\n');

    var success = currentCompiler.compile(code);

    if (success) {
      var factorialFn = currentCompiler.getFunction('factorial', 1);
      var fibonacciFn = currentCompiler.getFunction('fibonacci', 1);

      var results = [];
      results.push('factorial(5) = ' + factorialFn.call(5));
      results.push('factorial(10) = ' + factorialFn.call(10));
      results.push('fibonacci(10) = ' + fibonacciFn.call(10));
      results.push('fibonacci(20) = ' + fibonacciFn.call(20));

      log('TinyCC: Math functions compiled and executed', 'success');
      setOutput('tinycc-output', 'Compiled math functions:\n' + results.join('\n'));
    } else {
      var error = currentCompiler.getError();
      log('TinyCC compile failed: ' + error, 'error');
      setOutput('tinycc-output', 'Compile error: ' + error);
    }
  } catch (err) {
    log('TinyCC error: ' + err.message, 'error');
    setOutput('tinycc-output', 'Error: ' + err.message);
  }
}

function tinyccCompileCustom() {
  if (!addons.tinycc || !addons.tinycc.isAvailable()) {
    log('TinyCC addon not available', 'error');
    return;
  }

  try {
    if (currentCompiler) {
      currentCompiler.release();
    }

    currentCompiler = addons.tinycc.create();

    var code = document.getElementById('tinycc-code').value;
    var success = currentCompiler.compile(code);

    if (success) {
      log('TinyCC: Custom code compiled successfully', 'success');
      setOutput('tinycc-output', 'Code compiled successfully!\nUse "Call Function" to execute.');
    } else {
      var error = currentCompiler.getError();
      log('TinyCC compile failed: ' + error, 'error');
      setOutput('tinycc-output', 'Compile error:\n' + error);
    }
  } catch (err) {
    log('TinyCC error: ' + err.message, 'error');
    setOutput('tinycc-output', 'Error: ' + err.message);
  }
}

function tinyccCallFunction() {
  if (!currentCompiler) {
    log('No code compiled - compile first', 'warn');
    return;
  }

  try {
    var funcName = document.getElementById('tinycc-func-name').value;
    var argsStr = document.getElementById('tinycc-args').value;

    var args = argsStr.split(',').map(function(s) {
      return parseInt(s.trim(), 10);
    }).filter(function(n) {
      return !isNaN(n);
    });

    // Use native types mode for plain C functions: int return, int args
    var argTypes = args.map(function() { return 'int'; });
    var fn = currentCompiler.getFunction(funcName, 'int', argTypes);
    var result = fn.call.apply(fn, args);

    log('TinyCC: ' + funcName + '(' + args.join(', ') + ') = ' + result, 'success');
    setOutput('tinycc-output', funcName + '(' + args.join(', ') + ') = ' + result);
  } catch (err) {
    log('TinyCC call failed: ' + err.message, 'error');
    setOutput('tinycc-output', 'Error: ' + err.message);
  }
}

// ============================================
// CSV Parser Tests
// ============================================

function csvParse() {
  if (!addons.csvParser) {
    log('CSV Parser addon not available', 'error');
    return;
  }

  try {
    var csvText = document.getElementById('csv-input').value;
    var result = addons.csvParser.parse(csvText);

    log('CSV: Parsed ' + result.length + ' rows', 'success');
    setOutput('csv-output', 'Parsed (' + result.length + ' rows):\n' + JSON.stringify(result, null, 2));
  } catch (err) {
    log('CSV parse failed: ' + err.message, 'error');
    setOutput('csv-output', 'Error: ' + err.message);
  }
}

function csvParseNoHeaders() {
  if (!addons.csvParser) {
    log('CSV Parser addon not available', 'error');
    return;
  }

  try {
    var csvText = document.getElementById('csv-input').value;
    var result = addons.csvParser.parse(csvText, { headers: false });

    log('CSV: Parsed ' + result.length + ' rows (no headers)', 'success');
    setOutput('csv-output', 'Parsed (' + result.length + ' rows):\n' + JSON.stringify(result, null, 2));
  } catch (err) {
    log('CSV parse failed: ' + err.message, 'error');
    setOutput('csv-output', 'Error: ' + err.message);
  }
}

function csvParseCustomDelimiter() {
  if (!addons.csvParser) {
    log('CSV Parser addon not available', 'error');
    return;
  }

  try {
    var csvText = document.getElementById('csv-input').value;
    var delimiter = document.getElementById('csv-delimiter').value || ',';
    var result = addons.csvParser.parse(csvText, { delimiter: delimiter });

    log('CSV: Parsed with delimiter "' + delimiter + '"', 'success');
    setOutput('csv-output', 'Parsed (' + result.length + ' rows):\n' + JSON.stringify(result, null, 2));
  } catch (err) {
    log('CSV parse failed: ' + err.message, 'error');
    setOutput('csv-output', 'Error: ' + err.message);
  }
}

function csvStringify() {
  if (!addons.csvParser) {
    log('CSV Parser addon not available', 'error');
    return;
  }

  try {
    var data = [
      { name: 'Alice', age: 30, city: 'New York' },
      { name: 'Bob', age: 25, city: 'Los Angeles' },
      { name: 'Charlie', age: 35, city: 'Chicago' }
    ];

    var result = addons.csvParser.stringify(data);

    log('CSV: Stringified ' + data.length + ' objects', 'success');
    setOutput('csv-output', 'Stringified:\n' + result);
  } catch (err) {
    log('CSV stringify failed: ' + err.message, 'error');
    setOutput('csv-output', 'Error: ' + err.message);
  }
}

function csvRoundTrip() {
  if (!addons.csvParser) {
    log('CSV Parser addon not available', 'error');
    return;
  }

  try {
    var csvText = document.getElementById('csv-input').value;

    // Parse -> Stringify -> Parse
    var parsed1 = addons.csvParser.parse(csvText);
    var stringified = addons.csvParser.stringify(parsed1);
    var parsed2 = addons.csvParser.parse(stringified);

    var match = JSON.stringify(parsed1) === JSON.stringify(parsed2);

    var output = 'Round-trip test: ' + (match ? 'PASS' : 'FAIL') + '\n\n';
    output += 'Original parsed:\n' + JSON.stringify(parsed1, null, 2) + '\n\n';
    output += 'Stringified:\n' + stringified + '\n';
    output += 'Re-parsed:\n' + JSON.stringify(parsed2, null, 2);

    log('CSV: Round-trip test ' + (match ? 'passed' : 'FAILED'), match ? 'success' : 'error');
    setOutput('csv-output', output);
  } catch (err) {
    log('CSV round-trip failed: ' + err.message, 'error');
    setOutput('csv-output', 'Error: ' + err.message);
  }
}

// ============================================
// SQLite3 Tests
// ============================================

function sqlite3Open() {
  if (!addons.sqlite3 || !addons.sqlite3.isAvailable()) {
    log('SQLite3 addon not available', 'error');
    return;
  }

  try {
    if (currentDb) {
      currentDb.close();
    }

    currentDb = new addons.sqlite3(':memory:');
    log('SQLite3: Opened in-memory database', 'success');
    setOutput('sqlite3-output', 'Database opened: :memory:');
  } catch (err) {
    log('SQLite3 open failed: ' + err.message, 'error');
    setOutput('sqlite3-output', 'Error: ' + err.message);
  }
}

function sqlite3OpenFile() {
  if (!addons.sqlite3 || !addons.sqlite3.isAvailable()) {
    log('SQLite3 addon not available', 'error');
    return;
  }

  try {
    if (currentDb) {
      currentDb.close();
    }

    // Get current directory - works in NW.js browser context
    var path = require('path');
    var currentDir = path.dirname(window.location.pathname.replace(/^\/([A-Z]:)/i, '$1'));
    var dbPath = path.join(currentDir, 'test.db');

    currentDb = new addons.sqlite3(dbPath);
    log('SQLite3: Opened file database: ' + dbPath, 'success');
    setOutput('sqlite3-output', 'Database opened: ' + dbPath);
  } catch (err) {
    log('SQLite3 open failed: ' + err.message, 'error');
    setOutput('sqlite3-output', 'Error: ' + err.message);
  }
}

function sqlite3Close() {
  if (!currentDb) {
    log('No database open', 'warn');
    return;
  }

  try {
    currentDb.close();
    currentDb = null;
    log('SQLite3: Database closed', 'success');
    setOutput('sqlite3-output', 'Database closed');
  } catch (err) {
    log('SQLite3 close failed: ' + err.message, 'error');
    setOutput('sqlite3-output', 'Error: ' + err.message);
  }
}

function sqlite3Exec() {
  if (!currentDb) {
    log('No database open - open one first', 'warn');
    return;
  }

  try {
    var sql = document.getElementById('sqlite3-sql').value;
    var statements = sql.split(';').filter(function(s) { return s.trim(); });
    var results = [];

    statements.forEach(function(stmt) {
      stmt = stmt.trim();
      if (!stmt) return;

      currentDb.exec(stmt);
      results.push('Executed: ' + stmt.substring(0, 50) + (stmt.length > 50 ? '...' : ''));
    });

    log('SQLite3: Executed ' + statements.length + ' statement(s)', 'success');
    setOutput('sqlite3-output', results.join('\n'));
  } catch (err) {
    log('SQLite3 exec failed: ' + err.message, 'error');
    setOutput('sqlite3-output', 'Error: ' + err.message);
  }
}

function sqlite3Query() {
  if (!currentDb) {
    log('No database open - open one first', 'warn');
    return;
  }

  try {
    var sql = document.getElementById('sqlite3-sql').value;

    // Find SELECT statement
    var selectMatch = sql.match(/SELECT[^;]+/i);
    if (!selectMatch) {
      log('No SELECT statement found', 'warn');
      setOutput('sqlite3-output', 'No SELECT statement found in SQL');
      return;
    }

    var stmt = currentDb.prepare(selectMatch[0]);
    var rows = stmt.all();

    log('SQLite3: Query returned ' + rows.length + ' row(s)', 'success');
    setOutput('sqlite3-output', 'Results (' + rows.length + ' rows):\n' + JSON.stringify(rows, null, 2));
  } catch (err) {
    log('SQLite3 query failed: ' + err.message, 'error');
    setOutput('sqlite3-output', 'Error: ' + err.message);
  }
}

function sqlite3Benchmark() {
  if (!addons.sqlite3 || !addons.sqlite3.isAvailable()) {
    log('SQLite3 addon not available', 'error');
    return;
  }

  try {
    // Use fresh in-memory db for benchmark
    var db = new addons.sqlite3(':memory:');
    db.exec('CREATE TABLE bench (id INTEGER PRIMARY KEY, value TEXT)');

    var insert = db.prepare('INSERT INTO bench (value) VALUES (?)');

    var start = Date.now();

    // Transaction for speed
    var insertMany = db.transaction(function() {
      for (var i = 0; i < 1000; i++) {
        insert.run('value_' + i);
      }
    });

    insertMany();

    var elapsed = Date.now() - start;

    // Verify
    var count = db.prepare('SELECT COUNT(*) as cnt FROM bench').get();

    db.close();

    var msg = '1000 inserts in ' + elapsed + 'ms\n' +
              'Rate: ' + Math.round(1000 / elapsed * 1000) + ' inserts/sec\n' +
              'Verified row count: ' + count.cnt;

    log('SQLite3 benchmark: ' + elapsed + 'ms for 1000 inserts', 'success');
    setOutput('sqlite3-output', msg);
  } catch (err) {
    log('SQLite3 benchmark failed: ' + err.message, 'error');
    setOutput('sqlite3-output', 'Error: ' + err.message);
  }
}

// ============================================
// Initialization on load
// ============================================

document.addEventListener('DOMContentLoaded', init);
