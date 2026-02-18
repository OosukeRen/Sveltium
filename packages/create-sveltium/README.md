# create-sveltium

CLI tool to scaffold [Sveltium](https://github.com/OosukeRen/Sveltium) projects — desktop apps built with NW.js, Svelte 5, and Vite.

## Usage

```bash
npm create sveltium
```

Or with a project name:

```bash
npm create sveltium my-app
```

You can also use `npx`:

```bash
npx @sveltium/create
```

## What it does

The CLI walks you through a few questions and generates a ready-to-develop project:

```
  Project name
  Enable legacy mode? (Chrome 40 support for older NW.js)
  Select a language (TypeScript / JavaScript)
  Add preconfigured custom ESLint rules?
  Setup auto generation with Sveltium build tools?
```

## Generated project structure

```
my-app/
  .gitignore
  index.html
  package.json
  svelte.config.js
  vite.config.js
  src/
    App.svelte
    app.css
    main.js (or main.ts)
    assets/
    lib/
```

Depending on your choices, the project may also include:

- `tsconfig.json`, `tsconfig.app.json`, `tsconfig.node.json` — when TypeScript is selected
- `jsconfig.json` — when JavaScript is selected
- `eslint.config.js` — when custom ESLint rules are enabled
- `sveltium.config.js` — when build tools auto-generation is enabled

## Options

### Legacy mode

Enables `@vitejs/plugin-legacy` targeting Chrome 40 for older NW.js versions (e.g., 0.12.x - 0.14.x). The plugin is conditionally loaded in `vite.config.js` via the `LEGACY=true` environment variable.

### Language

Choose between TypeScript and JavaScript. TypeScript projects include `svelte-check`, `@tsconfig/svelte`, and full tsconfig setup.

### Custom ESLint rules

Adds `@sveltium/eslint-rules` and `eslint-config-prettier` with a preconfigured flat config for ESLint 9+.

### Build tools auto-generation

Adds `@sveltium/build-tools` with `sveltium:build` and `sveltium:run` npm scripts, plus a `sveltium.config.js` where you define NW.js build profiles (versions, platforms, flavors).

## Requirements

- Node.js >= 16.0.0

## License

MIT
