<script lang="ts">
  import type { RadioState } from '../protocol/types';
  import { buildFreqDisplayParts, formatSmeter, wheelFrequencySteps } from '../lib/format';

  export let state: RadioState;
  export let wsUrl: string;
  export let onStepFrequency: ((deltaHz: number) => void) | undefined = undefined;
  export let onStepVfoB: ((deltaHz: number) => void) | undefined = undefined;
  export let onSetVfo: ((hz: number) => void) | undefined = undefined;
  export let onSetVfoB: ((hz: number) => void) | undefined = undefined;
  export let onSelectVfo: ((which: 'a' | 'b') => void) | undefined = undefined;

  /** The radio's active VFO, echoed back by cudaSDR — step buttons and typing apply here. */
  $: activeVfo = state.activeVfo;
  let editOpen = false;
  let editText = '';

  $: freqParts = buildFreqDisplayParts(state.vfoHz);
  $: freqBParts = buildFreqDisplayParts(state.vfoBHz);
  $: txHz = state.splitEnabled ? state.vfoBHz : state.vfoHz;
  $: activeHz = activeVfo === 'a' ? state.vfoHz : state.vfoBHz;

  function selectVfo(which: 'a' | 'b'): void {
    editOpen = false;
    if (which !== state.activeVfo) onSelectVfo?.(which);
  }

  function stepActive(deltaHz: number): void {
    if (!state.connected) return;
    if (activeVfo === 'a') onStepFrequency?.(deltaHz);
    else onStepVfoB?.(deltaHz);
  }

  function onDigitWheel(e: WheelEvent, stepHz: number, which: 'a' | 'b'): void {
    selectVfo(which);
    const handler = which === 'a' ? onStepFrequency : onStepVfoB;
    if (!handler || stepHz <= 0) return;
    e.preventDefault();
    const notches = wheelFrequencySteps(e.deltaY);
    if (notches !== 0) handler(notches * stepHz);
  }

  function openEdit(): void {
    if (!state.connected) return;
    editText = (activeHz / 1_000_000).toFixed(6);
    editOpen = true;
  }

  function commitEdit(): void {
    editOpen = false;
    const mhz = Number(editText);
    if (!Number.isFinite(mhz) || mhz <= 0) return;
    const hz = Math.round(mhz * 1_000_000);
    if (activeVfo === 'a') onSetVfo?.(hz);
    else onSetVfoB?.(hz);
  }

  function onEditKey(e: KeyboardEvent): void {
    if (e.key === 'Enter') {
      e.preventDefault();
      commitEdit();
    } else if (e.key === 'Escape') {
      editOpen = false;
    }
  }
</script>

<header class="top-bar">
  <div class="vfo-stack">
    <div class="vfo-row">
      <button
        type="button"
        class="vfo-select"
        class:selected={activeVfo === 'a'}
        disabled={!state.connected}
        onclick={() => selectVfo('a')}
        title="Put the radio's dial on VFO-A. Step buttons and typing apply here."
      >A</button>
      <div
        class="vfo-panel"
        class:selected={activeVfo === 'a'}
        role="button"
        tabindex="0"
        onclick={() => selectVfo('a')}
        onkeydown={(e) => e.key === 'Enter' && selectVfo('a')}
      >
        <div class="vfo-label mono">VFO A · click to make active</div>
        <div class="freq-display mono" title="Selected: use − / + or scroll a digit">
          {#each freqParts as part}
            {#if part.stepHz > 0}
              <span
                class="freq-digit"
                onwheel={(e) => { e.preventDefault(); onDigitWheel(e, part.stepHz, 'a'); }}
              >{part.text}</span>
            {:else}
              <span class="freq-sep">{part.text}</span>
            {/if}
          {/each}
        </div>
      </div>
    </div>

    <div class="vfo-row">
      <button
        type="button"
        class="vfo-select"
        class:selected={activeVfo === 'b'}
        class:split-active={state.splitEnabled}
        disabled={!state.connected}
        onclick={() => selectVfo('b')}
        title="Put the radio's dial on VFO-B. Enable Split so MOX uses this frequency."
      >B</button>
      <div
        class="vfo-panel vfo-b"
        class:selected={activeVfo === 'b'}
        class:split-active={state.splitEnabled}
        role="button"
        tabindex="0"
        onclick={() => selectVfo('b')}
        onkeydown={(e) => e.key === 'Enter' && selectVfo('b')}
      >
        <div class="vfo-label mono">
          VFO B · click to make active{#if state.splitEnabled}<span class="split-tag"> · SPLIT ON</span>{/if}
        </div>
        <div class="freq-display freq-b mono" title="Click B, then − / + or scroll digits">
          {#each freqBParts as part}
            {#if part.stepHz > 0}
              <span
                class="freq-digit"
                onwheel={(e) => { e.preventDefault(); onDigitWheel(e, part.stepHz, 'b'); }}
              >{part.text}</span>
            {:else}
              <span class="freq-sep">{part.text}</span>
            {/if}
          {/each}
        </div>
      </div>
    </div>

    <div class="vfo-tune-bar">
      <span class="mono tune-hint">
        Tuning {activeVfo === 'a' ? 'VFO-A' : 'VFO-B'}
      </span>
      <button type="button" disabled={!state.connected} onclick={() => stepActive(-1000)} title="-1 kHz">−1k</button>
      <button type="button" disabled={!state.connected} onclick={() => stepActive(-100)} title="-100 Hz">−100</button>
      <button type="button" disabled={!state.connected} onclick={() => stepActive(100)} title="+100 Hz">+100</button>
      <button type="button" disabled={!state.connected} onclick={() => stepActive(1000)} title="+1 kHz">+1k</button>
      {#if editOpen}
        <input
          class="freq-edit mono"
          bind:value={editText}
          onkeydown={onEditKey}
          onblur={commitEdit}
          autofocus
          title="MHz — Enter to set"
        />
      {:else}
        <button type="button" disabled={!state.connected} onclick={openEdit} title="Type frequency in MHz">
          Set MHz…
        </button>
      {/if}
    </div>

    <div class="freq-meta mono">
      TX@{((txHz) / 1_000_000).toFixed(3)} MHz
      {#if !state.splitEnabled}
        <span class="hint"> · turn Split on to TX on VFO-B</span>
      {/if}
      · {state.device || 'cudaSDR'}
    </div>
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
    title="Default: Vite /tci proxy → cudaSDR plain WS :50001"
    placeholder="wss://host:5173/tci"
  />
</header>
