import fs from "node:fs";
import path from "node:path";
import { spawnSync } from "node:child_process";

export const EXIT_SUCCESS = 0;
export const EXIT_FAILURE = 1;
export const DEFAULT_PROFILE_NAME = "dev";
export const JSON_INDENT_SPACES = 2;

const PROCESS_ARGV_START_INDEX = 2;
const args = process.argv.slice(PROCESS_ARGV_START_INDEX);

export function hasArgument(name) {
  const hasValue = args.includes(`--${name}`);

  return hasValue;
}

export function runCommand(command, commandArgs, cwd, env = {}) {
  const result = spawnSync(command, commandArgs, {
    stdio: "inherit",
    shell: true,
    cwd,
    env: { ...process.env, ...env },
  });

  if (result.status !== EXIT_SUCCESS) {
    const exitCode = result.status || EXIT_FAILURE;
    process.exit(exitCode);
  }
}

export function writeAppPackageJson(outputDir, appConfig) {
  const packageFile = {
    name: appConfig.name,
    version: appConfig.version,
    main: appConfig.main,
    window: appConfig.window,
  };

  fs.writeFileSync(
    path.join(outputDir, "package.json"),
    JSON.stringify(packageFile, null, JSON_INDENT_SPACES)
  );
}

export function getProfileFromArgs(config) {
  const profileNames = Object.keys(config.profiles || {});

  for (const name of profileNames) {
    if (hasArgument(name)) {
      return name;
    }
  }

  return DEFAULT_PROFILE_NAME;
}

export function listProfiles(config) {
  const names = Object.keys(config.profiles || {});
  console.log(names.join("\n"));
}

export function resolveLegacy(config) {
  const legacyFromCli = hasArgument("legacy");
  const legacyEnabled = legacyFromCli || config.enableLegacy;

  return legacyEnabled;
}
