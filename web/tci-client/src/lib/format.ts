export function dbToColor(t: number): [number, number, number] {
  // Stretch low end so the noise floor is visible against black.
  const clamped = Math.max(0, Math.min(1, t));
  const x = Math.pow(clamped, 0.75);
  if (x < 0.12) {
    const u = x / 0.12;
    return [0, Math.round(20 * u), Math.round(40 + 80 * u)];
  }
  if (x < 0.35) {
    const u = (x - 0.12) / 0.23;
    return [0, Math.round(20 + 140 * u), Math.round(120 + 100 * u)];
  }
  if (x < 0.6) {
    const u = (x - 0.35) / 0.25;
    return [Math.round(200 * u), Math.round(160 + 80 * u), Math.round(220 - 180 * u)];
  }
  const u = (x - 0.6) / 0.4;
  return [255, Math.round(240 - 80 * u), Math.round(40 - 40 * u)];
}

export function formatFrequency(hz: number): string {
  const mhz = hz / 1_000_000;
  const whole = Math.floor(mhz);
  const frac = Math.round((mhz - whole) * 1_000);
  return `${whole}.${frac.toString().padStart(3, '0')}.000 MHz`;
}

export type FreqDisplayPart = {
  text: string;
  stepHz: number;
};

/** Digit groups for cudaSDR-style wheel tuning (step depends on hovered digit). */
export function buildFreqDisplayParts(hz: number): FreqDisplayPart[] {
  const rounded = Math.max(0, Math.round(hz));
  const mhz = Math.floor(rounded / 1_000_000);
  const belowMhz = rounded % 1_000_000;
  const khz = Math.floor(belowMhz / 1_000);
  const hzPart = belowMhz % 1_000;

  const wholeStr = mhz.toString();
  const khzStr = khz.toString().padStart(3, '0');
  const hzStr = hzPart.toString().padStart(3, '0');

  const parts: FreqDisplayPart[] = [];
  for (let i = 0; i < wholeStr.length; i++) {
    const place = wholeStr.length - 1 - i;
    parts.push({ text: wholeStr[i], stepHz: 10 ** (place + 6) });
  }
  parts.push({ text: '.', stepHz: 0 });
  for (let i = 0; i < 3; i++) {
    parts.push({ text: khzStr[i], stepHz: 10 ** (5 - i) });
  }
  parts.push({ text: '.', stepHz: 0 });
  for (let i = 0; i < 3; i++) {
    parts.push({ text: hzStr[i], stepHz: 10 ** (2 - i) });
  }
  parts.push({ text: ' MHz', stepHz: 0 });
  return parts;
}

export function wheelFrequencySteps(deltaY: number): number {
  if (deltaY === 0) return 0;
  const sign = deltaY < 0 ? 1 : -1;
  const notches = Math.max(1, Math.round(Math.abs(deltaY) / 100));
  return sign * notches;
}

export function formatSmeter(dbm: number): string {
  return `${dbm.toFixed(1)} dBm`;
}

export function filterLabel(lo: number, hi: number, mode: string): string {
  const width = Math.abs(hi - lo);
  let label: string;
  if (width >= 1000) label = `${(width / 1000).toFixed(1)}k`;
  else label = `${Math.round(width)}`;
  return `${label} ${mode}`;
}
