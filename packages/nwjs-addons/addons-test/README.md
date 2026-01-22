# NW.js Addons Test Suite

Interactive test application for all nwjs-addons.

## Requirements

- NW.js 0.12.3 (ia32)
- Built nwjs_addons.node (run `npx cmake-js build --arch ia32` in parent directory)

## Running the Tests

### Option 1: Using nw directly

```bash
# From the addons-test directory
path/to/nw.exe .

# Or from the nwjs-addons directory
path/to/nw.exe addons-test
```

### Option 2: Copy files to NW.js folder

1. Copy the entire `addons-test` folder to your NW.js directory
2. Copy `build/Release/nwjs_addons.node` to `addons-test/`
3. Copy `tinycc/lib/libtcc.dll` to `addons-test/`
4. Run `nw.exe addons-test`

## Test Features

### Clipboard
- Copy/paste text
- Get available clipboard formats
- Clear clipboard

### Folder Dialog
- Open native folder selection dialog
- Open with custom title

### IPC (Named Pipes)
- Create IPC server
- Connect IPC client
- Send messages between processes

### Call-DLL (FFI)
- MessageBox (user32.dll)
- GetTickCount (kernel32.dll)
- Beep (kernel32.dll)

### TinyCC (Runtime C Compiler)
- Compile simple functions
- Compile math functions (factorial, fibonacci)
- Compile custom C code
- Call compiled functions with arguments

### SQLite3
- Open in-memory database
- Open file database
- Execute SQL statements
- Query with SELECT
- Benchmark (1000 inserts)

## Troubleshooting

### "Module not found" errors
Make sure the addon is built:
```bash
cd packages/nwjs-addons
npx cmake-js build --arch ia32
```

### TinyCC "addon not available"
- Ensure `libtcc.dll` is in the same directory as `nwjs_addons.node` or in PATH
- The TinyCC runtime files should be in `tinycc/runtime/`

### SQLite3 issues
The SQLite3 amalgamation is compiled into the addon, so no external dependencies are needed.
