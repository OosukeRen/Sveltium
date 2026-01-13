import path from "node:path";
import { createRequire } from "node:module";

const require = createRequire(import.meta.url);
const rootPackage = require("../../package.json");

const DEFAULT_APP_NAME = "nwjs_app";
const DEFAULT_APP_VERSION = "1.0.0";
const DEFAULT_MAIN_ENTRY = "index.html";
const DEFAULT_WINDOW_WIDTH = 1024;
const DEFAULT_WINDOW_HEIGHT = 768;

const DEFAULT_APP_TITLE = rootPackage.name || DEFAULT_APP_NAME;

export default {
  app: {
    name: rootPackage.name || DEFAULT_APP_NAME,
    version: rootPackage.version || DEFAULT_APP_VERSION,
    main: DEFAULT_MAIN_ENTRY,
    window: {
      title: DEFAULT_APP_TITLE,
      width: DEFAULT_WINDOW_WIDTH,
      height: DEFAULT_WINDOW_HEIGHT,
    },
  },
  build: {
    distDir: path.resolve("dist"),
    stagingDir: path.resolve(".nwjs_build", "app"),
    cacheDir: path.resolve("nw_cache"),
    outputDir: path.resolve("builds"),
  },
  profiles: {
    dev: {
      version: "0.93.0",
      platforms: ["win64"],
      flavor: "sdk",
      outputDir: path.resolve("builds", "dev"),
    },
    xp: {
      version: "0.15.4",
      platforms: ["win32"],
      flavor: "sdk",
      outputDir: path.resolve("builds", "xp"),
    },
    release: {
      version: "0.93.0",
      platforms: ["win32", "win64"],
      flavor: "sdk",
      outputDir: path.resolve("builds", "release"),
    },
  },
};
