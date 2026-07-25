/// <reference types="vitest/config" />
import { defineConfig, type ProxyOptions } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';
import basicSsl from '@vitejs/plugin-basic-ssl';

// basicSsl generates a self-signed cert and enables https for both the dev
// server and `vite preview`. HTTPS is required so the browser exposes
// getUserMedia (microphone) in a secure context on the LAN, not just on
// http://localhost.
//
// cudaSDR's TCI server is plain WS (QWebSocketServer::NonSecureMode) on
// port 50001. An HTTPS page cannot use wss:// to that server (no TLS), and
// may block ws:// as mixed content when not on loopback. Proxy /tci through
// Vite so the browser speaks wss:// to Vite and Vite forwards to plain WS.
const tciWsProxy: Record<string, ProxyOptions> = {
  '/tci': {
    target: 'ws://127.0.0.1:50001',
    ws: true,
    changeOrigin: true,
    // QWebSocketServer accepts any path; strip the /tci prefix anyway.
    rewrite: (path) => path.replace(/^\/tci/, '') || '/',
  },
};

export default defineConfig({
  plugins: [svelte(), basicSsl()],
  server: {
    // Listen on all interfaces (required for phone/tablet on LAN).
    host: '0.0.0.0',
    port: 5173,
    strictPort: true,
    open: 'https://localhost:5173',
    // HMR websocket must use the same host the phone used (not localhost).
    hmr: {
      clientPort: 5173,
    },
    proxy: tciWsProxy,
  },
  preview: {
    host: '0.0.0.0',
    port: 5173,
    strictPort: true,
    proxy: tciWsProxy,
  },
  test: {
    environment: 'node',
    include: ['src/**/*.test.ts'],
  },
});
