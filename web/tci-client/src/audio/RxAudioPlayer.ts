/**
 * Gapless RX audio playback from TCI RX_AUDIO binary frames.
 */
export class RxAudioPlayer {
  private ctx: AudioContext | null = null;
  private gain: GainNode | null = null;
  private nextTime = 0;
  private active = false;
  private volume = 0.56;
  private muted = false;
  private nominalRate = 48_000;

  isActive(): boolean {
    return this.active;
  }

  async start(): Promise<void> {
    this.stop();
    const Ctor = window.AudioContext ?? (window as unknown as { webkitAudioContext: typeof AudioContext }).webkitAudioContext;
    if (!Ctor) throw new Error('Web Audio API not available');

    this.ctx = new Ctor();
    this.gain = this.ctx.createGain();
    this.applyGain();
    this.gain.connect(this.ctx.destination);

    if (this.ctx.state === 'suspended') {
      await this.ctx.resume();
    }

    this.nextTime = this.ctx.currentTime + 0.05;
    this.active = true;
  }

  stop(): void {
    this.active = false;
    void this.ctx?.close();
    this.ctx = null;
    this.gain = null;
    this.nextTime = 0;
  }

  setVolume(percent: number): void {
    this.volume = Math.max(0, Math.min(1, percent / 100));
    this.applyGain();
  }

  setMuted(muted: boolean): void {
    this.muted = muted;
    this.applyGain();
  }

  setSampleRate(rate: number): void {
    if (rate > 0) this.nominalRate = rate;
  }

  playFrame(
    payload: Float32Array,
    channels: number,
    sampleRate: number,
    _format: number,
  ): void {
    if (!this.active || !this.ctx || !this.gain) return;
    if (this.ctx.state === 'suspended') void this.ctx.resume();

    const rate = sampleRate > 0 ? sampleRate : this.nominalRate;
    const frames = channels === 1 ? payload.length : Math.floor(payload.length / 2);
    if (frames < 1) return;

    const ab = this.ctx.createBuffer(2, frames, rate);
    const L = ab.getChannelData(0);
    const R = ab.getChannelData(1);

    // Do NOT window every packet — fade-in/out on each block AM-modulates at
    // the packet rate (~21 ms → ~47 Hz buzz) while leaving content pitch intact.
    // Only a short fade-in when recovering from an underrun avoids a click.
    const now = this.ctx.currentTime;
    const recovering = this.nextTime < now;
    const fadeIn = recovering ? Math.min(64, frames >> 2) : 0;

    if (channels === 1) {
      for (let i = 0; i < frames; i++) {
        const s = fadeIn > 0 && i < fadeIn ? i / fadeIn : 1;
        const v = payload[i] * s;
        L[i] = v;
        R[i] = v;
      }
    } else {
      for (let i = 0; i < frames; i++) {
        const s = fadeIn > 0 && i < fadeIn ? i / fadeIn : 1;
        L[i] = payload[i * 2] * s;
        R[i] = payload[i * 2 + 1] * s;
      }
    }

    if (recovering) this.nextTime = now + 0.02;

    const src = this.ctx.createBufferSource();
    src.buffer = ab;
    src.connect(this.gain);
    src.start(this.nextTime);
    this.nextTime += ab.duration;
  }

  private applyGain(): void {
    if (!this.gain) return;
    this.gain.gain.value = this.muted ? 0 : this.volume;
  }
}
