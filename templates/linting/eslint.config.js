import localRules from "@sveltium/eslint-rules/index.js";
import prettier from "eslint-config-prettier";

export default [
  {
    plugins: {
      sveltium: localRules,
    },
    rules: {
      // Sveltium Linting rules:
      "sveltium/blank-line-after-block-open-if-long": [
        "error",
        { minInnerLines: 4, targets: ["function", "if"] },
      ],
      "sveltium/blank-line-after-object-declaration": "error",
      // add whatever other linting rules you'd want ...
    },
  },

  // prevents ESLint formatting rules from fighting Prettier
  prettier,
];
