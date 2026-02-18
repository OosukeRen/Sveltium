# Changelog

## 1.0.1

### Fixed

- Templates are now bundled inside the package, fixing broken scaffolding when installed from npm
- Fixed typo in JavaScript template directory name (`svelitum` -> `sveltium`)
- Fixed `tsconfig.node.json` referencing nonexistent `vite.config.ts` (now correctly points to `vite.config.js`)
- Added missing `@sveltium/eslint-rules` and `eslint-config-prettier` to generated ESLint devDependencies
- Regenerated stale `package-lock.json` with correct package name and dependency structure

### Added

- `engines` field requiring Node.js >= 16.0.0
- `@sveltium/build-tools` wired into auto-generation devDependencies
- `AppConfig` and `BuildConfig` typedefs in generated `sveltium.config.js` for optional overrides
- Author metadata
- This changelog and README

### Changed

- Removed stale `templates/*` entry from `pnpm-workspace.yaml`

## 1.0.0

Initial release.
