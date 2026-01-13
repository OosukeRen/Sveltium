import path from 'node:path'
import * as prompts from '@clack/prompts'
import colors from 'picocolors'
import {
  formatTargetDir,
  isValidPackageName,
  toValidPackageName,
} from './regex-helpers.js'

const { blue, cyan, yellow } = colors

const YES_NO_QUESTION = [
  { value: true, label: 'Yes' },
  { value: false, label: 'No' },
]

const defaultTargetDir = 'sveltium-project'

/**
 * Cancel handler for prompts
 */
const cancel = () => prompts.cancel('Operation cancelled')

// ============================================================================
// Question 1: Project Name
// ============================================================================
const askProjectName = async (argTargetDir) => {
  if (argTargetDir) {
    return formatTargetDir(argTargetDir)
  }

  const projectName = await prompts.text({
    message: 'Project name:',
    defaultValue: defaultTargetDir,
    placeholder: defaultTargetDir,
    validate: (value) => {
      return value.length === 0 || formatTargetDir(value).length > 0
        ? undefined
        : 'Invalid project name'
    },
  })

  if (prompts.isCancel(projectName)) {
    cancel()
    return null
  }

  return formatTargetDir(projectName)
}

// ============================================================================
// Question 2: Enable Legacy Mode
// ============================================================================
const askEnableLegacy = async () => {
  const enableLegacy = await prompts.select({
    message: 'Enable legacy mode?',
    options: YES_NO_QUESTION
  })

  if (prompts.isCancel(enableLegacy)) {
    cancel()
    return null
  }

  return enableLegacy
}

// ============================================================================
// Question 3: Language Selection (JavaScript or TypeScript)
// ============================================================================
const LANGUAGE_OPTIONS = [
  {
    value: 'typescript',
    label: blue('TypeScript'),
    hint: 'recommended',
  },
  {
    value: 'javascript',
    label: yellow('JavaScript'),
  },
]

const askLanguage = async () => {
  const language = await prompts.select({
    message: 'Select a language:',
    options: LANGUAGE_OPTIONS,
  })

  if (prompts.isCancel(language)) {
    cancel()
    return null
  }

  return language
}

// ============================================================================
// Question 4: Custom ESLint Rules
// ============================================================================
const askCustomEslintRules = async () => {
  const useCustomRules = await prompts.select({
    message: 'Add preconfigured custom ESLint rules?',
    options: YES_NO_QUESTION
  })

  if (prompts.isCancel(useCustomRules)) {
    cancel()
    return null
  }

  return useCustomRules
}

// ============================================================================
// Question 5: Setup Auto Generation Script
// ============================================================================
const askAutoGeneration = async () => {
  const autoGeneration = await prompts.select({
    message: 'Setup auto generation script with Sveltium build tools?',
    options: YES_NO_QUESTION
  })

  if (prompts.isCancel(autoGeneration)) {
    cancel()
    return null
  }

  return autoGeneration
}

// ============================================================================
// Main: Collect All User Responses
// ============================================================================
export const userResponses = async (argTargetDir) => {
  prompts.intro(cyan('Create Sveltium'))

  // Question 1: Project name
  const projectName = await askProjectName(argTargetDir)
  if (projectName === null) return null

  // Derive package name from project name
  let packageName = path.basename(path.resolve(projectName))

  if (!isValidPackageName(packageName)) {
    const packageNameResult = await prompts.text({
      message: 'Package name:',
      defaultValue: toValidPackageName(packageName),
      placeholder: toValidPackageName(packageName),
      validate(dir) {
        if (!isValidPackageName(dir)) {
          return 'Invalid package.json name'
        }
      },
    })

    if (prompts.isCancel(packageNameResult)) {
      cancel()
      return null
    }

    packageName = packageNameResult
  }

  // Question 2: Enable legacy mode
  const enableLegacy = await askEnableLegacy()

  // Question 3: Language selection
  const language = await askLanguage()

  // Question 4: Custom ESLint rules
  const customEslintRules = await askCustomEslintRules()

  // Question 5: Auto generation setup
  const autoGeneration = await askAutoGeneration()

  if([enableLegacy, language, customEslintRules, autoGeneration].includes(null)) {
    return null;
  }
  
  return {
    projectName,
    packageName,
    enableLegacy,
    language,
    customEslintRules,
    autoGeneration,
  }
}

export {
  askProjectName,
  askEnableLegacy,
  askLanguage,
  askCustomEslintRules,
  askAutoGeneration,
  formatTargetDir,
  isValidPackageName,
  toValidPackageName,
  defaultTargetDir,
  cancel,
}