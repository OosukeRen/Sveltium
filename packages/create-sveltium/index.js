#!/usr/bin/env node
import * as prompts from '@clack/prompts'
import colors from 'picocolors'
import { userResponses } from './questions.js'
import { scaffold } from './scaffolding/scaffold.js'
import { dirHasContents } from './utils/file-utils.js'
import path from 'node:path'

const { green, yellow, red, cyan } = colors

async function main() {
  // Get target directory from command line argument
  const argTargetDir = process.argv[2]

  // Collect user responses
  const responses = await userResponses(argTargetDir)

  if (responses === null) {
    process.exit(0)
  }

  // Check if target directory already has contents
  const targetDir = path.resolve(process.cwd(), responses.projectName)
  const hasContents = await dirHasContents(targetDir)

  if (hasContents) {
    const shouldOverwrite = await prompts.confirm({
      message: yellow(`Directory "${responses.projectName}" is not empty. Overwrite?`),
    })

    if (prompts.isCancel(shouldOverwrite) || !shouldOverwrite) {
      prompts.cancel('Operation cancelled')
      process.exit(0)
    }
  }

  // Run scaffolding
  const spinner = prompts.spinner()
  spinner.start('Creating project...')

  try {
    await scaffold(responses)
    spinner.stop(green('Project created!'))
  } catch (error) {
    spinner.stop(red('Failed to create project'))
    console.error(error)
    process.exit(1)
  }

  // Success message
  prompts.outro(cyan('Done! Now run:'))
  console.log()
  console.log(`  cd ${responses.projectName}`)
  console.log(`  npm install`)
  console.log(`  npm run dev`)
  console.log()
}

main().catch((error) => {
  console.error(red('Error:'), error.message)
  process.exit(1)
})
