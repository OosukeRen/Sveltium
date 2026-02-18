import fs from "node:fs";
import path from "node:path";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";

export const EXIT_SUCCESS = 0;
export const EXIT_FAILURE = 1;
export const DEFAULT_PROFILE_NAME = "dev";
export const JSON_INDENT_SPACES = 2;

const PROCESS_ARGV_START_INDEX = 2;
const args = process.argv.slice(PROCESS_ARGV_START_INDEX);

const __dirname = path.dirname(fileURLToPath(import.meta.url));

export function readPackageVersion() {
  const packageJsonPath = path.join(__dirname, "package.json");
  const content = JSON.parse(fs.readFileSync(packageJsonPath, "utf-8"));

  return content.version;
}

export function printHelp(commandName) {
  const lines = [
    `Usage: ${commandName} [--profile-name] [options]`,
    "",
    "Options:",
    "  --<profile>   Select a build profile (default: dev)",
    "  --list        List available profiles",
    "  --no-vite     Skip the Vite build step",
    "  --legacy      Enable legacy browser support",
    "  --help        Show this help message",
    "  --version     Show version number",
  ];

  console.log(lines.join("\n"));
}

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

  if (result.error) {
    console.error(`Failed to run "${command}": ${result.error.message}`);
  }

  if (result.signal) {
    console.error(`"${command}" was terminated by signal: ${result.signal}`);
  }

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

const RESERVED_FLAGS = ["list", "no-vite", "legacy", "help", "version"];

export function getProfileFromArgs(config) {
  const profileNames = Object.keys(config.profiles || {});

  for (const name of profileNames) {
    const isReserved = RESERVED_FLAGS.includes(name);

    if (isReserved) {
      console.warn(`Warning: profile name "${name}" conflicts with reserved flag --${name}`);
      continue;
    }

    if (hasArgument(name)) {
      return name;
    }
  }

  console.log(`No profile specified, using default: ${DEFAULT_PROFILE_NAME}`);
  return DEFAULT_PROFILE_NAME;
}

export function detectPackageManager(projectDir) {
  const lockfileToManager = {
    "pnpm-lock.yaml": "pnpm",
    "yarn.lock": "yarn",
    "bun.lockb": "bun",
    "bun.lock": "bun",
    "package-lock.json": "npm",
  };

  for (const [lockfile, manager] of Object.entries(lockfileToManager)) {
    const lockfilePath = path.join(projectDir, lockfile);

    if (fs.existsSync(lockfilePath)) {
      return manager;
    }
  }

  return "npm";
}

export function listProfiles(config) {
  const names = Object.keys(config.profiles || {});
  const hasProfiles = names.length > 0;

  if (!hasProfiles) {
    console.log("No profiles defined in sveltium.config.js");
    return;
  }

  for (const name of names) {
    const isReserved = RESERVED_FLAGS.includes(name);

    if (isReserved) {
      console.warn(`Warning: profile name "${name}" conflicts with reserved flag --${name}`);
    }
  }

  console.log("Available profiles:");
  console.log(names.map((name) => `  ${name}`).join("\n"));
}

export function resolveLegacy(config) {
  const legacyFromCli = hasArgument("legacy");
  const legacyEnabled = legacyFromCli || config.enableLegacy;

  return legacyEnabled;
}
