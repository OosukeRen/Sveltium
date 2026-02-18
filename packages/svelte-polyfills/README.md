# @sveltium/polyfills

DOM and Proxy polyfills for legacy browser support in [Sveltium](https://github.com/OosukeRen/Sveltium) NW.js applications. Targets older Chromium builds (e.g., Chrome 40 in NW.js 0.12.x).

## Installation

```bash
npm install @sveltium/polyfills
```

## Usage

Import once at the top of your entry point — all polyfills are applied as side effects:

```js
import "@sveltium/polyfills";
```

Or import individual modules:

```js
import "@sveltium/polyfills/dom.js";
import "@sveltium/polyfills/proxy.min.js";
```

You can also load them via script tags in `index.html`:

```html
<script type="module" src="/node_modules/@sveltium/polyfills/dom.js"></script>
<script type="module" src="/node_modules/@sveltium/polyfills/proxy.min.js"></script>
```

## What's included

### DOM polyfills (`dom.js`)

All patches are conditional — they only activate when the native API is missing.

| Polyfill | Targets |
|---|---|
| `before()` | `Element`, `CharacterData`, `DocumentType` |
| `after()` | `Element`, `CharacterData`, `DocumentType` |
| `remove()` | `Element`, `CharacterData`, `DocumentType` |
| `replaceWith()` | `Element`, `CharacterData`, `DocumentType` |
| `append()` | `Element`, `Document`, `DocumentFragment` |
| `prepend()` | `Element`, `Document`, `DocumentFragment` |
| `matches()` | `Element` (falls back to `msMatchesSelector` / `webkitMatchesSelector`) |
| `closest()` | `Element` |
| `forEach()` | `NodeList` |
| `CustomEvent` constructor | `window` |
| `classList.toggle` force parameter | `DOMTokenList` |

### Proxy polyfill (`proxy.min.js`)

A minimal ES6 `Proxy` polyfill for environments without native support.

**Supported traps:** `get`, `set`, `apply`, `construct`

**Supported features:**
- `new Proxy(target, handler)`
- `Proxy.revocable(target, handler)`
- Object, array, and function targets

**Limitations:**
- Cannot intercept property access on properties that didn't exist on the target at proxy creation time
- Traps beyond `get`/`set`/`apply`/`construct` are not supported and will throw

The polyfill only installs itself when `globalThis.Proxy` is not already defined.

## License

MIT
