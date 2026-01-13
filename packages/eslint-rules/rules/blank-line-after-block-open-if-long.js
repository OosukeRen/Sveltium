const ZERO = 0;
const ONE = 1;
const MAX_LINES_BEFORE_ERROR = 2;
const NOT_FOUND_INDEX = -1;
const RANGE_START_INDEX = 0;
const RANGE_END_INDEX = 1;
const DEFAULT_MIN_INNER_LINES = 2;
const DEFAULT_TARGETS = ["function", "if"];
const FUNCTION_TARGET = "function";
const IF_TARGET = "if";
const DOUBLE_NEWLINE = "\n\n";
const DEFAULT_INDENT = "  ";

function getInnerLineCount(block) {



  const lineSpan = block.loc.end.line - block.loc.start.line;
  const innerLineCount = Math.max(ZERO, lineSpan - ONE);

  return innerLineCount;
}

function hasBlankLineAfterOpenBrace(sourceCode, openBrace, firstInsideToken) {



  const betweenText = sourceCode.text.slice(
    openBrace.range[RANGE_END_INDEX],
    firstInsideToken.range[RANGE_START_INDEX]
  );
  const newlineMatches = betweenText.match(/\n/g) || [];
  const newlineCount = newlineMatches.length;
  const hasBlankLine = newlineCount >= MAX_LINES_BEFORE_ERROR;

  return hasBlankLine;
}

function makeFix(sourceCode, openBrace, firstInsideToken) {



  const betweenText = sourceCode.text.slice(
    openBrace.range[RANGE_END_INDEX],
    firstInsideToken.range[RANGE_START_INDEX]
  );
  
  const firstNewlineIndex = betweenText.indexOf("\n");
  let fixFunction = null;

  if (firstNewlineIndex !== NOT_FOUND_INDEX) {
    const insertPos = openBrace.range[RANGE_END_INDEX] + firstNewlineIndex + ONE;
    fixFunction = (fixer) => fixer.insertTextAfterRange([insertPos, insertPos], "\n");
  }

  if (fixFunction === null) {
    fixFunction = (fixer) => fixer.insertTextAfter(openBrace, `${DOUBLE_NEWLINE}${DEFAULT_INDENT}`);
  }

  return fixFunction;
}

const rule = {
  meta: {
    type: "layout",
    docs: {
      description:
        "Require a blank line immediately after '{' when a block is longer than N inner lines.",
    },
    fixable: "whitespace",
    schema: [
      {
        type: "object",
        properties: {
          minInnerLines: { type: "integer", minimum: ZERO },
          targets: {
            type: "array",
            items: { enum: DEFAULT_TARGETS },
          },
        },
        additionalProperties: false,
      },
    ],
    messages: {
      needBlank:
        "Add a blank line after '{' for long blocks (more than {{min}} inner lines).",
    },
  },

  create(context) {

    const sourceCode = context.getSourceCode();
    const options = context.options[ZERO] || {};
    const minInnerLines = Number.isInteger(options.minInnerLines)
      ? options.minInnerLines
      : DEFAULT_MIN_INNER_LINES;
    const targets = new Set(options.targets || DEFAULT_TARGETS);

    function checkBlock(blockNode, reportNode) {



      let shouldCheck = Boolean(blockNode && blockNode.loc);

      if (shouldCheck) {
        const innerLineCount = getInnerLineCount(blockNode);
        shouldCheck = innerLineCount > minInnerLines;
      }

      if (shouldCheck) {



        const openBraceToken = sourceCode.getFirstToken(blockNode);
        const tokenAfterOpen = sourceCode.getTokenAfter(openBraceToken, {
          includeComments: true,
        });
        const fallbackToken = sourceCode.getLastToken(blockNode);
        const firstInsideToken = tokenAfterOpen || fallbackToken;

        shouldCheck = Boolean(openBraceToken && firstInsideToken);

        if (shouldCheck) {



          const hasBlankLine = hasBlankLineAfterOpenBrace(
            sourceCode,
            openBraceToken,
            firstInsideToken
          );

          if (!hasBlankLine) {



            context.report({
              node: reportNode,
              messageId: "needBlank",
              data: { min: String(minInnerLines) },
              fix: makeFix(sourceCode, openBraceToken, firstInsideToken),
            });
          }
        }
      }
    }

    return {
      FunctionDeclaration(node) {



        let shouldCheck = targets.has(FUNCTION_TARGET);

        if (shouldCheck) {



          const hasBlockBody = Boolean(node.body && node.body.type === "BlockStatement");

          if (hasBlockBody) {
            checkBlock(node.body, node);
          }
        }
      },
      FunctionExpression(node) {



        let shouldCheck = targets.has(FUNCTION_TARGET);

        if (shouldCheck) {



          const hasBlockBody = Boolean(node.body && node.body.type === "BlockStatement");

          if (hasBlockBody) {
            checkBlock(node.body, node);
          }
        }
      },
      ArrowFunctionExpression(node) {



        let shouldCheck = targets.has(FUNCTION_TARGET);

        if (shouldCheck) {



          const hasBlockBody = Boolean(node.body && node.body.type === "BlockStatement");

          if (hasBlockBody) {
            checkBlock(node.body, node);
          }
        }
      },
      IfStatement(node) {



        let shouldCheck = targets.has(IF_TARGET);

        if (shouldCheck) {



          const hasBlockBody = Boolean(
            node.consequent && node.consequent.type === "BlockStatement"
          );

          if (hasBlockBody) {
            checkBlock(node.consequent, node);
          }
        }
      },
    };
  },
};

export default rule;
