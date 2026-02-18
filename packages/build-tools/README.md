# @sveltium/build-tools

Build and run tools for [Sveltium](https://github.com/OosukeRen/Sveltium) NW.js applications. Provides two CLI commands that read a shared `sveltium.config.js` from your project root.

## Installation

```bash
npm install --save-dev @sveltium/build-tools
```

Requires Node.js >= 18.0.0.

## CLI commands

### `sveltium-build`

Builds your app for distribution. Runs `vite build`, then packages the output with nw-builder for each platform target defined in the active profile.

```bash
npx sveltium-build              # builds using the "dev" profile
npx sveltium-build --prod       # builds using the "prod" profile
npx sveltium-build --list       # lists all available profiles
npx sveltium-build --no-vite    # skips the vite build step
npx sveltium-build --legacy     # enables legacy browser support
```

### `sveltium-run`

Launches your app locally with NW.js. Runs `vite build` first (unless skipped), then opens the app using the NW.js version from the active profile.

```bash
npx sveltium-run                # runs using the "dev" profile
npx sveltium-run --prod         # runs using the "prod" profile
npx sveltium-run --no-vite      # skips the vite build step
npx sveltium-run --legacy       # enables legacy browser support
```

## Configuration

Both commands read `sveltium.config.js` from `process.cwd()`. The file should use a default ESM export:

```js
/** @type {import('@sveltium/build-tools').SveltiumConfig} */
export default {
  enableLegacy: false,

  profiles: {
    dev: {
      version: "0.89.0",
      flavor: "sdk",
      platforms: ["win64"],
    },
    prod: {
      version: "0.89.0",
      flavor: "normal",
      platforms: ["win64", "linux64", "osx64"],
    },
  },
};
```

### Config fields

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `enableLegacy` | `boolean` | `false` | Enable legacy browser support (sets `LEGACY=true` env var for Vite) |
| `profiles` | `object` | `{}` | Named build profiles (see below) |
| `app` | `object` | from `package.json` | Override app metadata (name, version, main, window, icon) |
| `build` | `object` | see below | Override build directories |

### App defaults (from your project's `package.json`)

| Field | Default |
|-------|---------|
| `app.name` | `package.json` name |
| `app.version` | `package.json` version |
| `app.main` | `"index.html"` |
| `app.window.title` | `package.json` name |
| `app.window.width` | `1024` |
| `app.window.height` | `768` |

### Build defaults

| Field | Default |
|-------|---------|
| `build.distDir` | `"dist"` |
| `build.cacheDir` | `"nw_cache"` |
| `build.outputDir` | `"builds"` |

### Profile fields

| Field | Type | Description |
|-------|------|-------------|
| `version` | `string` | NW.js version to use (e.g. `"0.89.0"`) |
| `flavor` | `string` | `"sdk"` or `"normal"` |
| `platforms` | `string[]` | Target platform keys |
| `outputDir` | `string` | Override base output directory for this profile |

### Platform keys

| Key | OS | Architecture |
|-----|----|-------------|
| `win32` | Windows | x86 |
| `win64` | Windows | x64 |
| `osx64` | macOS | x64 |
| `osxarm64` | macOS | ARM64 |
| `linux32` | Linux | x86 |
| `linux64` | Linux | x64 |
| `linuxarm64` | Linux | ARM64 |

## Package manager detection

Both commands automatically detect your project's package manager from lockfiles (`pnpm-lock.yaml`, `yarn.lock`, `bun.lockb`, `package-lock.json`) and use it for the Vite build step. Falls back to `npm` if no lockfile is found.

## License

MIT
