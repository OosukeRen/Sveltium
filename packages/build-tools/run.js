#!/usr/bin/env node
import fs from "node:fs";
import nwbuild from "nw-builder";
import { loadConfig } from "./config-loader.js";
import {
  EXIT_FAILURE,
  hasArgument,
  runCommand,
  writeAppPackageJson,
  getProfileFromArgs,
  listProfiles,
  resolveLegacy,
} from "./shared.js";

const NO_VITE_ARG_NAME = "no-vite";

async function runProfile(profileName, config) {
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

  try {
    await nwbuild({
      mode: "run",
      srcDir: distDir,
      glob: false,
      version: profile.version,
      flavor: profile.flavor,
      cacheDir: config.build.cacheDir,
    });
  } catch (error) {
    console.error(error);
    process.exit(EXIT_FAILURE);
  }
}

async function main() {
  const config = await loadConfig();
  const profileName = getProfileFromArgs(config);
  await runProfile(profileName, config);
}

main();
