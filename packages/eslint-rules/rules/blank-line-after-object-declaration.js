const RANGE_START_INDEX = 0;
const RANGE_END_INDEX = 1;
const MIN_NEWLINES_FOR_BLANK_LINE = 2;

function hasBlankLineBetweenTokens(sourceCode, leftToken, rightToken) {
  const betweenText = sourceCode.text.slice(
    leftToken.range[RANGE_END_INDEX],
    rightToken.range[RANGE_START_INDEX]
  );
  const newlineMatches = betweenText.match(/\n/g) || [];
  const newlineCount = newlineMatches.length;
  const hasBlankLine = newlineCount >= MIN_NEWLINES_FOR_BLANK_LINE;

  return hasBlankLine;
}

function makeBlankLineAfterTokenFix(leftToken) {
  const fixFunction = (fixer) => fixer.insertTextAfter(leftToken, "\n");

  return fixFunction;
}

function hasObjectInitializer(declarations) {
  let hasMatch = false;

  for (const declaration of declarations) {
    const initializer = declaration.init;
    const isObjectInit = Boolean(initializer && initializer.type === "ObjectExpression");

    if (isObjectInit) {
      hasMatch = true;
    }
  }

  return hasMatch;
}

const rule = {
  meta: {
    type: "layout",
    docs: {
      description: "Require a blank line after variable declarations with object literals.",
    },
    fixable: "whitespace",
    schema: [],
    messages: {
      needBlank: "Add a blank line after object literal declarations.",
    },
  },

  create(context) {
    const sourceCode = context.getSourceCode();

    function checkDeclaration(node) {
      const hasObjectInit = hasObjectInitializer(node.declarations);
      let shouldCheck = hasObjectInit;
      let lastToken = null;
      let nextToken = null;

      if (shouldCheck) {
        lastToken = sourceCode.getLastToken(node);
        nextToken = sourceCode.getTokenAfter(node, { includeComments: true });
        shouldCheck = Boolean(lastToken && nextToken);
      }

      if (shouldCheck) {
        const hasBlankLine = hasBlankLineBetweenTokens(sourceCode, lastToken, nextToken);

        if (!hasBlankLine) {
          context.report({
            node,
            messageId: "needBlank",
            fix: makeBlankLineAfterTokenFix(lastToken),
          });
        }
      }
    }

    return {
      VariableDeclaration(node) {
        checkDeclaration(node);
      },
    };
  },
};

export default rule;
