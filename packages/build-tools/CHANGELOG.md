# Changelog

## 0.2.0

### Fixed

- Unhandled promise rejections in both CLIs (added `.catch()` to `main()`)
- Profile validation: `version` and `flavor` are now required fields, `platforms` must be non-empty
- `clearDirContents` no longer throws on missing directories
- `runCommand` now reports spawn errors and signal terminations
- Error catch blocks use `error.message` instead of raw error objects
- `app.window` config is now deep merged (partial overrides preserve defaults)
- Build paths (`distDir`, `cacheDir`, `outputDir`) are resolved with `path.resolve`
- `copyAppFiles` no longer wipes NW.js runtime when `package.nw` is missing
- `resolveTargets` throws instead of calling `process.exit` for better error flow

### Added

- `--help` flag for both `sveltium-build` and `sveltium-run`
- `--version` flag for both CLIs
- `--list` flag for `sveltium-run` (was only in build)
- Default profile fallback logging when no profile flag is given
- "Available profiles:" header in `listProfiles` output
- Progress messages during build and run operations
- `bun.lock` detection (new Bun lockfile format)
- Warning when profile names conflict with reserved CLI flags
- Normal-flavor NW.js runtime detection (`icudtl.dat`, `nw_elf.dll`, `libnw.so`)
- Symlink handling in `copyDir`, `clearDirContents`, and `copyAppFiles`
- Exported build helpers for testability
- Unit tests with `node:test` (44 tests across 3 files)
- Test scripts in `package.json` (`test`, `test:cli`, `test:all`)

### Changed

- Extracted `runViteBuild` and `validateDistDir` to `shared.js` (DRY)
- Extracted `buildTargets` from `buildProfile` for readability
- Replaced `hasAppBundle` with `findAppBundle` to fix TOCTOU race condition
- Simplified `resolveAppTargetDir` nested conditionals
- Renamed internal variables for clarity (`hasValue` -> `isPresent`, etc.)

## 0.1.1

### Fixed

- Config loader now shows friendly error messages instead of raw stack traces when `package.json` is missing or malformed
- Config loader catches syntax errors in `sveltium.config.js` with a clear message
- macOS `hasNwRuntime` now detects any `.app` bundle, not just the default `nwjs.app` name
- macOS `copyAppFiles` exits with an error if no `.app` bundle is found instead of silently wiping the output directory
- `run.js` now explicitly exits after NW.js closes, preventing potential process hangs

### Added

- `engines` field requiring Node.js >= 18.0.0
- Package manager auto-detection from lockfiles (pnpm, yarn, bun, npm) for the Vite build step
- `listProfiles` now prints a helpful message when no profiles are defined
- This README and changelog

### Changed

- Bumped `nw-builder` dependency from `^4.0.0` to `^4.17.0` to match the API features used

## 0.1.0

Initial release with:

- `sveltium-build` CLI for packaging NW.js apps
- `sveltium-run` CLI for local development
- `sveltium.config.js` loader with sensible defaults
- Profile-based build configuration
- Legacy browser support via `LEGACY` env var
- NW.js runtime caching and incremental rebuilds
