# @sveltium/eslint-rules

Custom ESLint layout rules for [Sveltium](https://github.com/OosukeRen/Sveltium) projects. Enforces consistent whitespace patterns around blocks and object declarations.

Requires ESLint 9+ (flat config).

## Installation

```bash
npm install -D @sveltium/eslint-rules eslint
```

## Usage

```js
// eslint.config.js
import sveltiumRules from "@sveltium/eslint-rules/index.js";

export default [
  {
    plugins: {
      sveltium: sveltiumRules,
    },
    rules: {
      "sveltium/blank-line-after-block-open-if-long": ["error", { minInnerLines: 4, targets: ["function", "if"] }],
      "sveltium/blank-line-after-object-declaration": "error",
    },
  },
];
```

## Rules

### `blank-line-after-object-declaration`

Enforces a blank line after any variable declaration that uses an object literal initializer. Auto-fixable.

```js
// Bad
const config = { a: 1 };
doSomething();

// Good
const config = { a: 1 };

doSomething();
```

No options.

### `blank-line-after-block-open-if-long`

Enforces a blank line after the opening `{` of a block when the block exceeds a configurable line count. Auto-fixable.

```js
// Bad (block has > 2 inner lines, no blank line after {)
function doThing() {
  const a = 1;
  const b = 2;
  const c = 3;
  return a + b + c;
}

// Good
function doThing() {

  const a = 1;
  const b = 2;
  const c = 3;
  return a + b + c;
}
```

#### Options

| Option | Type | Default | Description |
|---|---|---|---|
| `minInnerLines` | integer | `2` | Minimum inner lines before the rule triggers |
| `targets` | array | `["function", "if"]` | Which block types to check: `"function"` (declarations, expressions, arrows) and/or `"if"` |

## License

MIT
