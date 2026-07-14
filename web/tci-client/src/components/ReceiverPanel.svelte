<script lang="ts">
  import { onDestroy, onMount } from 'svelte';
  import { ReceiverDisplay } from '../display/ReceiverDisplay';
  import type { RadioState } from '../protocol/types';

  export let state: RadioState;
  export let onTune: (hz: number) => void;
  export let onDragTune: (deltaHz: number) => void;
  export let onReady: ((api: { pushFft: (m: Float32Array) => void }) => void) | undefined =
    undefined;

  let root: HTMLDivElement;
  let display: ReceiverDisplay | null = null;

  /** Called directly from TciClient on each FFT — bypasses Svelte reactivity. */
  export function pushFft(magnitudes: Float32Array): void {
    display?.setFft(magnitudes);
  }

  onMount(() => {
    display = new ReceiverDisplay(root);
    display.onTune = (hz) => onTune(hz);
    display.onDragTune = (deltaHz) => onDragTune(deltaHz);
    onReady?.({ pushFft: (m) => display?.setFft(m) });
  });

  onDestroy(() => {
    display?.destroy();
    display = null;
  });

  $: if (display) display.setState(state);
</script>

<div class="receiver-root" bind:this={root}></div>
