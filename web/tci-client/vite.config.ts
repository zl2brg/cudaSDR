/// <reference types="vitest/config" />
import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';
import basicSsl from '@vitejs/plugin-basic-ssl';

// basicSsl generates a self-signed cert and enables https for both the dev
// server and `vite preview`. HTTPS is required so the browser exposes
// getUserMedia (microphone) in a secure context on the LAN, not just on
// http://localhost. The client picks wss:// automatically when served over https.
export default defineConfig({
  plugins: [svelte(), basicSsl()],
  server: {
    // Listen on all interfaces (required for phone/tablet on LAN).
    host: '0.0.0.0',
    port: 5173,
    strictPort: true,
    open: 'http://localhost:5173',
    // HMR websocket must use the same host the phone used (not localhost).
    hmr: {
      clientPort: 5173,
    },
  },
  preview: {
    host: '0.0.0.0',
    port: 5173,
    strictPort: true,
  },
  test: {
    environment: 'node',
    include: ['src/**/*.test.ts'],
  },
});
