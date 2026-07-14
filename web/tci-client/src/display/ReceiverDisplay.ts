import { DB_MAX, DB_MIN, FFT_SIZE, type RadioState } from '../protocol/types';
import { dbToColor, filterLabel } from '../lib/format';

/** CSS pixels — keep in sync with app.css receiver-grid. */
const DBM_COL_W = 56;
const FREQ_RULER_H = 26;

export type ReceiverDisplayOptions = {
  specGain?: number;
  zoom?: number;
  wfSpeed?: number;
};

export class ReceiverDisplay {
  private panCanvas: HTMLCanvasElement;
  private dbmCanvas: HTMLCanvasElement;
  private freqCanvas: HTMLCanvasElement;
  private wfCanvas: HTMLCanvasElement;
  private secCanvas: HTMLCanvasElement;

  private panCtx: CanvasRenderingContext2D;
  private dbmCtx: CanvasRenderingContext2D;
  private freqCtx: CanvasRenderingContext2D;
  private wfCtx: CanvasRenderingContext2D;
  private secCtx: CanvasRenderingContext2D;
  private wfBuffer: HTMLCanvasElement;
  private wfBufCtx: CanvasRenderingContext2D;

  private fftResult: Float32Array | null = null;
  private wfPendingRows = 0;
  private rafId = 0;
  private state: RadioState | null = null;
  private gridEl: HTMLDivElement;

  specGain = -5;
  wfGain = 8;
  zoom = 1;
  zoomCentreHz = 0;

  /** Adjustable dB window (reference level) — dragged via the dBm scale. */
  private dbMin = DB_MIN;
  private dbMax = DB_MAX;
  private dbmDragPointerId = -1;
  private dbmDragLastY = 0;

  constructor(root: HTMLElement) {
    const grid = document.createElement('div');
    grid.className = 'receiver-grid';
    root.appendChild(grid);
    this.gridEl = grid;

    this.dbmCanvas = this.makeCanvas('dbm-scale');
    this.panCanvas = this.makeCanvas('panadapter');
    this.freqCanvas = this.makeCanvas('freq-ruler');
    this.secCanvas = this.makeCanvas('sec-scale');
    this.wfCanvas = this.makeCanvas('waterfall');

    grid.append(this.dbmCanvas, this.panCanvas, this.freqCanvas, this.secCanvas, this.wfCanvas);

    this.dbmCtx = this.dbmCanvas.getContext('2d')!;
    this.panCtx = this.panCanvas.getContext('2d')!;
    this.freqCtx = this.freqCanvas.getContext('2d')!;
    this.wfCtx = this.wfCanvas.getContext('2d')!;
    this.secCtx = this.secCanvas.getContext('2d')!;
    this.wfBuffer = document.createElement('canvas');
    this.wfBufCtx = this.wfBuffer.getContext('2d', { willReadFrequently: true })!;

    this.panCanvas.addEventListener('wheel', (e) => this.onWheel(e), { passive: false });
    this.wfCanvas.addEventListener('wheel', (e) => this.onWheel(e), { passive: false });
    this.bindTuneCanvas(this.panCanvas);
    this.bindTuneCanvas(this.wfCanvas);
    this.bindDbmScale(this.dbmCanvas);

    this.resizeObserver = new ResizeObserver(() => this.resize());
    this.resizeObserver.observe(grid);
    this.resizeObserver.observe(root);
    requestAnimationFrame(() => {
      requestAnimationFrame(() => {
        this.resize();
        this.loop();
      });
    });
  }

  private resizeObserver: ResizeObserver;
  onTune: ((hz: number) => void) | null = null;
  onDragTune: ((deltaHz: number) => void) | null = null;

  private tunePointerId = -1;
  private tuneDragLastX = 0;
  private tuneDownX = 0;
  private tuneDidDrag = false;

  /** Active pointers per canvas (pointerId -> canvas-space x) for pinch zoom. */
  private activePointers = new Map<number, number>();
  private pinching = false;
  private pinchStartDist = 0;
  private pinchStartZoom = 1;

  destroy(): void {
    cancelAnimationFrame(this.rafId);
    this.resizeObserver.disconnect();
  }

  setState(state: RadioState): void {
    this.state = state;
    if (this.zoom === 1 || this.zoomCentreHz === 0) {
      this.zoomCentreHz = state.ddsHz || state.vfoHz;
    }
  }

  setFft(magnitudes: Float32Array): void {
    this.fftResult = magnitudes;
    this.wfPendingRows = Math.min(this.wfPendingRows + 1, 16);
  }

  private makeCanvas(className: string): HTMLCanvasElement {
    const c = document.createElement('canvas');
    c.className = className;
    return c;
  }

  private dpr(): number {
    return window.devicePixelRatio || 1;
  }

  /** Map CSS px to canvas bitmap px (HiDPI-aware text and strokes). */
  private px(cssPx: number): number {
    return cssPx * this.dpr();
  }

  private resize(): void {
    const dpr = this.dpr();
    const rect = this.gridEl.getBoundingClientRect();
    if (rect.width < 8 || rect.height < 8) return;

    const dbmW = DBM_COL_W;
    const rulerH = FREQ_RULER_H;
    const mainW = rect.width - dbmW;
    const mainH = rect.height - rulerH;
    const panH = mainH / (1 + 0.55);
    const wfH = mainH - panH;

    let changed = false;
    changed = this.setCanvasSize(this.dbmCanvas, dbmW * dpr, panH * dpr) || changed;
    changed = this.setCanvasSize(this.panCanvas, mainW * dpr, panH * dpr) || changed;
    changed = this.setCanvasSize(this.freqCanvas, rect.width * dpr, rulerH * dpr) || changed;
    changed = this.setCanvasSize(this.secCanvas, dbmW * dpr, wfH * dpr) || changed;
    changed = this.setCanvasSize(this.wfCanvas, mainW * dpr, wfH * dpr) || changed;

    if (changed) {
      this.clearWfBuffer();
      this.wfCtx.fillStyle = '#010409';
      this.wfCtx.fillRect(0, 0, this.wfCanvas.width, this.wfCanvas.height);
    }
  }

  private clearWfBuffer(): void {
    const W = this.wfCanvas.width;
    const H = this.wfCanvas.height;
    if (W < 1 || H < 1) return;
    this.wfBuffer.width = W;
    this.wfBuffer.height = H;
    this.wfBufCtx.fillStyle = '#010409';
    this.wfBufCtx.fillRect(0, 0, W, H);
  }

  private setCanvasSize(canvas: HTMLCanvasElement, w: number, h: number): boolean {
    const bw = Math.max(2, Math.floor(w));
    const bh = Math.max(2, Math.floor(h));
    if (canvas.width === bw && canvas.height === bh) return false;
    canvas.width = bw;
    canvas.height = bh;
    return true;
  }

  private hasFft(): boolean {
    return this.fftResult !== null && this.fftResult.length === FFT_SIZE;
  }

  private centreHz(): number {
    return this.state?.ddsHz ?? this.state?.vfoHz ?? 0;
  }

  private sampleRate(): number {
    return this.state?.iqSampleRate ?? 192_000;
  }

  private visibleRange(): { lo: number; hi: number; sr: number } {
    const sr = this.sampleRate();
    const centre = this.centreHz();
    const lo = centre - sr / 2;
    const hi = centre + sr / 2;
    if (this.zoom <= 1) {
      this.zoomCentreHz = centre;
      return { lo, hi, sr };
    }
    const visSr = sr / this.zoom;
    this.zoomCentreHz = Math.max(lo + visSr / 2, Math.min(hi - visSr / 2, this.zoomCentreHz || centre));
    return { lo: this.zoomCentreHz - visSr / 2, hi: this.zoomCentreHz + visSr / 2, sr: visSr };
  }

  private hzToX(hz: number, W: number, lo: number, visSr: number): number {
    return ((hz - lo) / visSr) * W;
  }

  private xToHz(x: number, W: number, lo: number, visSr: number): number {
    return lo + (x / W) * visSr;
  }

  /** Linear interpolation between adjacent FFT bins at the given frequency. */
  private interpDb(
    hz: number,
    fft: Float32Array,
    fullLo: number,
    fullSr: number,
    gain = 0,
  ): number {
    const N = fft.length;
    const t = ((hz - fullLo) / fullSr) * N;
    const b0 = Math.max(0, Math.min(N - 1, Math.floor(t)));
    const b1 = Math.min(N - 1, b0 + 1);
    const frac = t - b0;
    const db = fft[b0] * (1 - frac) + fft[b1] * frac;
    return Math.max(this.dbMin, Math.min(this.dbMax, db + gain));
  }

  private pointerCanvasX(e: PointerEvent, canvas: HTMLCanvasElement): number {
    const rect = canvas.getBoundingClientRect();
    return ((e.clientX - rect.left) / rect.width) * canvas.width;
  }

  private bindTuneCanvas(canvas: HTMLCanvasElement): void {
    canvas.style.cursor = 'crosshair';
    canvas.style.touchAction = 'none';
    canvas.addEventListener('pointerdown', (e) => this.onTunePointerDown(e, canvas));
    canvas.addEventListener('pointermove', (e) => this.onTunePointerMove(e, canvas));
    canvas.addEventListener('pointerup', (e) => this.onTunePointerUp(e, canvas));
    canvas.addEventListener('pointercancel', (e) => this.onTunePointerUp(e, canvas));
  }

  private onTunePointerDown(e: PointerEvent, canvas: HTMLCanvasElement): void {
    if (e.button !== 0 && e.pointerType === 'mouse') return;
    if (!this.onTune && !this.onDragTune) return;
    canvas.setPointerCapture(e.pointerId);
    this.activePointers.set(e.pointerId, this.pointerCanvasX(e, canvas));

    if (this.activePointers.size >= 2) {
      // Second finger down: start a pinch and abandon any single-finger tune drag.
      this.beginPinch();
      return;
    }

    this.tunePointerId = e.pointerId;
    this.tuneDragLastX = this.pointerCanvasX(e, canvas);
    this.tuneDownX = this.tuneDragLastX;
    this.tuneDidDrag = false;
  }

  private beginPinch(): void {
    const xs = [...this.activePointers.values()];
    this.pinchStartDist = Math.max(1, Math.abs(xs[0] - xs[1]));
    this.pinchStartZoom = this.zoom;
    this.pinching = true;
    this.tunePointerId = -1;
    this.tuneDidDrag = false;
  }

  private onTunePointerMove(e: PointerEvent, canvas: HTMLCanvasElement): void {
    if (this.activePointers.has(e.pointerId)) {
      this.activePointers.set(e.pointerId, this.pointerCanvasX(e, canvas));
    }

    if (this.pinching && this.activePointers.size >= 2) {
      const xs = [...this.activePointers.values()];
      const dist = Math.max(1, Math.abs(xs[0] - xs[1]));
      // Spread fingers apart → expand (zoom in); pinch together → contract.
      const next = this.pinchStartZoom * (dist / this.pinchStartDist);
      this.zoom = Math.max(1, Math.min(32, next));
      return;
    }

    if (e.pointerId !== this.tunePointerId || !this.onDragTune) return;
    const x = this.pointerCanvasX(e, canvas);
    const dx = this.tuneDragLastX - x;
    this.tuneDragLastX = x;
    if (!this.tuneDidDrag && Math.abs(x - this.tuneDownX) > this.px(4)) {
      this.tuneDidDrag = true;
      canvas.style.cursor = 'grabbing';
    }
    if (!this.tuneDidDrag) return;
    const { sr } = this.visibleRange();
    const deltaHz = dx * (sr / canvas.width);
    if (deltaHz !== 0) this.onDragTune(deltaHz);
  }

  private onTunePointerUp(e: PointerEvent, canvas: HTMLCanvasElement): void {
    const wasTracked = this.activePointers.delete(e.pointerId);
    if (canvas.hasPointerCapture?.(e.pointerId)) canvas.releasePointerCapture(e.pointerId);

    if (this.pinching) {
      // Stay in pinch until both fingers lift; don't emit a stray tune.
      if (this.activePointers.size < 2) this.pinching = false;
      this.tuneDidDrag = false;
      return;
    }

    if (e.pointerId !== this.tunePointerId) {
      if (!wasTracked) return;
      return;
    }
    this.tunePointerId = -1;
    canvas.style.cursor = 'crosshair';
    if (!this.tuneDidDrag && this.onTune) {
      const x = this.pointerCanvasX(e, canvas);
      const { lo, sr } = this.visibleRange();
      const hz = this.xToHz(x, canvas.width, lo, sr);
      this.onTune(Math.round(hz));
    }
    this.tuneDidDrag = false;
  }

  private onWheel(e: WheelEvent): void {
    e.preventDefault();
    if (e.deltaY < 0) this.zoom = Math.min(32, this.zoom * 2);
    else this.zoom = Math.max(1, this.zoom / 2);
  }

  private pointerCanvasY(e: PointerEvent, canvas: HTMLCanvasElement): number {
    const rect = canvas.getBoundingClientRect();
    return ((e.clientY - rect.top) / rect.height) * canvas.height;
  }

  private bindDbmScale(canvas: HTMLCanvasElement): void {
    canvas.style.cursor = 'ns-resize';
    canvas.style.touchAction = 'none';
    canvas.title = 'Drag up/down to set the reference level';
    canvas.addEventListener('pointerdown', (e) => {
      this.dbmDragPointerId = e.pointerId;
      this.dbmDragLastY = this.pointerCanvasY(e, canvas);
      canvas.setPointerCapture(e.pointerId);
    });
    canvas.addEventListener('pointermove', (e) => {
      if (e.pointerId !== this.dbmDragPointerId) return;
      const y = this.pointerCanvasY(e, canvas);
      const dy = y - this.dbmDragLastY;
      this.dbmDragLastY = y;
      this.shiftReferenceLevel(dy, canvas.height);
    });
    const end = (e: PointerEvent) => {
      if (e.pointerId !== this.dbmDragPointerId) return;
      this.dbmDragPointerId = -1;
      if (canvas.hasPointerCapture?.(e.pointerId)) canvas.releasePointerCapture(e.pointerId);
    };
    canvas.addEventListener('pointerup', end);
    canvas.addEventListener('pointercancel', end);
  }

  /** Pan the dB window (reference level) by a vertical drag over the dBm scale. */
  private shiftReferenceLevel(dyPx: number, H: number): void {
    if (H < 1) return;
    const span = this.dbMax - this.dbMin;
    // Drag down → floor darkens (window shifts up in dB); drag up → more colour.
    const deltaDb = (dyPx / H) * span;
    let lo = this.dbMin + deltaDb;
    let hi = this.dbMax + deltaDb;
    const MIN_FLOOR = -180;
    const MAX_CEIL = 0;
    if (lo < MIN_FLOOR) {
      hi += MIN_FLOOR - lo;
      lo = MIN_FLOOR;
    }
    if (hi > MAX_CEIL) {
      lo -= hi - MAX_CEIL;
      hi = MAX_CEIL;
    }
    this.dbMin = lo;
    this.dbMax = hi;
  }

  private loop = (): void => {
    this.draw();
    this.rafId = requestAnimationFrame(this.loop);
  };

  private draw(): void {
    if (this.wfCanvas.height < 8 || this.panCanvas.height < 8) this.resize();
    this.drawDbmScale();
    this.drawPanadapter();
    this.drawFreqRuler();
    this.drawWaterfall();
    this.drawSecScale();
  }

  private drawDbmScale(): void {
    const ctx = this.dbmCtx;
    const W = this.dbmCanvas.width;
    const H = this.dbmCanvas.height;
    if (W < 1 || H < 1) return;

    ctx.fillStyle = '#141820';
    ctx.fillRect(0, 0, W, H);
    ctx.fillStyle = '#6e7681';
    const fontSize = Math.min(this.px(12), W * 0.38);
    ctx.font = `${fontSize}px monospace`;
    ctx.textAlign = 'right';
    const range = this.dbMax - this.dbMin;
    const pad = this.px(4);
    // 10 dB labels across the current (adjustable) window, skipping the edges.
    const first = Math.ceil((this.dbMin + 5) / 10) * 10;
    for (let db = first; db < this.dbMax - 2; db += 10) {
      const y = H * (1 - (db - this.dbMin) / range);
      ctx.fillText(String(db), W - pad, y + fontSize * 0.35);
    }
  }

  private drawPanadapter(): void {
    const ctx = this.panCtx;
    const W = ctx.canvas.width;
    const H = ctx.canvas.height;
    ctx.fillStyle = '#010409';
    ctx.fillRect(0, 0, W, H);

    const connected = this.state?.connected;
    const ready = connected && this.hasFft();

    if (!ready || !this.fftResult) {
      ctx.fillStyle = '#6e7681';
      ctx.font = `${this.px(16)}px monospace`;
      ctx.textAlign = 'center';
      ctx.fillText(
        connected ? 'Waiting for IQ data…' : 'Connect to cudaSDR TCI (port 50001)',
        W / 2,
        H / 2,
      );
      return;
    }

    const { lo, sr } = this.visibleRange();
    const range = this.dbMax - this.dbMin;
    const fullSr = this.sampleRate();
    const fullLo = this.centreHz() - fullSr / 2;

    ctx.strokeStyle = '#1c2128';
    ctx.lineWidth = this.px(1);
    ctx.font = `${this.px(12)}px monospace`;
    ctx.textAlign = 'left';
    const firstGrid = Math.ceil((this.dbMin + 5) / 10) * 10;
    for (let db = firstGrid; db < this.dbMax - 2; db += 10) {
      const y = H * (1 - (db - this.dbMin) / range);
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(W, y);
      ctx.stroke();
    }

    this.drawFilterAndVfo(ctx, W, H, lo, sr);

    ctx.strokeStyle = 'rgba(255, 200, 80, 0.95)';
    ctx.lineWidth = this.px(1.5);
    ctx.beginPath();
    for (let x = 0; x < W; x++) {
      const hz = lo + (x / W) * sr;
      const db = this.interpDb(hz, this.fftResult, fullLo, fullSr, this.specGain);
      const y = H * (1 - (db - this.dbMin) / range);
      if (x === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    }
    ctx.stroke();

    if (this.state) {
      const label = filterLabel(this.state.filterLo, this.state.filterHi, this.state.modulation);
      const top = this.px(18);
      const line = this.px(16);
      ctx.fillStyle = 'rgba(255, 170, 90, 0.9)';
      ctx.font = `bold ${this.px(14)}px monospace`;
      ctx.textAlign = 'center';
      ctx.fillText(label, W / 2, top);
      ctx.fillStyle = 'rgba(255, 170, 90, 0.75)';
      ctx.font = `${this.px(12)}px monospace`;
      ctx.textAlign = 'right';
      ctx.fillText(`sample size: ${FFT_SIZE}`, W - this.px(10), top);
      ctx.fillText('FFT: 4k', W - this.px(10), top + line);
    }
  }

  private drawFilterAndVfo(
    ctx: CanvasRenderingContext2D,
    W: number,
    H: number,
    lo: number,
    visSr: number,
  ): void {
    if (!this.state) return;
    const vfo = this.state.vfoHz;
    const flo = vfo + Math.min(this.state.filterLo, this.state.filterHi);
    const fhi = vfo + Math.max(this.state.filterLo, this.state.filterHi);
    const xLo = this.hzToX(flo, W, lo, visSr);
    const xHi = this.hzToX(fhi, W, lo, visSr);
    const cx1 = Math.max(0, xLo);
    const cx2 = Math.min(W, xHi);
    if (cx2 > cx1) {
      // cudaSDR default panFilterColor is gray (150,150,150) — not orange.
      ctx.fillStyle = 'rgba(150, 150, 150, 0.24)';
      ctx.fillRect(cx1, 0, cx2 - cx1, H);
    }
    const vx = this.hzToX(vfo, W, lo, visSr);
    ctx.strokeStyle = 'rgba(255, 60, 60, 0.85)';
    ctx.lineWidth = this.px(1);
    ctx.setLineDash([]);
    ctx.beginPath();
    ctx.moveTo(vx, 0);
    ctx.lineTo(vx, H);
    ctx.stroke();
  }

  private drawFreqRuler(): void {
    const ctx = this.freqCtx;
    const W = ctx.canvas.width;
    const H = ctx.canvas.height;
    ctx.fillStyle = '#161b22';
    ctx.fillRect(0, 0, W, H);

    const { lo, hi, sr } = this.visibleRange();
    const step = sr > 100_000 ? 20_000 : sr > 40_000 ? 10_000 : sr > 10_000 ? 2_000 : 500;
    const first = Math.ceil(lo / step) * step;

    ctx.strokeStyle = '#30363d';
    ctx.fillStyle = '#c9d1d9';
    ctx.font = `${this.px(12)}px monospace`;
    ctx.textAlign = 'center';
    const labelY = H - this.px(5);
    for (let hz = first; hz <= hi; hz += step) {
      const x = this.hzToX(hz, W, lo, sr);
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, H);
      ctx.stroke();
      const mhz = (hz / 1_000_000).toFixed(3);
      ctx.fillText(mhz, x, labelY);
    }
  }

  private drawWaterfall(): void {
    const ctx = this.wfCtx;
    const buf = this.wfBufCtx;
    let W = ctx.canvas.width;
    let H = ctx.canvas.height;
    if (W < 8 || H < 8) {
      this.resize();
      W = ctx.canvas.width;
      H = ctx.canvas.height;
      if (W < 2 || H < 2) return;
    }

    if (this.wfBuffer.width !== W || this.wfBuffer.height !== H) {
      this.clearWfBuffer();
    }

    if (!this.hasFft() || !this.fftResult) {
      ctx.fillStyle = '#010409';
      ctx.fillRect(0, 0, W, H);
      return;
    }

    buf.imageSmoothingEnabled = false;
    while (this.wfPendingRows > 0) {
      this.wfPendingRows--;
      // Newest row at top; older rows scroll down (cudaSDR convention).
      if (H > 1) {
        buf.drawImage(this.wfBuffer, 0, 0, W, H - 1, 0, 1, W, H - 1);
      }
      this.paintWaterfallRow(buf, W, 0, this.fftResult);
    }

    ctx.imageSmoothingEnabled = false;
    ctx.drawImage(this.wfBuffer, 0, 0);
    const range = this.visibleRange();
    this.drawFilterAndVfo(ctx, W, H, range.lo, range.sr);
  }

  private paintWaterfallRow(
    ctx: CanvasRenderingContext2D,
    W: number,
    y: number,
    fft: Float32Array,
  ): void {
    const { lo, sr } = this.visibleRange();
    const fullSr = this.sampleRate();
    const fullLo = this.centreHz() - fullSr / 2;
    const range = this.dbMax - this.dbMin;
    const gain = this.specGain + this.wfGain;
    const row = ctx.createImageData(W, 1);
    const d = row.data;
    for (let x = 0; x < W; x++) {
      const hz = lo + (x / W) * sr;
      const db = this.interpDb(hz, fft, fullLo, fullSr, gain);
      const t = (db - this.dbMin) / range;
      const [r, g, b] = dbToColor(t);
      const idx = x * 4;
      d[idx] = r;
      d[idx + 1] = g;
      d[idx + 2] = b;
      d[idx + 3] = 255;
    }
    ctx.putImageData(row, 0, y);
  }

  private drawSecScale(): void {
    const ctx = this.secCtx;
    const W = this.secCanvas.width;
    const H = this.secCanvas.height;
    if (W < 1 || H < 1) return;

    ctx.fillStyle = '#141820';
    ctx.fillRect(0, 0, W, H);
    ctx.fillStyle = '#8b949e';
    const fontSize = Math.min(this.px(12), W * 0.38);
    ctx.font = `${fontSize}px monospace`;
    ctx.textAlign = 'right';
    const pad = this.px(4);
    const marks = [-2, -4, -6, -8];
    for (const sec of marks) {
      // Newest at top (-2 s), older toward bottom (-8 s).
      const y = (H * (-sec - 2)) / 6;
      if (y > fontSize && y < H - pad) {
        ctx.fillText(sec.toFixed(1), W - pad, y);
      }
    }
    ctx.fillStyle = '#ef386d';
    ctx.fillText('sec', W - pad, H - pad);
  }
}
