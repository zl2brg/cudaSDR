import { describe, expect, it } from 'vitest';
import {
  buildFreqDisplayParts,
  dbToColor,
  filterLabel,
  formatFrequency,
  formatSmeter,
  wheelFrequencySteps,
} from './format';

describe('formatFrequency', () => {
  it('formats MHz with zero-padded fractional groups', () => {
    expect(formatFrequency(7_050_000)).toBe('7.050.000 MHz');
    expect(formatFrequency(14_200_000)).toBe('14.200.000 MHz');
  });
});

describe('buildFreqDisplayParts', () => {
  it('assigns per-digit wheel step sizes', () => {
    const parts = buildFreqDisplayParts(7_050_000);
    expect(parts.find((p) => p.text === '7')?.stepHz).toBe(1_000_000);
    expect(parts.find((p) => p.text === '5')?.stepHz).toBe(10_000);
    expect(parts.find((p) => p.text === ' MHz')).toBeDefined();
  });

  it('clamps negative frequencies to zero', () => {
    const parts = buildFreqDisplayParts(-100);
    expect(parts.some((p) => p.text === '0')).toBe(true);
  });
});

describe('wheelFrequencySteps', () => {
  it('returns zero for no wheel movement', () => {
    expect(wheelFrequencySteps(0)).toBe(0);
  });

  it('steps up on negative deltaY and down on positive', () => {
    expect(wheelFrequencySteps(-100)).toBe(1);
    expect(wheelFrequencySteps(100)).toBe(-1);
    expect(wheelFrequencySteps(-250)).toBe(3);
  });
});

describe('formatSmeter', () => {
  it('formats one decimal place with dBm suffix', () => {
    expect(formatSmeter(-73.4)).toBe('-73.4 dBm');
  });
});

describe('filterLabel', () => {
  it('labels kHz widths and Hz widths', () => {
    expect(filterLabel(-1500, 1500, 'USB')).toBe('3.0k USB');
    expect(filterLabel(-200, 200, 'CW')).toBe('400 CW');
  });
});

describe('dbToColor', () => {
  it('returns RGB tuples within byte range', () => {
    for (const t of [0, 0.25, 0.5, 0.75, 1]) {
      const [r, g, b] = dbToColor(t);
      expect(r).toBeGreaterThanOrEqual(0);
      expect(r).toBeLessThanOrEqual(255);
      expect(g).toBeGreaterThanOrEqual(0);
      expect(g).toBeLessThanOrEqual(255);
      expect(b).toBeGreaterThanOrEqual(0);
      expect(b).toBeLessThanOrEqual(255);
    }
  });

  it('clamps out-of-range inputs', () => {
    expect(dbToColor(-1)).toEqual(dbToColor(0));
    expect(dbToColor(2)).toEqual(dbToColor(1));
  });
});
