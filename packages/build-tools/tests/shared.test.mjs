import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const TEMP_DIR = path.join(__dirname, ".test-temp-shared");

const sharedUrl = pathToFileURL(path.join(__dirname, "..", "shared.js")).href;
const {
  detectPackageManager,
  resolveLegacy,
  readPackageVersion,
  DEFAULT_PROFILE_NAME,
  EXIT_SUCCESS,
  EXIT_FAILURE,
} = await import(sharedUrl);

function createTempDir(name) {
  const dir = path.join(TEMP_DIR, name);
  fs.mkdirSync(dir, { recursive: true });

  return dir;
}

describe("detectPackageManager", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  const lockfileCases = [
    { lockfile: "pnpm-lock.yaml", expected: "pnpm" },
    { lockfile: "yarn.lock", expected: "yarn" },
    { lockfile: "bun.lockb", expected: "bun" },
    { lockfile: "bun.lock", expected: "bun" },
    { lockfile: "package-lock.json", expected: "npm" },
  ];

  for (const { lockfile, expected } of lockfileCases) {
    it(`detects ${expected} from ${lockfile}`, () => {
      const dir = createTempDir(`pm-${lockfile.replace(".", "-")}`);
      fs.writeFileSync(path.join(dir, lockfile), "");

      const result = detectPackageManager(dir);

      assert.equal(result, expected);
    });
  }

  it("defaults to npm when no lockfile exists", () => {
    const dir = createTempDir("pm-none");

    const result = detectPackageManager(dir);

    assert.equal(result, "npm");
  });

  it("prioritizes pnpm over npm when both exist", () => {
    const dir = createTempDir("pm-priority");
    fs.writeFileSync(path.join(dir, "pnpm-lock.yaml"), "");
    fs.writeFileSync(path.join(dir, "package-lock.json"), "");

    const result = detectPackageManager(dir);

    assert.equal(result, "pnpm");
  });
});

describe("resolveLegacy", () => {
  it("returns false when neither CLI nor config enables legacy", () => {
    const result = resolveLegacy({ enableLegacy: false });

    assert.equal(result, false);
  });

  it("returns true when config enables legacy", () => {
    const result = resolveLegacy({ enableLegacy: true });

    assert.equal(result, true);
  });

  it("returns falsy when config.enableLegacy is undefined", () => {
    const result = resolveLegacy({});

    assert.ok(!result);
  });
});

describe("readPackageVersion", () => {
  it("returns a semver-like string", () => {
    const version = readPackageVersion();
    const semverPattern = /^\d+\.\d+\.\d+/;

    assert.match(version, semverPattern);
  });
});

describe("constants", () => {
  it("DEFAULT_PROFILE_NAME is 'dev'", () => {
    assert.equal(DEFAULT_PROFILE_NAME, "dev");
  });

  it("EXIT_SUCCESS is 0", () => {
    assert.equal(EXIT_SUCCESS, 0);
  });

  it("EXIT_FAILURE is 1", () => {
    assert.equal(EXIT_FAILURE, 1);
  });
});
