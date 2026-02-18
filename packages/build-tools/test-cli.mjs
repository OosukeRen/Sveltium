#!/usr/bin/env node
import fs from "node:fs";
import path from "node:path";
import { spawnSync } from "node:child_process";
import { fileURLToPath, pathToFileURL } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const TEMP_DIR = path.join(__dirname, ".test-temp");

let passed = 0;
let failed = 0;

function test(name, fn) {
  try {
    fn();
    console.log(`  PASS: ${name}`);
    passed++;
  } catch (err) {
    console.log(`  FAIL: ${name} — ${err.message}`);
    failed++;
  }
}

function assert(condition, message) {
  if (!condition) {
    throw new Error(message);
  }
}

function createTempProject(name, opts = {}) {
  const dir = path.join(TEMP_DIR, name);
  fs.mkdirSync(dir, { recursive: true });

  if (opts.packageJson !== false) {
    const defaultPkg = { name: "test-app", version: "1.0.0" };
    const pkg = opts.packageJson || defaultPkg;
    fs.writeFileSync(path.join(dir, "package.json"), JSON.stringify(pkg, null, 2));
  }

  if (opts.config) {
    const configContent = `export default ${JSON.stringify(opts.config, null, 2)};`;
    fs.writeFileSync(path.join(dir, "sveltium.config.js"), configContent);
  }

  if (opts.configRaw) {
    fs.writeFileSync(path.join(dir, "sveltium.config.js"), opts.configRaw);
  }

  if (opts.lockfile) {
    fs.writeFileSync(path.join(dir, opts.lockfile), "");
  }

  return dir;
}

function cleanup() {
  if (fs.existsSync(TEMP_DIR)) {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  }
}

function runCli(script, args, cwd) {
  const scriptPath = path.join(__dirname, script);
  const result = spawnSync("node", [scriptPath, ...args], {
    cwd,
    encoding: "utf-8",
    timeout: 15000,
  });

  return {
    stdout: result.stdout || "",
    stderr: result.stderr || "",
    status: result.status,
  };
}

// ---------------------------------------------------------------------------
// Test suites
// ---------------------------------------------------------------------------

function testPackageJson() {
  console.log("\n--- package.json validation ---");

  const pkg = JSON.parse(fs.readFileSync(path.join(__dirname, "package.json"), "utf-8"));

  test("has engines field with node constraint", () => {
    assert(pkg.engines, "engines field missing");
    assert(pkg.engines.node, "engines.node missing");
  });

  test("nw-builder pinned to >= 4.17.0", () => {
    const version = pkg.dependencies["nw-builder"];
    assert(version, "nw-builder dependency missing");
    assert(
      !version.includes("4.0.0"),
      `nw-builder version too low: ${version}`
    );
  });

  test("bin entries defined", () => {
    assert(pkg.bin["sveltium-build"], "missing sveltium-build bin");
    assert(pkg.bin["sveltium-run"], "missing sveltium-run bin");
  });

  test("all bin targets exist on disk", () => {
    for (const [name, file] of Object.entries(pkg.bin)) {
      const filePath = path.join(__dirname, file);
      assert(fs.existsSync(filePath), `bin ${name} -> ${file} not found`);
    }
  });

  test("all files entries exist on disk", () => {
    for (const file of pkg.files) {
      const filePath = path.join(__dirname, file);
      assert(fs.existsSync(filePath), `files entry ${file} not found`);
    }
  });
}

function testNpmPack() {
  console.log("\n--- npm pack --dry-run ---");

  const result = spawnSync("npm", ["pack", "--dry-run"], {
    cwd: __dirname,
    encoding: "utf-8",
    shell: true,
    timeout: 15000,
  });

  const output = result.stdout + result.stderr;

  test("npm pack succeeds", () => {
    assert(result.status === 0, `npm pack failed: ${result.stderr}`);
  });

  const requiredFiles = ["build.js", "run.js", "config-loader.js", "shared.js"];
  for (const file of requiredFiles) {
    test(`pack includes ${file}`, () => {
      assert(output.includes(file), `${file} missing from pack output`);
    });
  }

  test("pack does NOT include test-cli.mjs", () => {
    assert(!output.includes("test-cli.mjs"), "test-cli.mjs should not be packed");
  });
}

function testNoStaleReferences() {
  console.log("\n--- stale reference checks ---");

  const sourceFiles = ["build.js", "run.js", "config-loader.js", "shared.js"];

  for (const file of sourceFiles) {
    const content = fs.readFileSync(path.join(__dirname, file), "utf-8");

    test(`${file}: no reference to packager.config`, () => {
      assert(!content.includes("packager.config"), `stale packager.config in ${file}`);
    });

    test(`${file}: no reference to ../../package.json`, () => {
      assert(!content.includes("../../package.json"), `stale ../../package.json in ${file}`);
    });
  }
}

function testConfigMissingPackageJson() {
  console.log("\n--- config: missing package.json ---");

  const dir = createTempProject("no-pkg", { packageJson: false });
  const result = runCli("build.js", ["--list"], dir);

  test("exits non-zero", () => {
    assert(result.status !== 0, `expected non-zero exit, got ${result.status}`);
  });

  test("mentions package.json in output", () => {
    const output = result.stdout + result.stderr;
    assert(
      output.toLowerCase().includes("package.json"),
      "output should mention package.json"
    );
  });

  test("no raw ENOENT stack trace", () => {
    const output = result.stdout + result.stderr;
    const hasRawStack = output.includes("at readFileSync") || output.includes("at Object.readFileSync");
    assert(!hasRawStack, "should not show raw Node.js stack trace");
  });
}

function testConfigMissingSveltiumConfig() {
  console.log("\n--- config: missing sveltium.config.js ---");

  const dir = createTempProject("no-config", {
    packageJson: { name: "test-app", version: "2.0.0" },
  });
  const result = runCli("build.js", ["--list"], dir);

  test("exits 0 (uses defaults)", () => {
    assert(result.status === 0, `expected exit 0, got ${result.status}: ${result.stderr}`);
  });
}

function testConfigMalformed() {
  console.log("\n--- config: malformed sveltium.config.js ---");

  const dir = createTempProject("bad-config", {
    packageJson: { name: "test-app", version: "1.0.0" },
    configRaw: "export default {{{INVALID SYNTAX",
  });
  const result = runCli("build.js", ["--list"], dir);

  test("exits non-zero", () => {
    assert(result.status !== 0, `expected non-zero exit, got ${result.status}`);
  });

  test("mentions sveltium.config.js in output", () => {
    const output = result.stdout + result.stderr;
    assert(
      output.toLowerCase().includes("sveltium.config.js"),
      "output should mention sveltium.config.js"
    );
  });
}

function testConfigValidProfiles() {
  console.log("\n--- config: valid project with profiles ---");

  const dir = createTempProject("valid-profiles", {
    packageJson: { name: "my-app", version: "3.0.0" },
    config: {
      profiles: {
        dev: { version: "0.89.0", flavor: "sdk", platforms: ["win64"] },
        prod: { version: "0.89.0", flavor: "normal", platforms: ["win64", "linux64"] },
      },
    },
  });
  const result = runCli("build.js", ["--list"], dir);

  test("exits 0", () => {
    assert(result.status === 0, `expected exit 0, got ${result.status}: ${result.stderr}`);
  });

  test("lists dev profile", () => {
    assert(result.stdout.includes("dev"), "stdout should include 'dev'");
  });

  test("lists prod profile", () => {
    assert(result.stdout.includes("prod"), "stdout should include 'prod'");
  });
}

function testConfigEmptyProfiles() {
  console.log("\n--- config: empty profiles ---");

  const dir = createTempProject("empty-profiles", {
    packageJson: { name: "test-app", version: "1.0.0" },
    config: { profiles: {} },
  });
  const result = runCli("build.js", ["--list"], dir);

  test("exits 0", () => {
    assert(result.status === 0, `expected exit 0, got ${result.status}: ${result.stderr}`);
  });

  test("output mentions no profiles", () => {
    const output = result.stdout + result.stderr;
    const mentionsNoProfiles =
      output.toLowerCase().includes("no profiles") ||
      output.toLowerCase().includes("no profile");
    assert(mentionsNoProfiles, `expected 'no profiles' message, got: ${output.trim()}`);
  });
}

function testProfileMissingVersion() {
  console.log("\n--- profile: missing version ---");

  const dir = createTempProject("no-version", {
    packageJson: { name: "test-app", version: "1.0.0" },
    config: {
      profiles: {
        dev: { flavor: "sdk", platforms: ["win64"] },
      },
    },
  });

  const buildResult = runCli("build.js", ["--no-vite"], dir);

  test("build exits non-zero", () => {
    assert(buildResult.status !== 0, `expected non-zero exit, got ${buildResult.status}`);
  });

  test("build mentions missing version", () => {
    const output = buildResult.stdout + buildResult.stderr;
    assert(output.includes("version"), "output should mention 'version'");
  });

  const runResult = runCli("run.js", ["--no-vite"], dir);

  test("run exits non-zero", () => {
    assert(runResult.status !== 0, `expected non-zero exit, got ${runResult.status}`);
  });

  test("run mentions missing version", () => {
    const output = runResult.stdout + runResult.stderr;
    assert(output.includes("version"), "output should mention 'version'");
  });
}

function testProfileMissingFlavor() {
  console.log("\n--- profile: missing flavor ---");

  const dir = createTempProject("no-flavor", {
    packageJson: { name: "test-app", version: "1.0.0" },
    config: {
      profiles: {
        dev: { version: "0.89.0", platforms: ["win64"] },
      },
    },
  });

  const buildResult = runCli("build.js", ["--no-vite"], dir);

  test("build exits non-zero", () => {
    assert(buildResult.status !== 0, `expected non-zero exit, got ${buildResult.status}`);
  });

  test("build mentions missing flavor", () => {
    const output = buildResult.stdout + buildResult.stderr;
    assert(output.includes("flavor"), "output should mention 'flavor'");
  });

  const runResult = runCli("run.js", ["--no-vite"], dir);

  test("run exits non-zero", () => {
    assert(runResult.status !== 0, `expected non-zero exit, got ${runResult.status}`);
  });

  test("run mentions missing flavor", () => {
    const output = runResult.stdout + runResult.stderr;
    assert(output.includes("flavor"), "output should mention 'flavor'");
  });
}

function testProfileEmptyPlatforms() {
  console.log("\n--- profile: empty platforms ---");

  const dir = createTempProject("empty-platforms", {
    packageJson: { name: "test-app", version: "1.0.0" },
    config: {
      profiles: {
        dev: { version: "0.89.0", flavor: "sdk", platforms: [] },
      },
    },
  });

  const result = runCli("build.js", ["--no-vite"], dir);

  test("build exits non-zero", () => {
    assert(result.status !== 0, `expected non-zero exit, got ${result.status}`);
  });

  test("build mentions no platforms", () => {
    const output = result.stdout + result.stderr;
    assert(output.includes("platforms"), "output should mention 'platforms'");
  });
}

async function testPackageManagerDetection() {
  console.log("\n--- package manager detection ---");

  const sharedUrl = pathToFileURL(path.join(__dirname, "shared.js")).href;
  const { detectPackageManager } = await import(sharedUrl);

  const cases = [
    { lockfile: "pnpm-lock.yaml", expected: "pnpm" },
    { lockfile: "yarn.lock", expected: "yarn" },
    { lockfile: "bun.lockb", expected: "bun" },
    { lockfile: "package-lock.json", expected: "npm" },
  ];

  for (const { lockfile, expected } of cases) {
    const dir = createTempProject(`pm-${expected}`, { lockfile });

    test(`detects ${expected} from ${lockfile}`, () => {
      const detected = detectPackageManager(dir);
      assert(
        detected === expected,
        `expected "${expected}", got "${detected}"`
      );
    });
  }

  const emptyDir = createTempProject("pm-default");
  test("defaults to npm when no lockfile", () => {
    const detected = detectPackageManager(emptyDir);
    assert(detected === "npm", `expected "npm", got "${detected}"`);
  });
}

// ---------------------------------------------------------------------------
// Run all tests
// ---------------------------------------------------------------------------

cleanup();
fs.mkdirSync(TEMP_DIR, { recursive: true });

try {
  testPackageJson();
  testNpmPack();
  testNoStaleReferences();
  testConfigMissingPackageJson();
  testConfigMissingSveltiumConfig();
  testConfigMalformed();
  testConfigValidProfiles();
  testConfigEmptyProfiles();
  testProfileMissingVersion();
  testProfileMissingFlavor();
  testProfileEmptyPlatforms();
  await testPackageManagerDetection();
} finally {
  cleanup();
}

console.log(`\n=== Results: ${passed} passed, ${failed} failed ===`);
const exitCode = failed > 0 ? 1 : 0;
process.exit(exitCode);
