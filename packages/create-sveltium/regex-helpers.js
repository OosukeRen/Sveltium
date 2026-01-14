// ============================================================================
// Trailing Slashes Pattern
// Matches one or more forward slashes at the end of a string
// ============================================================================
const trailingSlashesPattern = /\/+$/

export const removeTrailingSlashes = (str) => {
  return str.replace(trailingSlashesPattern, '')
}

// ============================================================================
// Whitespace Pattern
// Matches one or more whitespace characters
// ============================================================================
const whitespacePattern = /\s+/g

export const replaceWhitespace = (str, replacement = '-') => {
  return str.replace(whitespacePattern, replacement)
}

// ============================================================================
// Leading Dot or Underscore Pattern
// Matches a dot or underscore at the start of a string
// ============================================================================
const leadingDotOrUnderscorePattern = /^[._]/

export const removeLeadingDotOrUnderscore = (str) => {
  return str.replace(leadingDotOrUnderscorePattern, '')
}

// ============================================================================
// Valid Package Name Characters Pattern
// Matches valid npm package name characters: a-z, 0-9, dash, tilde
// ============================================================================
const validPackageCharPattern = /^[a-z0-9\-~]$/

export const removeInvalidPackageChars = (str, replacement = '-') => {
  let result = ''
  let hasInvalidRun = false

  for (const char of str) {
    const isValid = validPackageCharPattern.test(char)

    if (isValid) {
      if (hasInvalidRun) {
        result += replacement
        hasInvalidRun = false
      }
      result += char
    } else {
      hasInvalidRun = true
    }
  }

  return result
}

// ============================================================================
// Valid Package Name Pattern
// Validates npm package names according to npm conventions
// Supports scoped packages: @scope/package-name
// ============================================================================

// First char patterns (more restrictive)
const scopeFirstCharPattern = /^[a-z0-9\-*~]$/
const scopeRestCharPattern = /^[a-z0-9\-*._~]$/
const packageFirstCharPattern = /^[a-z0-9\-~]$/
const packageRestCharPattern = /^[a-z0-9\-._~]$/

export const isValidPackageName = (name) => {
  if (!name || name.length === 0) {
    return false
  }

  let remaining = name

  // Check for scoped package (@scope/name)
  if (remaining.startsWith('@')) {
    remaining = remaining.slice(1)
    const slashIndex = remaining.indexOf('/')

    if (slashIndex === -1) {
      return false
    }

    const scope = remaining.slice(0, slashIndex)
    remaining = remaining.slice(slashIndex + 1)

    if (scope.length === 0) {
      return false
    }

    const scopeFirstCharValid = scopeFirstCharPattern.test(scope[0])
    if (!scopeFirstCharValid) {
      return false
    }

    for (const char of scope.slice(1)) {
      const charValid = scopeRestCharPattern.test(char)
      if (!charValid) {
        return false
      }
    }
  }

  // Validate package name part
  if (remaining.length === 0) {
    return false
  }

  const firstCharValid = packageFirstCharPattern.test(remaining[0])
  if (!firstCharValid) {
    return false
  }

  for (const char of remaining.slice(1)) {
    const charValid = packageRestCharPattern.test(char)
    if (!charValid) {
      return false
    }
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
