<script lang="ts">
  import type { RadioState } from '../protocol/types';
  import { MODULATIONS, SPEC_AVG_MODES } from '../protocol/types';

  export let state: RadioState;
  export let onConnect: () => void;
  export let onDisconnect: () => void;
  export let onMox: () => void;
  export let onTune: () => void;
  export let onModeChange: (mode: string) => void;
  export let onDriveChange: (level: number) => void;
  export let onAvgChange: (count: number) => void;
  export let onAvgModeChange: (mode: number) => void;
  export let onAvgDomainChange: (domain: 'log' | 'linear') => void;
  export let onRxAudioToggle: () => void;
  export let onVolumeChange: (level: number) => void;
  export let onMuteToggle: () => void;
  export let onSplitToggle: () => void;
  export let onCopyVfoAToB: () => void;
</script>

<div class="control-bar">
  {#if state.connected}
    <button type="button" onclick={onDisconnect}>Disconnect</button>
  {:else}
    <button type="button" onclick={onConnect}>Connect</button>
  {/if}

  <select
    value={state.modulation}
    disabled={!state.connected}
    onchange={(e) => onModeChange((e.currentTarget as HTMLSelectElement).value)}
  >
    {#each MODULATIONS as mode}
      <option value={mode}>{mode}</option>
    {/each}
  </select>

  <button
    type="button"
    class:on={state.rxAudioOn}
    disabled={!state.connected}
    onclick={onRxAudioToggle}
  >
    RX Audio
  </button>

  <button
    type="button"
    class:on={state.splitEnabled}
    disabled={!state.connected}
    onclick={onSplitToggle}
    title="split_enable — TX dial follows VFO-B when on"
  >
    Split
  </button>

  <button
    type="button"
    disabled={!state.connected}
    onclick={onCopyVfoAToB}
    title="Copy VFO-A → VFO-B"
  >
    A→B
  </button>

  <button
    type="button"
    class:tx-on={state.trx}
    disabled={!state.connected}
    onclick={onMox}
  >
    MOX
  </button>

  <button
    type="button"
    class:on={state.tune}
    disabled={!state.connected}
    onclick={onTune}
  >
    Tune
  </button>

  <div class="slider-group">
    <label for="drive">Drive:</label>
    <input
      id="drive"
      type="range"
      min="0"
      max="100"
      value={state.drive}
      disabled={!state.connected}
      oninput={(e) => onDriveChange(Number((e.currentTarget as HTMLInputElement).value))}
    />
    <span class="mono">{state.drive}</span>
  </div>

  <div class="slider-group">
    <label for="vol">Vol:</label>
    <input
      id="vol"
      type="range"
      min="0"
      max="100"
      value={state.volume}
      disabled={!state.connected || !state.rxAudioOn}
      oninput={(e) => onVolumeChange(Number((e.currentTarget as HTMLInputElement).value))}
    />
    <span class="mono">{state.muted ? 'Mute' : `${state.volume}%`}</span>
  </div>

  <select
    value={state.specAvgMode}
    onchange={(e) => onAvgModeChange(Number((e.currentTarget as HTMLSelectElement).value))}
  >
    {#each SPEC_AVG_MODES as label, i}
      <option value={i}>{label}</option>
    {/each}
  </select>

  <select
    value={state.specAvgDomain}
    onchange={(e) =>
      onAvgDomainChange((e.currentTarget as HTMLSelectElement).value as 'log' | 'linear')}
  >
    <option value="log">Log</option>
    <option value="linear">Linear</option>
  </select>

  <div class="slider-group">
    <label for="avg">Avg:</label>
    <input
      id="avg"
      type="range"
      min="1"
      max="20"
      value={state.specAvg}
      disabled={state.specAvgMode === 0}
      oninput={(e) => onAvgChange(Number((e.currentTarget as HTMLInputElement).value))}
    />
    <span class="mono">{state.specAvg}</span>
  </div>

  <button
    type="button"
    class:on={state.muted}
    disabled={!state.connected || !state.rxAudioOn}
    onclick={onMuteToggle}
  >
    Mute
  </button>
</div>
