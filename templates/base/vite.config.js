import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'

const useLegacy = process.env.LEGACY === 'true'

// Dynamic import - only loads @vitejs/plugin-legacy when LEGACY=true
const legacyPlugin = useLegacy
  ? (await import('@vitejs/plugin-legacy')).default({
      targets: ['chrome 40'],
      // Skip modern polyfills - we handle them manually in src/polyfills
      modernPolyfills: false,
      // Use faster renderModernChunks when legacy chunks are generated
      renderModernChunks: true,
    })
  : null

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    svelte(),
    legacyPlugin,
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
