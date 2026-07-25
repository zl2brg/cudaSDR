<script lang="ts">
  import type { RadioState } from '../protocol/types';
  import { buildFreqDisplayParts, formatSmeter, wheelFrequencySteps } from '../lib/format';

  export let state: RadioState;
  export let wsUrl: string;
  export let onStepFrequency: ((deltaHz: number) => void) | undefined = undefined;

  $: freqParts = buildFreqDisplayParts(state.vfoHz);

  function onDigitWheel(e: WheelEvent, stepHz: number): void {
    if (!onStepFrequency || stepHz <= 0) return;
    e.preventDefault();
    const notches = wheelFrequencySteps(e.deltaY);
    if (notches !== 0) onStepFrequency(notches * stepHz);
  }
</script>

<header class="top-bar">
  <div>
    <div class="freq-display mono" title="Scroll wheel on a digit to tune">
      {#each freqParts as part}
        {#if part.stepHz > 0}
          <span
            class="freq-digit"
            on:wheel|preventDefault={(e) => onDigitWheel(e, part.stepHz)}
          >{part.text}</span>
        {:else}
          <span class="freq-sep">{part.text}</span>
        {/if}
      {/each}
    </div>
    <div class="freq-meta mono">Rx: 0 · wheel a digit to tune · {state.device || 'cudaSDR'}</div>
  </div>
  <div class="spacer"></div>
  <div
    class="status-dot"
    class:on={state.connected && !state.trx}
    class:tx={state.trx}
    title={state.connected ? (state.trx ? 'TX' : 'RX') : 'Disconnected'}
  ></div>
  <div class="smeter-wrap">
    <div class="smeter-bar">
      <div
        class="smeter-fill"
        style:width="{Math.max(0, Math.min(100, ((state.smeterDbm + 130) / 100) * 100))}%"
      ></div>
    </div>
    <div class="smeter-label mono">{formatSmeter(state.smeterDbm)}</div>
  </div>
  {#if state.trx || state.tune}
    <div class="tx-meters mono" title="Forward power / SWR">
      <span>{state.txPowerWatts.toFixed(1)} W</span>
      <span
        class:swr-ok={state.swr < 1.5}
        class:swr-warn={state.swr >= 1.5 && state.swr < 2.5}
        class:swr-bad={state.swr >= 2.5}
      >SWR {state.swr.toFixed(2)}</span>
    </div>
  {/if}
  <input
    class="conn-input mono"
    bind:value={wsUrl}
    disabled={state.connected}
    spellcheck="false"
    title="Default: Vite /tci proxy → cudaSDR plain WS :50001. Or ws://127.0.0.1:50001"
    placeholder="wss://host:5173/tci"
  />
</header>
