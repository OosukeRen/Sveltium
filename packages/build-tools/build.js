#!/usr/bin/env node
import fs from "node:fs";
import path from "node:path";
import nwbuild from "nw-builder";
import { loadConfig } from "./config-loader.js";
import {
  EXIT_SUCCESS,
  EXIT_FAILURE,
  hasArgument,
  writeAppPackageJson,
  getProfileFromArgs,
  listProfiles,
  readPackageVersion,
  printHelp,
  runViteBuild,
  validateDistDir,
} from "./shared.js";

const LIST_ARG_NAME = "list";
const HELP_ARG_NAME = "help";
const VERSION_ARG_NAME = "version";

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
    } else if (entry.isFile()) {
      fs.copyFileSync(srcPath, destPath);
    }
  }
}

function clearDirContents(dirPath) {
  if (!fs.existsSync(dirPath)) {
    return;
  }

  for (const entry of fs.readdirSync(dirPath, { withFileTypes: true })) {
    const entryPath = path.join(dirPath, entry.name);

    if (entry.isDirectory()) {
      fs.rmSync(entryPath, { recursive: true, force: true });
    } else if (entry.isFile()) {
      fs.rmSync(entryPath, { force: true });
    }
  }
}

function findAppBundle(outputDir) {
  if (!fs.existsSync(outputDir)) {
    return null;
  }

  const bundleName = fs.readdirSync(outputDir).find((entry) => entry.endsWith(".app"));
  return bundleName || null;
}

function hasNwRuntime(outputDir, platform) {
  // Check for files that are always present in NW.js builds
  // nw.exe gets renamed to app name, so check for other SDK files
  const markersByPlatform = {
    win: ["nwjc.exe", "chromedriver.exe"],
    linux: ["nwjc", "chromedriver"],
  };

  // macOS: the .app bundle may be renamed, so check for any .app directory
  if (platform === "osx") {
    return findAppBundle(outputDir) !== null;
  }

  const markers = markersByPlatform[platform] || [];

  for (const marker of markers) {
    if (fs.existsSync(path.join(outputDir, marker))) {
      return true;
    }
  }

  return false;
}

function resolveAppTargetDir(outputDir, platform, appBundleName) {
  let targetDir = outputDir;

  if (platform === "osx") {
    if (appBundleName) {
      targetDir = path.join(outputDir, appBundleName, "Contents", "Resources", "app.nw");
    }
  } else {
    const packageDir = path.join(outputDir, PACKAGE_NW_DIR_NAME);
    const isPackageDir = fs.existsSync(packageDir) && fs.statSync(packageDir).isDirectory();

    if (isPackageDir) {
      targetDir = packageDir;
    }
  }

  return targetDir;
}

function copyAppFiles(distDir, outputDir, platform) {
  // On Windows/Linux, app files usually go into package.nw when it exists
  // On macOS, they go inside the .app bundle's Resources/app.nw
  const isMacOS = platform === "osx";
  const appBundleName = isMacOS ? findAppBundle(outputDir) : null;
  const macOSBundleMissing = isMacOS && !appBundleName;

  if (macOSBundleMissing) {
    throw new Error(`No .app bundle found in ${outputDir}. Cannot copy app files.`);
  }

  const targetDir = resolveAppTargetDir(outputDir, platform, appBundleName);

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

  if (!profile.version) {
    console.error(`Profile "${profileName}" is missing required field: version`);
    process.exit(EXIT_FAILURE);
  }

  if (!profile.flavor) {
    console.error(`Profile "${profileName}" is missing required field: flavor`);
    process.exit(EXIT_FAILURE);
  }

  const hasPlatforms = Array.isArray(profile.platforms) && profile.platforms.length > 0;

  if (!hasPlatforms) {
    console.error(`Profile "${profileName}" has no platforms defined.`);
    process.exit(EXIT_FAILURE);
  }

  runViteBuild(config);

  const distDir = config.build.distDir;
  validateDistDir(distDir);

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
      const outputDir = path.join(
        baseOutputDir,
        `${target.platform}${PLATFORM_DIR_SEPARATOR}${target.arch}`
      );

      // Skip full nwbuild if runtime already exists in output
      if (hasNwRuntime(outputDir, target.platform)) {
        console.log(`NW.js runtime found in ${outputDir}, copying app files only...`);
        copyAppFiles(distDir, outputDir, target.platform);
        continue;
      }

      const targetLabel = `${target.platform}-${target.arch}`;
      console.log(`Building ${targetLabel}...`);

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

      console.log(`Done: ${targetLabel} -> ${outputDir}`);
    }
  } catch (error) {
    console.error("Build failed:", error.message);
    process.exit(EXIT_FAILURE);
  }
}

async function main() {
  if (hasArgument(HELP_ARG_NAME)) {
    printHelp("sveltium-build");
    process.exit(EXIT_SUCCESS);
  }

  if (hasArgument(VERSION_ARG_NAME)) {
    console.log(readPackageVersion());
    process.exit(EXIT_SUCCESS);
  }

  const config = await loadConfig();

  if (hasArgument(LIST_ARG_NAME)) {
    listProfiles(config);
    process.exit(EXIT_SUCCESS);
  }

  const profileName = getProfileFromArgs(config);
  await buildProfile(profileName, config);
  process.exit(EXIT_SUCCESS);
}

main().catch((error) => {
  console.error("Build failed:", error.message);
  process.exit(EXIT_FAILURE);
});
