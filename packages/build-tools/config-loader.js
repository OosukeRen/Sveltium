import fs from "node:fs";
import path from "node:path";
import { pathToFileURL } from "node:url";

const CONFIG_FILE_NAME = "sveltium.config.js";
const PACKAGE_JSON_NAME = "package.json";

const DEFAULT_APP_NAME = "nwjs_app";
const DEFAULT_APP_VERSION = "1.0.0";
const DEFAULT_MAIN_ENTRY = "index.html";
const DEFAULT_WINDOW_WIDTH = 1024;
const DEFAULT_WINDOW_HEIGHT = 768;

function readProjectPackageJson(projectDir) {
  const packageJsonPath = path.join(projectDir, PACKAGE_JSON_NAME);
  const rawContent = fs.readFileSync(packageJsonPath, "utf-8");
  const packageJson = JSON.parse(rawContent);

  return packageJson;
}

async function loadUserConfig(projectDir) {
  const configPath = path.join(projectDir, CONFIG_FILE_NAME);
  const configExists = fs.existsSync(configPath);

  if (!configExists) {
    return {};
  }

  // Dynamic import using file:// URL for Windows compatibility
  const configUrl = pathToFileURL(configPath).href;
  const configModule = await import(configUrl);

  return configModule.default || {};
}

function buildDefaultAppConfig(packageJson) {
  const appName = packageJson.name || DEFAULT_APP_NAME;
  const appVersion = packageJson.version || DEFAULT_APP_VERSION;

  return {
    name: appName,
    version: appVersion,
    main: DEFAULT_MAIN_ENTRY,
    window: {
      title: appName,
      width: DEFAULT_WINDOW_WIDTH,
      height: DEFAULT_WINDOW_HEIGHT,
    },
  };
}

function buildDefaultBuildConfig() {
  return {
    distDir: path.resolve("dist"),
    cacheDir: path.resolve("nw_cache"),
    outputDir: path.resolve("builds"),
  };
}

export async function loadConfig() {
  const projectDir = process.cwd();
  const packageJson = readProjectPackageJson(projectDir);
  const userConfig = await loadUserConfig(projectDir);

  const defaultApp = buildDefaultAppConfig(packageJson);
  const defaultBuild = buildDefaultBuildConfig();

  const app = { ...defaultApp, ...userConfig.app };
  const build = { ...defaultBuild, ...userConfig.build };
  const profiles = userConfig.profiles || {};
  const enableLegacy = userConfig.enableLegacy || false;

  return { app, build, profiles, enableLegacy };
}
