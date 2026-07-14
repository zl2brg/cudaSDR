import { describe, expect, it } from 'vitest';
import {
  STREAM_FORMAT_FLOAT32,
  STREAM_HEADER_BYTES,
  STREAM_IQ,
  STREAM_RX_AUDIO,
  STREAM_TX_AUDIO,
  buildTxAudioFrame,
  formatTciCommand,
  parseStreamFrame,
  parseTciMessages,
} from './StreamParser';

describe('parseTciMessages', () => {
  it('parses a single command without args', () => {
    expect(parseTciMessages('READY;')).toEqual([{ name: 'READY', args: [] }]);
  });

  it('parses multiple semicolon-separated commands', () => {
    expect(parseTciMessages('TRX:0,true;VFO:0,0,7050000;')).toEqual([
      { name: 'TRX', args: ['0', 'true'] },
      { name: 'VFO', args: ['0', '0', '7050000'] },
    ]);
  });

  it('uppercases command names and preserves arg casing', () => {
    expect(parseTciMessages('modulation:0,usb;')).toEqual([
      { name: 'MODULATION', args: ['0', 'usb'] },
    ]);
  });

  it('skips empty segments', () => {
    expect(parseTciMessages(';;READY;;')).toEqual([{ name: 'READY', args: [] }]);
  });
});

describe('formatTciCommand', () => {
  it('formats commands with and without args', () => {
    expect(formatTciCommand('START')).toBe('START;');
    expect(formatTciCommand('TRX', [0, true])).toBe('TRX:0,true;');
    expect(formatTciCommand('VFO', [0, 0, 7_050_000])).toBe('VFO:0,0,7050000;');
  });
});

describe('buildTxAudioFrame / parseStreamFrame', () => {
  it('round-trips a mono float32 TX audio frame', () => {
    const samples = new Float32Array([0.25, -0.5, 0.75]);
    const buf = buildTxAudioFrame(samples, 48_000, 0);
    const frame = parseStreamFrame(buf);

    expect(frame).not.toBeNull();
    expect(frame!.receiver).toBe(0);
    expect(frame!.sampleRate).toBe(48_000);
    expect(frame!.format).toBe(STREAM_FORMAT_FLOAT32);
    expect(frame!.length).toBe(3);
    expect(frame!.type).toBe(STREAM_TX_AUDIO);
    expect(frame!.channels).toBe(1);
    expect(frame!.payload[0]).toBeCloseTo(0.25);
    expect(frame!.payload[1]).toBeCloseTo(-0.5);
    expect(frame!.payload[2]).toBeCloseTo(0.75);
  });

  it('uses the cudaSDR 64-byte little-endian header layout', () => {
    const samples = new Float32Array([1.0]);
    const buf = buildTxAudioFrame(samples, 96_000, 2);
    const dv = new DataView(buf);

    expect(buf.byteLength).toBe(STREAM_HEADER_BYTES + 4);
    expect(dv.getUint32(0, true)).toBe(2);
    expect(dv.getUint32(4, true)).toBe(96_000);
    expect(dv.getUint32(8, true)).toBe(STREAM_FORMAT_FLOAT32);
    expect(dv.getUint32(20, true)).toBe(1);
    expect(dv.getUint32(24, true)).toBe(STREAM_TX_AUDIO);
    expect(dv.getUint32(28, true)).toBe(1);
  });

  it('returns null for undersized buffers', () => {
    expect(parseStreamFrame(new ArrayBuffer(STREAM_HEADER_BYTES))).toBeNull();
    expect(parseStreamFrame(new ArrayBuffer(0))).toBeNull();
  });
});

/** Golden header bytes shared with src/Util/tci_protocol_utils.h (type=2 TX audio). */
describe('client/server TX frame parity', () => {
  it('matches the C++ buildTxAudioFrame layout for a known sample', () => {
    const samples = new Float32Array([0.125, -0.25]);
    const buf = buildTxAudioFrame(samples, 48_000, 0);
    const bytes = new Uint8Array(buf);

    // Header field spot-checks (offsets from tci_protocol_utils.h).
    const dv = new DataView(buf);
    expect(dv.getUint32(24, true)).toBe(STREAM_TX_AUDIO);
    expect(dv.getUint32(20, true)).toBe(2);
    expect(bytes.length).toBe(STREAM_HEADER_BYTES + 8);

    const payload = new Float32Array(buf, STREAM_HEADER_BYTES, 2);
    expect(payload[0]).toBeCloseTo(0.125);
    expect(payload[1]).toBeCloseTo(-0.25);
  });

  it('distinguishes stream types by header offset 24', () => {
    const samples = new Float32Array([0]);
    const tx = buildTxAudioFrame(samples, 48_000);
    expect(parseStreamFrame(tx)!.type).toBe(STREAM_TX_AUDIO);
    expect(STREAM_IQ).toBe(0);
    expect(STREAM_RX_AUDIO).toBe(1);
    expect(STREAM_TX_AUDIO).toBe(2);
  });
});
