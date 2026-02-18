# nwjs-addons

Native C++ addons for NW.js applications, compiled into a single `.node` binary via [NAN](https://github.com/nodejs/nan) and [cmake-js](https://github.com/nicknisi/cmake-js). Ships a prebuilt binary for NW.js 0.12.3 (Windows XP, ia32) with source included for rebuilding against other NW.js versions.

## Addons

| Addon | Description |
|-------|-------------|
| **clipboard** | Full Windows clipboard access (text, files, images) |
| **folder-dialog** | Native folder/file open and save dialogs |
| **ipc** | Inter-process communication via named pipes + process monitoring |
| **call-dll** | Dynamic DLL loading and function calling (FFI) |
| **csv-parser** | CSV parsing and serialization |
| **rss-parser** | RSS/Atom feed parsing |
| **sdl2-input** | Joystick, gamepad, and mouse input via SDL2 |
| **nw-sqlite3** | Synchronous SQLite3 database (optional, requires sqlite3 amalgamation) |
| **tinycc** | Runtime C compilation via TinyCC (optional, requires libtcc) |

## Installation

```bash
npm install nwjs-addons
```

The package ships a prebuilt binary for NW.js 0.12.3 (ia32, Windows XP). If the prebuilt binary is present, `npm install` skips compilation. If not, it attempts to compile from source (requires build tools below).

## Usage

```js
// Import all addons
var addons = require('nwjs-addons')

// Or import individual addons
var clipboard = require('nwjs-addons/clipboard')
var folderDialog = require('nwjs-addons/folder-dialog')
var ipc = require('nwjs-addons/ipc')
var callDll = require('nwjs-addons/call-dll')
var csvParser = require('nwjs-addons/csv-parser')
var rssParser = require('nwjs-addons/rss-parser')
var sdl2Input = require('nwjs-addons/sdl2-input')
var sqlite3 = require('nwjs-addons/nw-sqlite3')
var tinycc = require('nwjs-addons/tinycc')
```

## Build profiles

The default build targets NW.js 0.12.3 with Windows XP compatibility (ia32, VS2015, v140_xp toolset):

```bash
npm run build           # default: NW.js 0.12.3, ia32, VS2015 v140_xp
npm run build:xp        # same as above (explicit name)
npm run build:modern    # NW.js 0.89.0, x64, system default compiler
npm run rebuild         # clean + build (XP target)
npm run rebuild:modern  # clean + build (modern target)
npm run build:debug     # debug build (XP target)
npm run clean           # remove build artifacts
```

To target a different NW.js version, modify the `--runtime-version` flag in the build script.

### Build requirements (source compilation)

- **Visual Studio 2015** with v140_xp toolset (for XP target) or any modern VS (for modern target)
- **CMake** (cmake-js uses it internally)
- **cmake-js 4.0.0**: `npm install -g cmake-js@4.0.0`
- **Windows SDK**

### NAN version compatibility

The package uses NAN 2.5.1 which supports V8 versions used by NW.js 0.12.3 through moderately recent versions. For very new NW.js versions, you may need to update NAN in `devDependencies` to a newer release.

## API Reference

### clipboard

```js
var clipboard = require('nwjs-addons/clipboard')

clipboard.getType()                    // 'empty' | 'text' | 'files' | 'image' | 'unknown'
clipboard.hasText()                    // boolean
clipboard.hasFiles()                   // boolean
clipboard.hasImage()                   // boolean
clipboard.getText()                    // string | null
clipboard.setText(text)                // boolean
clipboard.getFiles()                   // string[]
clipboard.copyFiles(paths)             // boolean
clipboard.cutFiles(paths)              // boolean
clipboard.pasteFiles(destDir)          // string[]
clipboard.isCutOperation()             // boolean
clipboard.getImageSize()               // { width, height } | null
clipboard.saveImageToFile(filePath)    // boolean (.png, .jpg, .bmp, .gif)
clipboard.getFormats()                 // { text, files, image }
clipboard.clear()                      // boolean
```

### folder-dialog

```js
var dialog = require('nwjs-addons/folder-dialog')

dialog.open({ title, initialPath })                              // string | null
dialog.openAsync({ title, initialPath })                         // Promise
dialog.openFile({ title, initialPath, filters, multiSelect })    // string | string[] | null
dialog.openFileAsync({ title, initialPath, filters, multiSelect })  // Promise
dialog.saveFile({ title, initialPath, defaultName, filters })    // string | null
dialog.saveFileAsync({ title, initialPath, defaultName, filters })  // Promise
```

Filters are alternating name/pattern pairs: `['Text Files', '*.txt', 'All Files', '*.*']`.

### ipc

```js
var ipc = require('nwjs-addons/ipc')

var server = ipc.createServer(name)     // Channel (server side)
var client = ipc.connect(name)          // Channel (client side)
ipc.isProcessRunning(pid)               // boolean
ipc.generateChannelName()               // string (UUID)
ipc.monitorProcess(pid)                 // ProcessMonitor
```

**Channel** (extends EventEmitter): `.connect()`, `.send(data)`, `.receive()`, `.receiveString(encoding?)`, `.close()`. Properties: `.name`, `.isServer`, `.connected`.

**ProcessMonitor** (extends EventEmitter): `.start(pollInterval?)`, `.stop()`. Emits `'exit'` when process terminates.

### call-dll

```js
var callDll = require('nwjs-addons/call-dll')

var dll = callDll.load('user32.dll')
var fn = dll.getFunction('MessageBoxA', 'stdcall', 'int32', ['int32', 'string', 'string', 'int32'])
fn(0, 'Hello', 'Title', 0)
dll.close()
```

Call conventions: `'cdecl'` (default), `'stdcall'`. Types: `'int32'`, `'uint32'`, `'void'`, `'string'`, `'pointer'`.

### csv-parser

```js
var csv = require('nwjs-addons/csv-parser')

csv.parse(csvString, { delimiter, quote, headers, trim })    // Object[] | Array[]
csv.parseFile(filePath, options)                              // Object[] | Array[]
csv.stringify(data, { delimiter, quote, headers })            // string
csv.writeFile(filePath, data, options)                        // boolean
```

### rss-parser

```js
var rss = require('nwjs-addons/rss-parser')

var feed = rss.parse(xmlString)       // { title, description, link, language, items[] }
var feed = rss.parseFile(filePath)    // same
```

### sdl2-input

```js
var sdl2 = require('nwjs-addons/sdl2-input')

sdl2.update()                          // pump SDL events (call once per frame)
sdl2.getJoysticks()                    // joystick info array
sdl2.getGameControllers()              // controller info array (mapped devices)
sdl2.getMouseState()                   // { x, y, left, middle, right, x1, x2 }
sdl2.getGlobalMouseState()             // screen coordinates
sdl2.getRelativeMouseState()           // delta since last call
sdl2.quit()                            // shut down SDL2

var joy = new sdl2.Joystick(index)     // raw joystick
joy.getState()                         // { axes[], buttons[], hats[] }
joy.rumble(lowFreq, highFreq, ms)      // boolean (0-65535)
joy.close()

var pad = new sdl2.GameController(index)  // Xbox-style controller
pad.getState()                            // standardized button/axis names
pad.rumble(lowFreq, highFreq, ms)
pad.getName()                             // string
pad.close()
```

### nw-sqlite3 (optional)

```js
var sqlite = require('nwjs-addons/nw-sqlite3')

if (!sqlite.isAvailable()) {
  console.log(sqlite.getLoadError())
}

var db = new sqlite.Database('test.db', { readonly: false })
db.exec('CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)')

var insert = db.prepare('INSERT INTO users (name) VALUES (?)')
insert.run('Alice')

var select = db.prepare('SELECT * FROM users')
select.all()     // [{ id: 1, name: 'Alice' }]

var wrapped = db.transaction(function() {
  insert.run('Bob')
  insert.run('Charlie')
})
wrapped()        // auto-commits; rolls back on error

db.close()
```

### tinycc (optional)

```js
var tinycc = require('nwjs-addons/tinycc')

if (!tinycc.isAvailable()) {
  console.log(tinycc.getLoadError())
}

var cc = tinycc.create({ useBuiltinRuntime: true })
cc.compile('int add(int a, int b) { return a + b; }')
var add = cc.getFunction('add', 'int', ['int', 'int'])
add(2, 3)  // 5
cc.release()
```

## Testing

The `addons-test/` directory contains a manual NW.js test application. To run it, open the app in NW.js 0.12.3:

```bash
cd packages/nwjs-addons/addons-test
path/to/nw.exe .
```

## License

MIT
