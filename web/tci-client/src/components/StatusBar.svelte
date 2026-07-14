<script lang="ts">
  import { onMount } from 'svelte';

  export let connected: boolean;
  export let wsUrl: string;
  export let logMessage: string;
  export let logLevel: 'sys' | 'err';

  onMount(() => {
    const el = document.getElementById('utc-clock');
    const tick = () => {
      if (!el) return;
      const now = new Date();
      const h = now.getUTCHours().toString().padStart(2, '0');
      const m = now.getUTCMinutes().toString().padStart(2, '0');
      const s = now.getUTCSeconds().toString().padStart(2, '0');
      el.textContent = `${h}:${m}:${s} UTC`;
    };
    tick();
    const id = setInterval(tick, 1000);
    return () => clearInterval(id);
  });
</script>

<footer class="status-bar mono">
  <span>{connected ? 'Connected' : 'Disconnected'}</span>
  <span>{wsUrl}</span>
  <span class="log-line" class:err={logLevel === 'err'}>{logMessage}</span>
  <span class="spacer"></span>
  <span id="utc-clock"></span>
</footer>
