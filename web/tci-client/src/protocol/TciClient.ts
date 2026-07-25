import { RxAudioPlayer } from '../audio/RxAudioPlayer';
import { TxAudioCapture } from '../audio/TxAudioCapture';
import { defaultRadioState, type RadioState } from './types';
import {
  buildTxAudioFrame,
  formatTciCommand,
  parseStreamFrame,
  parseTciMessages,
  STREAM_HEADER_BYTES,
  STREAM_IQ,
  STREAM_RX_AUDIO,
} from './StreamParser';

export type TciClientHandlers = {
  onState?: (state: RadioState) => void;
  onFft?: (magnitudes: Float32Array, sampleRate: number) => void;
  onLog?: (level: 'sys' | 'err', message: string) => void;
};

export class TciClient {
  private ws: WebSocket | null = null;
  private state: RadioState = defaultRadioState();
  private handlers: TciClientHandlers;
  private iqWorker: Worker | null = null;
  private audioPlayer = new RxAudioPlayer();
  private txCapture: TxAudioCapture | null = null;
  private userClosed = false;
  private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  private reconnectDelay = 1000;
  private readonly reconnectMax = 30_000;
  private lastUrl = '';

  constructor(handlers: TciClientHandlers = {}) {
    this.handlers = handlers;
  }

  getState(): RadioState {
    return { ...this.state };
  }

  async connect(url: string): Promise<void> {
    this.disconnect(false);
    this.userClosed = false;
    this.lastUrl = url;
    this.log('sys', `Connecting → ${url}`);

    // AudioContext must be created during the Connect click (user gesture).
    try {
      await this.audioPlayer.start();
      this.audioPlayer.setVolume(this.state.volume);
      this.audioPlayer.setMuted(this.state.muted);
      this.patchState({ rxAudioOn: true });
    } catch (err) {
      this.log('err', `RX audio unlock failed: ${(err as Error).message}`);
      this.patchState({ rxAudioOn: false });
    }

    try {
      const ws = new WebSocket(url);
      ws.binaryType = 'arraybuffer';
      this.ws = ws;

      ws.onopen = () => {
        this.patchState({ connected: true });
        this.reconnectDelay = 1000;
        this.log('sys', 'Connected');
        this.startIqWorker();
        window.setTimeout(() => {
          this.subscribeIq(0);
          this.send(formatTciCommand('TX_SENSORS_ENABLE', ['true', 200]));
          if (this.state.rxAudioOn) {
            this.subscribeAudio(0);
            this.log('sys', 'RX audio stream requested');
          }
        }, 300);
      };

      ws.onmessage = (ev) => {
        if (typeof ev.data === 'string') {
          this.handleText(ev.data);
        } else if (ev.data instanceof ArrayBuffer) {
          this.handleBinary(ev.data);
        }
      };

      ws.onclose = () => {
        const wasConnected = this.state.connected;
        this.stopTxAudio();
        this.stopRxAudio();
        this.patchState({ connected: false, fftReady: false });
        this.stopIqWorker();
        if (wasConnected) this.log('sys', 'Disconnected');
        if (!this.userClosed) this.scheduleReconnect(this.lastUrl);
      };

      ws.onerror = () => {
        this.log(
          'err',
          'WebSocket error — is cudaSDR TCI on :50001? Use Vite /tci proxy or ws://127.0.0.1:50001 (not wss:// to cudaSDR)',
        );
      };
    } catch (err) {
      this.log('err', `Bad URL: ${(err as Error).message}`);
    }
  }

  disconnect(userInitiated = true): void {
    if (userInitiated) this.userClosed = true;
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }
    this.stopTxAudio();
    this.stopRxAudio();
    this.stopIqWorker();
    if (this.ws) {
      this.ws.onclose = null;
      this.ws.close();
      this.ws = null;
    }
    this.patchState({ connected: false, fftReady: false, rxAudioOn: false });
  }

  setVfo(hz: number, rx = 0): void {
    this.resetSpectrumAvg();
    this.send(formatTciCommand('VFO', [0, rx, Math.round(hz)]));
  }

  setDds(hz: number, rx = 0): void {
    this.resetSpectrumAvg();
    this.send(formatTciCommand('DDS', [0, rx, Math.round(hz)]));
  }

  /** Wheel on main frequency readout: step VFO and DDS to the same frequency. */
  stepMainFrequency(deltaHz: number, rx = 0): void {
    if (!Number.isFinite(deltaHz) || deltaHz === 0) return;
    const newFreq = Math.max(0, Math.round(this.state.vfoHz + deltaHz));
    this.resetSpectrumAvg();
    this.send(formatTciCommand('VFO', [0, rx, newFreq]));
    this.send(formatTciCommand('DDS', [0, rx, newFreq]));
    this.patchState({ vfoHz: newFreq, ddsHz: newFreq });
  }

  /** Drag panadapter: move VFO and DDS together (cudaSDR unpinned pan). */
  shiftFrequency(deltaHz: number, rx = 0): void {
    if (!Number.isFinite(deltaHz) || deltaHz === 0) return;
    this.resetSpectrumAvg();
    const vfo = Math.round(this.state.vfoHz + deltaHz);
    const dds = Math.round(this.state.ddsHz + deltaHz);
    this.send(formatTciCommand('VFO', [0, rx, vfo]));
    this.send(formatTciCommand('DDS', [0, rx, dds]));
    this.patchState({ vfoHz: vfo, ddsHz: dds });
  }

  setModulation(mode: string, rx = 0): void {
    this.send(formatTciCommand('MODULATION', [rx, mode]));
  }

  setFilter(lo: number, hi: number, rx = 0): void {
    this.send(formatTciCommand('RX_FILTER_BAND', [rx, Math.round(lo), Math.round(hi)]));
  }

  setTrx(enabled: boolean): void {
    this.send(formatTciCommand('TRX', [0, enabled ? 'true' : 'false']));
    if (enabled) {
      void this.startTxAudio();
    } else {
      this.stopTxAudio();
    }
  }

  private async startTxAudio(): Promise<void> {
    try {
      if (!this.txCapture) {
        this.txCapture = new TxAudioCapture((frame, rate) => {
          if (this.ws?.readyState === WebSocket.OPEN) {
            this.ws.send(buildTxAudioFrame(frame, rate));
          }
        });
      }
      await this.txCapture.start();
      this.log('sys', 'Mic TX streaming on');
    } catch (err) {
      this.log('err', `Mic capture failed: ${(err as Error).message}`);
    }
  }

  private stopTxAudio(): void {
    if (this.txCapture?.isActive()) {
      this.txCapture.stop();
      this.log('sys', 'Mic TX streaming off');
    }
  }

  setTune(enabled: boolean): void {
    this.send(formatTciCommand('TUNE', [0, enabled ? 'true' : 'false']));
  }

  setDrive(level: number): void {
    this.send(formatTciCommand('DRIVE', [0, Math.max(0, Math.min(100, Math.round(level)))]));
  }

  setSpectrumAveraging(count: number): void {
    const n = Math.max(1, Math.min(50, Math.round(count)));
    this.patchState({ specAvg: n });
    this.iqWorker?.postMessage({ type: 'setAvg', value: n });
  }

  setSpectrumAvgMode(mode: number): void {
    const m = Math.max(0, Math.round(mode));
    this.patchState({ specAvgMode: m });
    this.iqWorker?.postMessage({ type: 'setAvgMode', value: m });
  }

  setSpectrumAvgDomain(domain: 'log' | 'linear'): void {
    const d: 'log' | 'linear' = domain === 'linear' ? 'linear' : 'log';
    this.patchState({ specAvgDomain: d });
    this.iqWorker?.postMessage({ type: 'setAvgDomain', value: d });
  }

  setVolume(percent: number): void {
    this.patchState({ volume: Math.max(0, Math.min(100, Math.round(percent))) });
    this.audioPlayer.setVolume(this.state.volume);
  }

  setMuted(muted: boolean): void {
    this.patchState({ muted });
    this.audioPlayer.setMuted(muted);
  }

  async toggleRxAudio(rx = 0): Promise<void> {
    if (this.state.rxAudioOn) {
      this.stopRxAudio();
    } else {
      await this.startRxAudio(rx);
    }
  }

  async startRxAudio(rx = 0): Promise<void> {
    try {
      if (!this.audioPlayer.isActive()) {
        await this.audioPlayer.start();
        this.audioPlayer.setVolume(this.state.volume);
        this.audioPlayer.setMuted(this.state.muted);
      }
      this.patchState({ rxAudioOn: true });
      if (this.ws?.readyState === WebSocket.OPEN) {
        this.subscribeAudio(rx);
      }
      this.log('sys', 'RX audio on');
    } catch (err) {
      this.log('err', `RX audio failed: ${(err as Error).message}`);
      this.patchState({ rxAudioOn: false });
    }
  }

  stopRxAudio(): void {
    if (this.state.rxAudioOn && this.ws?.readyState === WebSocket.OPEN) {
      this.send(formatTciCommand('AUDIO_STOP', [0]));
    }
    this.audioPlayer.stop();
    if (this.state.rxAudioOn) {
      this.patchState({ rxAudioOn: false });
      this.log('sys', 'RX audio off');
    }
  }

  private subscribeIq(rx = 0): void {
    this.send(formatTciCommand('IQ_STREAM_SAMPLE_TYPE', ['float32']));
    this.send(formatTciCommand('IQ_STREAM_CHANNELS', [2]));
    this.send(formatTciCommand('IQ_STREAM_SAMPLES', [512]));
    this.send(formatTciCommand('IQ_SAMPLERATE', [192000]));
    this.send(formatTciCommand('IQ_START', [rx]));
    this.log('sys', 'IQ panadapter stream requested');
  }

  private subscribeAudio(rx = 0): void {
    const cmds = [
      formatTciCommand('AUDIO_START', [rx]),
      formatTciCommand('AUDIO_STREAM_SAMPLES', [2048]),
      formatTciCommand('AUDIO_STREAM_CHANNELS', [2]),
      formatTciCommand('AUDIO_STREAM_SAMPLE_TYPE', ['float32']),
      formatTciCommand('AUDIO_SAMPLERATE', [48000]),
    ];
    cmds.forEach((cmd, i) => {
      window.setTimeout(() => this.send(cmd), i * 20);
    });
  }

  private send(cmd: string): void {
    if (!this.ws || this.ws.readyState !== WebSocket.OPEN) return;
    this.ws.send(cmd);
  }

  private handleText(raw: string): void {
    for (const msg of parseTciMessages(raw)) {
      switch (msg.name) {
        case 'DEVICE':
          if (msg.args[0]) this.patchState({ device: msg.args[0] });
          break;
        case 'VFO':
          if (msg.args.length >= 3) {
            const hz = Number(msg.args[2]);
            if (Number.isFinite(hz)) this.patchState({ vfoHz: hz });
          }
          break;
        case 'DDS':
          if (msg.args.length >= 3) {
            const hz = Number(msg.args[2]);
            if (Number.isFinite(hz)) this.patchState({ ddsHz: hz });
          }
          break;
        case 'IF':
          if (msg.args.length >= 3) {
            const hz = Number(msg.args[2]);
            if (Number.isFinite(hz)) this.patchState({ ifHz: hz });
          }
          break;
        case 'MODULATION':
          if (msg.args.length >= 2 && msg.args[1]) {
            this.patchState({ modulation: msg.args[1].toUpperCase() });
          }
          break;
        case 'RX_FILTER_BAND':
          if (msg.args.length >= 3) {
            const lo = Number(msg.args[1]);
            const hi = Number(msg.args[2]);
            if (Number.isFinite(lo) && Number.isFinite(hi)) {
              this.patchState({ filterLo: lo, filterHi: hi });
            }
          }
          break;
        case 'RX_SMETER':
          if (msg.args.length >= 3) {
            const dbm = Number(msg.args[2]);
            if (Number.isFinite(dbm)) this.patchState({ smeterDbm: dbm });
          }
          break;
        case 'TRX':
        case 'TX_ENABLE':
          if (msg.args.length >= 2) {
            this.patchState({ trx: msg.args[1].toLowerCase() === 'true' });
          }
          break;
        case 'TUNE':
          if (msg.args.length >= 2) {
            this.patchState({ tune: msg.args[1].toLowerCase() === 'true' });
          }
          break;
        case 'DRIVE':
          if (msg.args.length >= 2) {
            const level = Number(msg.args[1]);
            if (Number.isFinite(level)) this.patchState({ drive: level });
          }
          break;
        case 'TX_SENSORS':
          if (msg.args.length >= 5) {
            const fwd = Number(msg.args[2]);
            const swr = Number(msg.args[4]);
            const patch: Partial<RadioState> = {};
            if (Number.isFinite(fwd)) patch.txPowerWatts = fwd;
            if (Number.isFinite(swr)) patch.swr = swr;
            if (Object.keys(patch).length) this.patchState(patch);
          }
          break;
        case 'TX_POWER':
        case 'POWER':
          if (msg.args.length >= 2) {
            const watts = Number(msg.args[1]);
            if (Number.isFinite(watts)) this.patchState({ txPowerWatts: watts });
          }
          break;
        case 'TX_SWR':
        case 'SWR':
          if (msg.args.length >= 2) {
            const swr = Number(msg.args[1]);
            if (Number.isFinite(swr)) this.patchState({ swr });
          }
          break;
        case 'IQ_SAMPLERATE':
          if (msg.args.length >= 1) {
            const rate = Number(msg.args[0]);
            if (Number.isFinite(rate) && rate > 0) {
              this.patchState({ iqSampleRate: rate });
              this.iqWorker?.postMessage({ type: 'sampleRate', sampleRate: rate });
            }
          }
          break;
        case 'AUDIO_SAMPLERATE':
          if (msg.args.length >= 1) {
            const rate = Number(msg.args[0]);
            if (Number.isFinite(rate) && rate > 0) {
              this.patchState({ audioSampleRate: rate });
              this.audioPlayer.setSampleRate(rate);
            }
          }
          break;
        default:
          break;
      }
    }
  }

  private handleBinary(buf: ArrayBuffer): void {
    const frame = parseStreamFrame(buf);
    if (!frame) return;

    if (frame.type === STREAM_IQ) {
      const payload = frame.payload.slice();
      this.iqWorker?.postMessage(
        { type: 'iq', buffer: payload.buffer, sampleRate: this.state.iqSampleRate },
        [payload.buffer],
      );
      return;
    }

    if (frame.type === STREAM_RX_AUDIO && this.state.rxAudioOn) {
      let payload: Float32Array;
      if (frame.format === 0) {
        const ints = new Int16Array(buf, STREAM_HEADER_BYTES);
        payload = new Float32Array(ints.length);
        for (let i = 0; i < ints.length; i++) payload[i] = ints[i] / 32768;
      } else {
        payload = frame.payload.slice();
      }
      this.audioPlayer.playFrame(payload, frame.channels, frame.sampleRate, frame.format);
    }
  }

  private startIqWorker(): void {
    this.stopIqWorker();
    this.iqWorker = new Worker(new URL('../dsp/iqWorker.ts', import.meta.url), { type: 'module' });
    this.iqWorker.postMessage({ type: 'setAvg', value: this.state.specAvg });
    this.iqWorker.postMessage({ type: 'setAvgMode', value: this.state.specAvgMode });
    this.iqWorker.postMessage({ type: 'setAvgDomain', value: this.state.specAvgDomain });
    this.iqWorker.onmessage = (ev: MessageEvent<{ type: string; magnitudes?: Float32Array; sampleRate?: number }>) => {
      if (ev.data.type === 'fft' && ev.data.magnitudes) {
        this.patchState({ fftReady: true });
        this.handlers.onFft?.(ev.data.magnitudes, ev.data.sampleRate ?? this.state.iqSampleRate);
      }
    };
  }

  private stopIqWorker(): void {
    this.iqWorker?.terminate();
    this.iqWorker = null;
  }

  private resetSpectrumAvg(): void {
    this.iqWorker?.postMessage({ type: 'resetAvg' });
  }

  private scheduleReconnect(url: string): void {
    if (this.reconnectTimer) return;
    const delay = Math.min(this.reconnectDelay, this.reconnectMax);
    this.log('sys', `Reconnecting in ${(delay / 1000).toFixed(1)}s…`);
    this.reconnectTimer = setTimeout(() => {
      this.reconnectTimer = null;
      if (!this.userClosed) void this.connect(url);
    }, delay);
    this.reconnectDelay = Math.min(this.reconnectDelay * 2, this.reconnectMax);
  }

  private patchState(patch: Partial<RadioState>): void {
    this.state = { ...this.state, ...patch };
    this.handlers.onState?.(this.getState());
  }

  private log(level: 'sys' | 'err', message: string): void {
    this.handlers.onLog?.(level, message);
  }
}
