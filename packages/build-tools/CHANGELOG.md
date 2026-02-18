# Changelog

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
