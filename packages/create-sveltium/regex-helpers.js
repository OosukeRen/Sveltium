import VerEx from 'verbal-expressions'

// ============================================================================
// Trailing Slashes Pattern
// Matches one or more forward slashes at the end of a string
// ============================================================================
const trailingSlashesPattern = VerEx()
  .find('/')
  .oneOrMore()
  .endOfLine()

export const removeTrailingSlashes = (str) => {
  return str.replace(trailingSlashesPattern, '')
}

// ============================================================================
// Whitespace Pattern
// Matches one or more whitespace characters
// ============================================================================
const whitespacePattern = VerEx()
  .whitespace()
  .oneOrMore()

export const replaceWhitespace = (str, replacement = '-') => {
  const globalPattern = new RegExp(whitespacePattern.source, 'g')
  return str.replace(globalPattern, replacement)
}

// ============================================================================
// Leading Dot or Underscore Pattern
// Matches a dot or underscore at the start of a string
// ============================================================================
const leadingDotOrUnderscorePattern = VerEx()
  .startOfLine()
  .anyOf('._')

export const removeLeadingDotOrUnderscore = (str) => {
  return str.replace(leadingDotOrUnderscorePattern, '')
}

// ============================================================================
// Valid Package Name Characters Pattern
// Matches valid npm package name characters: a-z, 0-9, dash, tilde
// ============================================================================
const validPackageChars = VerEx()
  .range('a', 'z')
  .or()
  .range('0', '9')
  .or()
  .anyOf('-~')

// For invalid chars, we need to negate - VerEx doesn't support [^...] directly
// So we filter by keeping only valid characters
export const removeInvalidPackageChars = (str, replacement = '-') => {
  let result = ''
  let invalidRun = ''

  for (const char of str) {
    if (validPackageChars.test(char)) {
      if (invalidRun.length > 0) {
        result += replacement
        invalidRun = ''
      }
      result += char
    } else {
      invalidRun += char
    }
  }

  if (invalidRun.length > 0) {
    result += replacement
  }

  return result
}

// ============================================================================
// Valid Package Name Pattern
// Validates npm package names according to npm conventions
// Supports scoped packages: @scope/package-name
// ============================================================================

// Scope part: @[a-z0-9-*~][a-z0-9-*._~]*/
const scopeFirstChar = VerEx().range('a', 'z').or().range('0', '9').or().anyOf('-*~')
const scopeRestChars = VerEx().range('a', 'z').or().range('0', '9').or().anyOf('-*._~')

// Package name part: [a-z0-9-~][a-z0-9-._~]*
const packageFirstChar = VerEx().range('a', 'z').or().range('0', '9').or().anyOf('-~')
const packageRestChars = VerEx().range('a', 'z').or().range('0', '9').or().anyOf('-._~')

export const isValidPackageName = (name) => {
  if (!name || name.length === 0) return false

  let remaining = name

  // Check for scoped package (@scope/name)
  if (remaining.startsWith('@')) {
    remaining = remaining.slice(1)
    const slashIndex = remaining.indexOf('/')

    if (slashIndex === -1) return false

    const scope = remaining.slice(0, slashIndex)
    remaining = remaining.slice(slashIndex + 1)

    if (scope.length === 0) return false
    if (!scopeFirstChar.test(scope[0])) return false

    for (const char of scope.slice(1)) {
      if (!scopeRestChars.test(char)) return false
    }
  }

  // Validate package name part
  if (remaining.length === 0) return false
  if (!packageFirstChar.test(remaining[0])) return false

  for (const char of remaining.slice(1)) {
    if (!packageRestChars.test(char)) return false
  }

  return true
}

// ============================================================================
// Utility: Convert to Valid Package Name
// Combines multiple transformations to create a valid npm package name
// ============================================================================
export const toValidPackageName = (name) => {
  let result = name.trim().toLowerCase()
  result = replaceWhitespace(result, '-')
  result = removeLeadingDotOrUnderscore(result)
  result = removeInvalidPackageChars(result, '-')
  return result
}

// ============================================================================
// Utility: Format Target Directory
// Trims whitespace and removes trailing slashes
// ============================================================================
export const formatTargetDir = (dir) => {
  return removeTrailingSlashes(dir.trim())
}
