import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'
import legacy from '@vitejs/plugin-legacy'

const useLegacy = process.env.LEGACY === 'true';

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    svelte({
      compilerOptions: {
        dev: false
      }
    }),
    useLegacy && legacy({
      targets: ['chrome 40'],
      // Skip modern polyfills - we handle them manually in src/polyfills
      modernPolyfills: false,
      // Use faster renderModernChunks when legacy chunks are generated
      renderModernChunks: true,
    }),
  ].filter(Boolean),
  build: {
    // Faster minification with esbuild (default, but explicit)
    minify: 'esbuild',
    // Skip sourcemaps in production
    sourcemap: false,
    // Inline small assets to reduce requests
    assetsInlineLimit: 4096,
    // Single CSS file for small apps
    cssCodeSplit: false,
    // Reduce chunk size warnings threshold
    chunkSizeWarningLimit: 1000,
  },
  // Optimize dependency pre-bundling
  optimizeDeps: {
    // Force include svelte internals
    include: ['svelte'],
  },
})
