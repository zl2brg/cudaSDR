/**
 * Microphone capture for TX (transmit) audio.
 *
 * Captures the local microphone via getUserMedia, resamples to a fixed 48 kHz
 * mono stream, chunks it into fixed-size frames and hands each frame to the
 * supplied callback (which serialises it as a TCI TX-audio binary frame).
 *
 * getUserMedia only works in a secure context (https or http://localhost), so
 * start() throws a descriptive error when navigator.mediaDevices is missing.
 *
 * A ScriptProcessorNode is used (rather than an AudioWorklet) to avoid the
 * bundler complexity of shipping a separate worklet module; it is deprecated
 * but universally supported and adequate for a single mono 48 kHz mic stream.
 */
export type TxAudioFrameCallback = (frame: Float32Array, sampleRate: number) => void;

const TARGET_RATE = 48_000;
const FRAME_SIZE = 1024;
const CAPTURE_BUFFER = 2048;

export class TxAudioCapture {
  private ctx: AudioContext | null = null;
  private stream: MediaStream | null = null;
  private source: MediaStreamAudioSourceNode | null = null;
  private processor: ScriptProcessorNode | null = null;
  private sink: GainNode | null = null;
  private active = false;
  private readonly onFrame: TxAudioFrameCallback;

  // Streaming linear resampler state (input rate -> 48 kHz).
  private prevSample = 0;
  private readPos = 0;

  // Accumulator for producing fixed-size output frames.
  private accum: number[] = [];

  constructor(onFrame: TxAudioFrameCallback) {
    this.onFrame = onFrame;
  }

  isActive(): boolean {
    return this.active;
  }

  async start(): Promise<void> {
    if (this.active) return;

    if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
      throw new Error(
        'Microphone unavailable: open the app over https:// (or http://localhost) to grant mic access.',
      );
    }

    this.stream = await navigator.mediaDevices.getUserMedia({
      audio: {
        channelCount: 1,
        echoCancellation: true,
        noiseSuppression: true,
        autoGainControl: true,
      },
    });

    const Ctor =
      window.AudioContext ??
      (window as unknown as { webkitAudioContext: typeof AudioContext }).webkitAudioContext;
    if (!Ctor) {
      this.releaseStream();
      throw new Error('Web Audio API not available');
    }

    this.ctx = new Ctor();
    if (this.ctx.state === 'suspended') {
      await this.ctx.resume();
    }

    this.prevSample = 0;
    this.readPos = 0;
    this.accum = [];

    this.source = this.ctx.createMediaStreamSource(this.stream);
    this.processor = this.ctx.createScriptProcessor(CAPTURE_BUFFER, 1, 1);
    this.processor.onaudioprocess = (ev: AudioProcessingEvent) => {
      this.handleInput(ev.inputBuffer.getChannelData(0));
    };

    // ScriptProcessorNode only fires while connected to the graph. Route it to
    // destination through a muted gain so the mic is never echoed to speakers.
    this.sink = this.ctx.createGain();
    this.sink.gain.value = 0;
    this.source.connect(this.processor);
    this.processor.connect(this.sink);
    this.sink.connect(this.ctx.destination);

    this.active = true;
  }

  stop(): void {
    this.active = false;
    if (this.processor) {
      this.processor.onaudioprocess = null;
      this.processor.disconnect();
      this.processor = null;
    }
    if (this.source) {
      this.source.disconnect();
      this.source = null;
    }
    if (this.sink) {
      this.sink.disconnect();
      this.sink = null;
    }
    void this.ctx?.close();
    this.ctx = null;
    this.releaseStream();
    this.accum = [];
  }

  private releaseStream(): void {
    if (this.stream) {
      for (const track of this.stream.getTracks()) track.stop();
      this.stream = null;
    }
  }

  private handleInput(input: Float32Array): void {
    if (!this.ctx || input.length === 0) return;

    const inRate = this.ctx.sampleRate;
    if (inRate === TARGET_RATE) {
      // Fast path: no resampling needed.
      for (let i = 0; i < input.length; i++) this.accum.push(input[i]);
    } else {
      this.resampleInto(input, inRate);
    }

    while (this.accum.length >= FRAME_SIZE) {
      const frame = new Float32Array(this.accum.slice(0, FRAME_SIZE));
      this.accum.splice(0, FRAME_SIZE);
      this.onFrame(frame, TARGET_RATE);
    }
  }

  /**
   * Streaming linear resampler. Virtual sample space V has V[0] = prevSample
   * (last sample of the previous block) and V[1..N] = input[0..N-1]. readPos is
   * the continuous read cursor carried across blocks.
   */
  private resampleInto(input: Float32Array, inRate: number): void {
    const step = inRate / TARGET_RATE;
    const n = input.length;

    while (this.readPos < n) {
      const i = Math.floor(this.readPos);
      const t = this.readPos - i;
      const s0 = i === 0 ? this.prevSample : input[i - 1];
      const s1 = input[i];
      this.accum.push(s0 + (s1 - s0) * t);
      this.readPos += step;
    }

    this.readPos -= n;
    this.prevSample = input[n - 1];
  }
}
