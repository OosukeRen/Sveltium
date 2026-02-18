#!/usr/bin/env node
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

async function runProfile(profileName, config) {
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

  runViteBuild(config);

  const distDir = config.build.distDir;
  validateDistDir(distDir);

  writeAppPackageJson(distDir, config.app);

  try {
    console.log(`Running with NW.js ${profile.version} (${profile.flavor})...`);

    await nwbuild({
      mode: "run",
      srcDir: distDir,
      glob: false,
      version: profile.version,
      flavor: profile.flavor,
      cacheDir: config.build.cacheDir,
    });
  } catch (error) {
    console.error("Run failed:", error.message);
    process.exit(EXIT_FAILURE);
  }
}

async function main() {
  if (hasArgument(HELP_ARG_NAME)) {
    printHelp("sveltium-run");
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
  await runProfile(profileName, config);
  process.exit(EXIT_SUCCESS);
}

main().catch((error) => {
  console.error("Run failed:", error.message);
  process.exit(EXIT_FAILURE);
});
