import { describe, it, before, after } from "node:test";
import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const TEMP_DIR = path.join(__dirname, ".test-temp-build");

const buildUrl = pathToFileURL(path.join(__dirname, "..", "build.js")).href;
const {
  ensureDir,
  copyDir,
  clearDirContents,
  findAppBundle,
  hasNwRuntime,
  resolveTargets,
} = await import(buildUrl);

function createTempDir(name) {
  const dir = path.join(TEMP_DIR, name);
  fs.mkdirSync(dir, { recursive: true });

  return dir;
}

describe("ensureDir", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("creates a nested directory structure", () => {
    const dir = path.join(TEMP_DIR, "ensure", "nested", "deep");
    ensureDir(dir);

    assert.equal(fs.existsSync(dir), true);
  });

  it("does not throw if directory already exists", () => {
    const dir = createTempDir("ensure-exists");

    assert.doesNotThrow(() => ensureDir(dir));
  });
});

describe("copyDir", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("copies files from source to destination", () => {
    const src = createTempDir("copy-src");
    const dest = path.join(TEMP_DIR, "copy-dest");
    fs.writeFileSync(path.join(src, "file.txt"), "hello");

    copyDir(src, dest);

    const content = fs.readFileSync(path.join(dest, "file.txt"), "utf-8");
    assert.equal(content, "hello");
  });

  it("copies nested directories recursively", () => {
    const src = createTempDir("copy-nested-src");
    const subDir = path.join(src, "sub");
    fs.mkdirSync(subDir);
    fs.writeFileSync(path.join(subDir, "nested.txt"), "nested");
    const dest = path.join(TEMP_DIR, "copy-nested-dest");

    copyDir(src, dest);

    const content = fs.readFileSync(path.join(dest, "sub", "nested.txt"), "utf-8");
    assert.equal(content, "nested");
  });

  it("creates destination directory if it does not exist", () => {
    const src = createTempDir("copy-create-src");
    fs.writeFileSync(path.join(src, "a.txt"), "a");
    const dest = path.join(TEMP_DIR, "copy-nonexistent-dest");

    copyDir(src, dest);

    assert.equal(fs.existsSync(dest), true);
  });
});

describe("clearDirContents", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("removes all files in directory", () => {
    const dir = createTempDir("clear-files");
    fs.writeFileSync(path.join(dir, "a.txt"), "a");
    fs.writeFileSync(path.join(dir, "b.txt"), "b");

    clearDirContents(dir);

    const entries = fs.readdirSync(dir);
    assert.equal(entries.length, 0);
  });

  it("removes subdirectories", () => {
    const dir = createTempDir("clear-subdirs");
    const sub = path.join(dir, "sub");
    fs.mkdirSync(sub);
    fs.writeFileSync(path.join(sub, "file.txt"), "x");

    clearDirContents(dir);

    const entries = fs.readdirSync(dir);
    assert.equal(entries.length, 0);
  });

  it("does not throw if directory does not exist", () => {
    const dir = path.join(TEMP_DIR, "clear-nonexistent");

    assert.doesNotThrow(() => clearDirContents(dir));
  });

  it("keeps the directory itself", () => {
    const dir = createTempDir("clear-keeps-dir");
    fs.writeFileSync(path.join(dir, "file.txt"), "x");

    clearDirContents(dir);

    assert.equal(fs.existsSync(dir), true);
  });
});

describe("findAppBundle", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("returns null for nonexistent directory", () => {
    const result = findAppBundle(path.join(TEMP_DIR, "no-such-dir"));

    assert.equal(result, null);
  });

  it("returns null when no .app bundle exists", () => {
    const dir = createTempDir("no-bundle");
    fs.writeFileSync(path.join(dir, "nw"), "");

    const result = findAppBundle(dir);

    assert.equal(result, null);
  });

  it("returns bundle name when .app directory exists", () => {
    const dir = createTempDir("has-bundle");
    fs.mkdirSync(path.join(dir, "MyApp.app"));

    const result = findAppBundle(dir);

    assert.equal(result, "MyApp.app");
  });
});

describe("hasNwRuntime", () => {
  before(() => {
    fs.mkdirSync(TEMP_DIR, { recursive: true });
  });

  after(() => {
    fs.rmSync(TEMP_DIR, { recursive: true, force: true });
  });

  it("returns false for empty directory", () => {
    const dir = createTempDir("runtime-empty");

    const result = hasNwRuntime(dir, "win");

    assert.equal(result, false);
  });

  it("detects Windows SDK markers", () => {
    const dir = createTempDir("runtime-win-sdk");
    fs.writeFileSync(path.join(dir, "nwjc.exe"), "");

    const result = hasNwRuntime(dir, "win");

    assert.equal(result, true);
  });

  it("detects Windows normal-flavor markers", () => {
    const dir = createTempDir("runtime-win-normal");
    fs.writeFileSync(path.join(dir, "icudtl.dat"), "");

    const result = hasNwRuntime(dir, "win");

    assert.equal(result, true);
  });

  it("detects Linux markers", () => {
    const dir = createTempDir("runtime-linux");
    fs.writeFileSync(path.join(dir, "chromedriver"), "");

    const result = hasNwRuntime(dir, "linux");

    assert.equal(result, true);
  });

  it("detects macOS via .app bundle", () => {
    const dir = createTempDir("runtime-osx");
    fs.mkdirSync(path.join(dir, "Test.app"));

    const result = hasNwRuntime(dir, "osx");

    assert.equal(result, true);
  });

  it("returns false for macOS without .app bundle", () => {
    const dir = createTempDir("runtime-osx-no-app");

    const result = hasNwRuntime(dir, "osx");

    assert.equal(result, false);
  });
});

describe("resolveTargets", () => {
  it("resolves win64 to platform/arch", () => {
    const targets = resolveTargets(["win64"]);

    assert.deepEqual(targets, [{ platform: "win", arch: "x64" }]);
  });

  it("resolves multiple platforms", () => {
    const targets = resolveTargets(["win64", "linux64"]);

    assert.equal(targets.length, 2);
    assert.deepEqual(targets[0], { platform: "win", arch: "x64" });
    assert.deepEqual(targets[1], { platform: "linux", arch: "x64" });
  });

  it("resolves all supported platform keys", () => {
    const allKeys = ["win32", "win64", "osx64", "osxarm64", "linux32", "linux64", "linuxarm64"];
    const targets = resolveTargets(allKeys);

    assert.equal(targets.length, allKeys.length);
  });

  it("throws for unsupported platform", () => {
    assert.throws(() => resolveTargets(["unsupported"]), {
      message: /Unsupported platform: unsupported/,
    });
  });

  it("returns empty array for empty input", () => {
    const targets = resolveTargets([]);

    assert.deepEqual(targets, []);
  });

  it("returns empty array for null input", () => {
    const targets = resolveTargets(null);

    assert.deepEqual(targets, []);
  });
});
