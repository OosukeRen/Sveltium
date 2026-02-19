# Changelog

## 0.2.0

### Added

- **Dual NAN/N-API backend**: All addon source files now use `addon_api.h` macros that compile against either NAN (legacy) or N-API via node-addon-api (modern), selected at build time
- `src/addon_api.h` — router header selecting backend based on `USE_NAPI` / `USE_NAN` defines
- `src/addon_api_nan.h` — NAN 2.5.1 backend macro definitions
- `src/addon_api_napi.h` — N-API (node-addon-api 3.x) backend macro definitions
- `build:napi` and `rebuild:napi` scripts for N-API builds (NW.js 0.89.0, x64)
- `node-addon-api` added to devDependencies

### Changed

- All 9 addon modules migrated from direct NAN calls to `ADDON_*` macros
- CMakeLists.txt updated with `USE_NAPI`/`USE_NAN` build-time selection and node-addon-api include paths
- tinycc jsbridge ported for dual NAN/N-API compilation (napi_ref storage, ADDON_* macros for value conversion)
- tinycc module.cpp fixed for x64 calling convention (uintptr_t slot type, proper 64-bit value packing)
- Vendor libraries restructured with x86/x64 subdirectories (SDL2-static.lib, libtcc)
- README updated with dual-backend architecture docs and build profile table

## 0.1.1

### Fixed

- Repository URL now points to the correct monorepo with `directory` field
- Removed unused `bindings` dependency (all JS wrappers use direct `require` paths)
- IPC test now uses `Channel.send()` instead of non-existent `broadcast()` method
- SDL2 include/library paths no longer reference monorepo-external `temp/vendored/` directory

### Added

- `install.js` smart install script: skips compilation when prebuilt binary exists, gracefully handles missing build tools
- SDL2 headers and static library bundled in `sdl2-input/vendor/SDL2/` for self-contained source builds
- `src/` and `CMakeLists.txt` included in npm tarball for source rebuilds
- Build profiles: `build:xp` (NW.js 0.12.3, ia32, VS2015) and `build:modern` (NW.js 0.89.0, x64)
- `rebuild:modern` script for clean modern rebuilds
- README with full API reference, build instructions, and NAN compatibility notes
- This changelog

### Changed

- Install script changed from raw `cmake-js compile` to `node install.js` (prebuilt-aware)
- Default `build` script uses `cmake-js compile` instead of `cmake-js build`

## 0.1.0

Initial release with 9 native addons:

- **clipboard** — full Windows clipboard access (text, files, images)
- **folder-dialog** — native folder/file open and save dialogs
- **ipc** — inter-process communication via named pipes + process monitoring
- **call-dll** — dynamic DLL loading and function calling (FFI)
- **csv-parser** — CSV parsing and serialization
- **rss-parser** — RSS/Atom feed parsing
- **sdl2-input** — joystick, gamepad, and mouse input via SDL2
- **nw-sqlite3** — synchronous SQLite3 database (optional)
- **tinycc** — runtime C compilation via TinyCC (optional)

Built with NAN 2.5.1 and cmake-js 4.0.0, targeting NW.js 0.12.3 (V8 4.1, Windows XP compatible).
