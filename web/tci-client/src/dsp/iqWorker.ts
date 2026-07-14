import FFT from 'fft.js';
import { FFT_SIZE } from '../protocol/types';

const N = FFT_SIZE;
const fft = new FFT(N);
const fftIn = fft.createComplexArray();
const fftOut = fft.createComplexArray();

// Circular sample history so we can run overlapped FFTs (rolling window).
const ringI = new Float32Array(N);
const ringQ = new Float32Array(N);
const window = new Float32Array(N);
const magnitudes = new Float32Array(N);

/** Moving-average length / time-constant — adjustable from UI. */
let avgCnt = 4;

/** Averaging method: 0 = None, 1 = Box-car, 2 = Exponential. */
const AVG_NONE = 0;
const AVG_BOXCAR = 1;
const AVG_EXPONENTIAL = 2;
let avgMode = AVG_BOXCAR;

/** Averaging domain: average dB values ('log') or linear power then convert ('linear'). */
let avgDomain: 'log' | 'linear' = 'log';

/** Box-car ring of per-bin frames (units depend on domain: dB or linear power). */
const avgRing: Float32Array[] = [];

/** Exponential running frame (units depend on domain); null until first frame after reset. */
let emaFrame: Float32Array | null = null;

/**
 * Overlap hop: run one FFT every HOP new samples over the last N samples.
 * HOP = N/4 → 75% overlap → 4× the spectrum frame rate vs. non-overlapped,
 * decoupling refresh rate from FFT size without needing more IQ data.
 */
const HOP = N >> 2;

let writePos = 0;
let sinceLastFft = 0;
let filled = 0;
let sampleRate = 192_000;

/**
 * After an isolated frequency change we briefly stop emitting until the rolling
 * window is full of fresh samples again, so we never FFT across the discontinuity
 * (which flashes as a noise burst). During *continuous* tuning the resets arrive
 * faster than the window refills — in that case we clear suppression and keep
 * emitting live frames so the pan never freezes.
 */
let suppressing = false;
let freshSinceReset = 0;

for (let i = 0; i < N; i++) {
  const x = (2 * Math.PI * i) / (N - 1);
  window[i] =
    0.35875 - 0.48829 * Math.cos(x) + 0.14128 * Math.cos(2 * x) - 0.01168 * Math.cos(3 * x);
}

function resetAveraging(): void {
  avgRing.length = 0;
  emaFrame = null;
}

/** Convert a bin value expressed in the current domain to display dB. */
function toDb(value: number): number {
  if (avgDomain === 'linear') {
    return value > 1e-20 ? 10 * Math.log10(value) - 70 : -150;
  }
  return value;
}

function runFft(): void {
  // Read the last N samples in chronological order (oldest first).
  for (let i = 0; i < N; i++) {
    const idx = (writePos + i) % N;
    fftIn[i * 2] = ringI[idx] * window[i];
    fftIn[i * 2 + 1] = ringQ[idx] * window[i];
  }

  fft.transform(fftOut, fftIn);

  // Compute the raw frame in the selected domain's units: linear power or dB.
  const frame = new Float32Array(N);
  const half = N >> 1;
  const linear = avgDomain === 'linear';
  for (let i = 0; i < N; i++) {
    const idx = (i + half) % N;
    const re = fftOut[idx * 2];
    const im = fftOut[idx * 2 + 1];
    const mag = re * re + im * im;
    frame[i] = linear ? mag : mag > 1e-20 ? 10 * Math.log10(mag) - 70 : -150;
  }

  if (avgMode === AVG_NONE || avgCnt <= 1) {
    for (let i = 0; i < N; i++) magnitudes[i] = toDb(frame[i]);
  } else if (avgMode === AVG_EXPONENTIAL) {
    // Single-pole IIR: alpha derived from avgCnt (EMA equivalent of N-sample avg).
    const alpha = 2 / (avgCnt + 1);
    if (!emaFrame) {
      emaFrame = frame.slice();
    } else {
      const y = emaFrame;
      for (let i = 0; i < N; i++) y[i] = alpha * frame[i] + (1 - alpha) * y[i];
    }
    for (let i = 0; i < N; i++) magnitudes[i] = toDb(emaFrame[i]);
  } else {
    // Box-car: equal-weight moving average over the last avgCnt frames.
    avgRing.push(frame);
    while (avgRing.length > avgCnt) avgRing.shift();
    const n = avgRing.length;
    for (let i = 0; i < N; i++) {
      let sum = 0;
      for (let f = 0; f < n; f++) sum += avgRing[f][i];
      magnitudes[i] = toDb(sum / n);
    }
  }

  const out = magnitudes.slice();
  // In a dedicated worker `self.postMessage` accepts a transfer list; TS resolves
  // `self` to the DOM Window overload, so cast to the transferable-aware form.
  (self.postMessage as (message: unknown, transfer: Transferable[]) => void)(
    { type: 'fft', magnitudes: out, sampleRate },
    [out.buffer],
  );
}

function ingest(payload: Float32Array): void {
  const pairCount = Math.floor(payload.length / 2);
  for (let i = 0; i < pairCount; i++) {
    // HPSDR legacy: cudaSDR TCI IQ is Q,I — swap for correct spectrum sign.
    ringI[writePos] = payload[i * 2 + 1];
    ringQ[writePos] = payload[i * 2];
    writePos = (writePos + 1) % N;
    if (filled < N) filled++;
    sinceLastFft++;
    if (suppressing) {
      freshSinceReset++;
      if (freshSinceReset >= N) suppressing = false;
    }
    // Only emit once the window is full, a hop's worth of new data arrived, and
    // we're not suppressing a post-tune transient.
    if (filled >= N && sinceLastFft >= HOP && !suppressing) {
      sinceLastFft = 0;
      runFft();
    }
  }
}

self.onmessage = (
  ev: MessageEvent<{
    type: string;
    buffer?: ArrayBuffer;
    sampleRate?: number;
    value?: number | string;
  }>,
) => {
  const data = ev.data;
  if (data.type === 'resetAvg') {
    resetAveraging();
    if (suppressing) {
      // Another change arrived before the window refilled → continuous tuning.
      // Stop suppressing so the pan keeps updating live instead of freezing.
      suppressing = false;
    } else {
      // Isolated change: suppress briefly so the next FFT uses a clean window.
      suppressing = true;
      freshSinceReset = 0;
    }
    return;
  }
  if (data.type === 'setAvg' && typeof data.value === 'number') {
    avgCnt = Math.max(1, Math.round(data.value));
    resetAveraging();
    return;
  }
  if (data.type === 'setAvgMode' && typeof data.value === 'number') {
    avgMode = Math.round(data.value);
    resetAveraging();
    return;
  }
  if (data.type === 'setAvgDomain' && typeof data.value === 'string') {
    avgDomain = data.value === 'linear' ? 'linear' : 'log';
    resetAveraging();
    return;
  }
  if (data.type === 'sampleRate' && data.sampleRate) {
    sampleRate = data.sampleRate;
    return;
  }
  if (data.type === 'iq' && data.buffer) {
    if (data.sampleRate && data.sampleRate > 0) sampleRate = data.sampleRate;
    ingest(new Float32Array(data.buffer));
  }
};
