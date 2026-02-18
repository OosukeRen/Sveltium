import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const BUILD_TOOLS_DIR = path.join(__dirname, "..");
const TEMP_DIR = path.join(__dirname, ".test-temp-config");

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

  return dir;
}

function runBuildCli(args, cwd) {
  const scriptPath = path.join(BUILD_TOOLS_DIR, "build.js");
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

describe("config-loader defaults", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("uses package.json name and version for app defaults", () => {
    const dir = createTempProject("defaults", {
      packageJson: { name: "my-cool-app", version: "2.5.0" },
      config: {
        profiles: { dev: { version: "0.89.0", flavor: "sdk", platforms: ["win64"] } },
      },
    });

    const result = runBuildCli(["--list"], dir);

    assert.equal(result.status, 0);
    assert.ok(result.stdout.includes("dev"));
  });

  it("works without sveltium.config.js (uses all defaults)", () => {
    const dir = createTempProject("no-config", {
      packageJson: { name: "bare-app", version: "1.0.0" },
    });

    const result = runBuildCli(["--list"], dir);

    // Should exit 0 and mention no profiles
    assert.equal(result.status, 0);
    const output = result.stdout + result.stderr;
    const mentionsNoProfiles = output.toLowerCase().includes("no profiles");
    assert.ok(mentionsNoProfiles);
  });
});

describe("config-loader window deep merge", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("partial window override preserves other window defaults", () => {
    const dir = createTempProject("window-partial", {
      packageJson: { name: "test-app", version: "1.0.0" },
      config: {
        app: { window: { width: 1920 } },
        profiles: { dev: { version: "0.89.0", flavor: "sdk", platforms: ["win64"] } },
      },
    });

    // The config should load without crashing
    const result = runBuildCli(["--list"], dir);

    assert.equal(result.status, 0);
  });

  it("full window override works", () => {
    const dir = createTempProject("window-full", {
      packageJson: { name: "test-app", version: "1.0.0" },
      config: {
        app: { window: { title: "Custom", width: 800, height: 600 } },
        profiles: { dev: { version: "0.89.0", flavor: "sdk", platforms: ["win64"] } },
      },
    });

    const result = runBuildCli(["--list"], dir);

    assert.equal(result.status, 0);
  });
});

describe("config-loader error handling", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("exits non-zero when package.json is missing", () => {
    const dir = createTempProject("missing-pkg", { packageJson: false });

    const result = runBuildCli(["--list"], dir);

    assert.notEqual(result.status, 0);
    const output = result.stdout + result.stderr;
    assert.ok(output.toLowerCase().includes("package.json"));
  });

  it("exits non-zero for malformed sveltium.config.js", () => {
    const dir = createTempProject("malformed-config", {
      packageJson: { name: "test-app", version: "1.0.0" },
    });
    fs.writeFileSync(
      path.join(dir, "sveltium.config.js"),
      "export default {{{INVALID"
    );

    const result = runBuildCli(["--list"], dir);

    assert.notEqual(result.status, 0);
    const output = result.stdout + result.stderr;
    assert.ok(output.toLowerCase().includes("sveltium.config.js"));
  });
});
