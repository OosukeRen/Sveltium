#!/usr/bin/env node
import fs from "node:fs";
import path from "node:path";
import nwbuild from "nw-builder";
import { loadConfig } from "./config-loader.js";
import {
  EXIT_SUCCESS,
  EXIT_FAILURE,
  hasArgument,
  runCommand,
  writeAppPackageJson,
  getProfileFromArgs,
  listProfiles,
  resolveLegacy,
} from "./shared.js";

const LIST_ARG_NAME = "list";
const NO_VITE_ARG_NAME = "no-vite";

const PLATFORM_ARCH_MAP = {
  win32: { platform: "win", arch: "ia32" },
  win64: { platform: "win", arch: "x64" },
  osx64: { platform: "osx", arch: "x64" },
  osxarm64: { platform: "osx", arch: "arm64" },
  linux32: { platform: "linux", arch: "ia32" },
  linux64: { platform: "linux", arch: "x64" },
  linuxarm64: { platform: "linux", arch: "arm64" },
};

const PLATFORM_DIR_SEPARATOR = "-";
const PACKAGE_NW_DIR_NAME = "package.nw";

function ensureDir(dirPath) {
  fs.mkdirSync(dirPath, { recursive: true });
}

function copyDir(src, dest) {
  ensureDir(dest);

  for (const entry of fs.readdirSync(src, { withFileTypes: true })) {
    const srcPath = path.join(src, entry.name);
    const destPath = path.join(dest, entry.name);

    if (entry.isDirectory()) {
      copyDir(srcPath, destPath);
      continue;
    }

    if (entry.isFile()) {
      fs.copyFileSync(srcPath, destPath);
    }
  }
}

function clearDirContents(dirPath) {
  for (const entry of fs.readdirSync(dirPath, { withFileTypes: true })) {
    const entryPath = path.join(dirPath, entry.name);

    if (entry.isDirectory()) {
      fs.rmSync(entryPath, { recursive: true, force: true });
      continue;
    }

    if (entry.isFile()) {
      fs.rmSync(entryPath, { force: true });
    }
  }
}

function hasNwRuntime(outputDir, platform) {
  // Check for files that are always present in NW.js builds
  // nw.exe gets renamed to app name, so check for other SDK files
  const markers = {
    win: ["nwjc.exe", "chromedriver.exe"],
    osx: ["nwjs.app"],
    linux: ["nwjc", "chromedriver"],
  };

  const platformMarkers = markers[platform] || [];

  for (const marker of platformMarkers) {
    if (fs.existsSync(path.join(outputDir, marker))) {
      return true;
    }
  }

  return false;
}

function resolveAppTargetDir(outputDir, platform) {
  let targetDir = outputDir;

  if (platform === "osx") {
    // Find the .app bundle
    const appBundle = fs.readdirSync(outputDir).find((f) => f.endsWith(".app"));

    if (appBundle) {
      targetDir = path.join(outputDir, appBundle, "Contents", "Resources", "app.nw");
    }
  } else {
    const packageDir = path.join(outputDir, PACKAGE_NW_DIR_NAME);
    const packageDirExists = fs.existsSync(packageDir);

    if (packageDirExists) {
      const packageDirStats = fs.statSync(packageDir);
      const isPackageDir = packageDirStats.isDirectory();

      if (isPackageDir) {
        targetDir = packageDir;
      }
    }
  }

  return targetDir;
}

function copyAppFiles(distDir, outputDir, platform) {
  // On Windows/Linux, app files usually go into package.nw when it exists
  // On macOS, they go inside the .app bundle's Resources/app.nw
  const targetDir = resolveAppTargetDir(outputDir, platform);

  clearDirContents(targetDir);

  for (const entry of fs.readdirSync(distDir, { withFileTypes: true })) {
    const srcPath = path.join(distDir, entry.name);
    const destPath = path.join(targetDir, entry.name);

    if (entry.isDirectory()) {
      if (fs.existsSync(destPath)) {
        fs.rmSync(destPath, { recursive: true, force: true });
      }

      copyDir(srcPath, destPath);
    } else if (entry.isFile()) {
      fs.copyFileSync(srcPath, destPath);
    }
  }
}

function resolveTargets(platforms) {
  const targets = [];
  const platformList = platforms || [];

  for (const platformKey of platformList) {
    const target = PLATFORM_ARCH_MAP[platformKey];

    if (!target) {
      console.error(`Unsupported platform: ${platformKey}`);
      process.exit(EXIT_FAILURE);
    }

    targets.push(target);
  }

  return targets;
}

async function buildProfile(profileName, config) {
  const profile = config.profiles?.[profileName];

  if (!profile) {
    console.error(`Unknown profile: ${profileName}`);
    listProfiles(config);
    process.exit(EXIT_FAILURE);
  }

  if (!hasArgument(NO_VITE_ARG_NAME)) {
    const buildEnv = {};
    const legacyEnabled = resolveLegacy(config);

    if (legacyEnabled) {
      buildEnv.LEGACY = "true";
    }

    runCommand("npm", ["run", "build"], process.cwd(), buildEnv);
  }

  const distDir = config.build.distDir;

  if (!fs.existsSync(distDir)) {
    console.error(`Missing dist folder: ${distDir}`);
    console.error("Run `npm run build` first or skip with --no-vite.");
    process.exit(EXIT_FAILURE);
  }

  writeAppPackageJson(distDir, config.app);
  ensureDir(config.build.cacheDir);

  const targets = resolveTargets(profile.platforms);
  const baseOutputDir = profile.outputDir || config.build.outputDir;
  const appIconPath = config.app.icon || "";
  const appConfig = {
    name: config.app.name,
    version: config.app.version,
    icon: appIconPath,
  };

  try {
    for (const target of targets) {
      let outputDir = baseOutputDir;
      const hasMultipleTargets = targets.length > 1;

      if (hasMultipleTargets) {
        outputDir = path.join(
          baseOutputDir,
          `${target.platform}${PLATFORM_DIR_SEPARATOR}${target.arch}`
        );
      }

      // Skip full nwbuild if runtime already exists in output
      if (hasNwRuntime(outputDir, target.platform)) {
        console.log(`NW.js runtime found in ${outputDir}, copying app files only...`);
        copyAppFiles(distDir, outputDir, target.platform);
        continue;
      }

      await nwbuild({
        mode: "build",
        srcDir: distDir,
        glob: false,
        version: profile.version,
        flavor: profile.flavor,
        platform: target.platform,
        arch: target.arch,
        cacheDir: config.build.cacheDir,
        outDir: outputDir,
        app: appConfig,
      });
    }
  } catch (error) {
    console.error(error);
    process.exit(EXIT_FAILURE);
  }
}

async function main() {
  const config = await loadConfig();

  if (hasArgument(LIST_ARG_NAME)) {
    listProfiles(config);
    process.exit(EXIT_SUCCESS);
  }

  const profileName = getProfileFromArgs(config);
  await buildProfile(profileName, config);
}

main();
