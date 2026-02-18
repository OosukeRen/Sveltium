'use strict'

var fs = require('fs')
var path = require('path')
var childProcess = require('child_process')

var PREBUILT_PATH = path.join(__dirname, 'build', 'Release', 'nwjs_addons.node')

var CMAKE_JS_ARGS = [
  'compile',
  '--runtime', 'nw',
  '--runtime-version', '0.12.3',
  '--arch', 'ia32',
  '-G', 'Visual Studio 14 2015',
  '-t', 'v140_xp'
]

var BUILD_REQUIREMENTS = [
  'Build requirements for nwjs-addons:',
  '  - Visual Studio 2015 with v140_xp toolset',
  '  - CMake (cmake-js uses it internally)',
  '  - cmake-js 4.0.0: npm install -g cmake-js@4.0.0',
  '  - Windows SDK',
  '',
  'To build manually: npm run build'
]

function main() {
  var prebuiltExists = fs.existsSync(PREBUILT_PATH)

  if (prebuiltExists) {
    console.log('nwjs-addons: prebuilt binary found, skipping compilation')
    process.exit(0)
  }

  console.log('nwjs-addons: no prebuilt binary found, attempting compilation...')

  var cmakeJsBin = path.join(__dirname, 'node_modules', '.bin', 'cmake-js')
  var cmakeJsExists = fs.existsSync(cmakeJsBin) || fs.existsSync(cmakeJsBin + '.cmd')

  if (!cmakeJsExists) {
    console.warn('nwjs-addons: cmake-js not found, skipping native compilation')
    console.warn(BUILD_REQUIREMENTS.join('\n'))
    process.exit(0)
  }

  var result = childProcess.spawnSync(cmakeJsBin, CMAKE_JS_ARGS, {
    cwd: __dirname,
    stdio: 'inherit',
    shell: true
  })

  var buildFailed = result.error || result.status !== 0

  if (buildFailed) {
    console.warn('')
    console.warn('nwjs-addons: native compilation failed (this is expected if build tools are not installed)')
    console.warn(BUILD_REQUIREMENTS.join('\n'))
    process.exit(0)
  }

  console.log('nwjs-addons: native compilation successful')
}

main()
