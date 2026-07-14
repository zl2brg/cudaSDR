<script lang="ts">
  import { onMount } from 'svelte';
  import TopBar from './components/TopBar.svelte';
  import ControlBar from './components/ControlBar.svelte';
  import ReceiverPanel from './components/ReceiverPanel.svelte';
  import StatusBar from './components/StatusBar.svelte';
  import { TciClient } from './protocol/TciClient';
  import { defaultRadioState, type RadioState } from './protocol/types';

  let wsUrl =
    typeof window !== 'undefined'
      ? `${window.location.protocol === 'https:' ? 'wss' : 'ws'}://${window.location.hostname}:50001`
      : 'ws://127.0.0.1:50001';
  let state: RadioState = defaultRadioState();
  let logMessage = 'Ready';
  let logLevel: 'sys' | 'err' = 'sys';

  let client: TciClient | undefined;
  let pushFft: ((m: Float32Array) => void) | null = null;

  onMount(() => {
    client = new TciClient({
      onState: (s) => {
        state = s;
      },
      onFft: (m) => {
        pushFft?.(m);
      },
      onLog: (level, message) => {
        logLevel = level;
        logMessage = message;
      },
    });
    return () => client?.disconnect();
  });

  function onReceiverReady(api: { pushFft: (m: Float32Array) => void }): void {
    pushFft = api.pushFft;
  }

  async function connect() {
    await client?.connect(wsUrl.trim());
  }

  function disconnect() {
    client?.disconnect();
  }

  function onTune(hz: number) {
    client?.setVfo(hz);
  }

  function onDragTune(deltaHz: number) {
    client?.shiftFrequency(deltaHz);
  }

  function onStepFrequency(deltaHz: number) {
    if (!state.connected) return;
    client?.stepMainFrequency(deltaHz);
  }

  function onMox() {
    client?.setTrx(!state.trx);
  }

  function onTuneBtn() {
    client?.setTune(!state.tune);
  }

  function onModeChange(mode: string) {
    client?.setModulation(mode);
  }

  function onDriveChange(level: number) {
    client?.setDrive(level);
  }

  function onAvgChange(count: number) {
    client?.setSpectrumAveraging(count);
  }

  function onAvgModeChange(mode: number) {
    client?.setSpectrumAvgMode(mode);
  }

  function onAvgDomainChange(domain: 'log' | 'linear') {
    client?.setSpectrumAvgDomain(domain);
  }

  function onRxAudioToggle() {
    void client?.toggleRxAudio();
  }

  function onVolumeChange(level: number) {
    client?.setVolume(level);
  }

  function onMuteToggle() {
    client?.setMuted(!state.muted);
  }
</script>

<div class="app">
  <TopBar {state} bind:wsUrl {onStepFrequency} />
  <ControlBar
    {state}
    onConnect={connect}
    onDisconnect={disconnect}
    onMox={onMox}
    onTune={onTuneBtn}
    {onModeChange}
    {onDriveChange}
    {onAvgChange}
    {onAvgModeChange}
    {onAvgDomainChange}
    {onRxAudioToggle}
    {onVolumeChange}
    {onMuteToggle}
  />
  <ReceiverPanel {state} {onTune} {onDragTune} onReady={onReceiverReady} />
  <StatusBar connected={state.connected} {wsUrl} {logMessage} {logLevel} />
</div>
