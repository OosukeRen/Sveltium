import { userResponses } from './questions.js'

const responses = await userResponses()

if (responses) {
  console.log('\nCollected responses:')
  console.log(responses)
} else {
  console.log('\nCancelled')
}