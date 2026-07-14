import type { TciMessage } from './types';

export function parseTciMessages(raw: string): TciMessage[] {
  const messages: TciMessage[] = [];
  for (const part of raw.split(';')) {
    const trimmed = part.trim();
    if (!trimmed) continue;
    const colon = trimmed.indexOf(':');
    if (colon < 0) {
      messages.push({ name: trimmed.toUpperCase(), args: [] });
      continue;
    }
    const name = trimmed.slice(0, colon).trim().toUpperCase();
    const args = trimmed.slice(colon + 1).split(',');
    messages.push({ name, args });
  }
  return messages;
}

export function formatTciCommand(name: string, args: (string | number)[] = []): string {
  if (args.length === 0) return `${name};`;
  return `${name}:${args.join(',')};`;
}

export type StreamFrame = {
  receiver: number;
  sampleRate: number;
  format: number;
  length: number;
  type: number;
  channels: number;
  payload: Float32Array;
};

export const STREAM_HEADER_BYTES = 64;
export const STREAM_IQ = 0;
export const STREAM_RX_AUDIO = 1;
export const STREAM_TX_AUDIO = 2;

export const STREAM_FORMAT_INT16 = 0;
export const STREAM_FORMAT_FLOAT32 = 3;

/**
 * Build a binary stream frame (64-byte little-endian header + float32 payload)
 * matching the cudaSDR TCI server layout used by sendAudioPacket/sendIqPacket:
 *   0=receiver 4=sampleRate 8=format 20=length 24=streamType 28=channels.
 * Used for the client→server TX (mic) audio stream (type 2, mono float32).
 */
export function buildTxAudioFrame(
  samples: Float32Array,
  sampleRate: number,
  receiver = 0,
): ArrayBuffer {
  const length = samples.length;
  const buf = new ArrayBuffer(STREAM_HEADER_BYTES + length * 4);
  const dv = new DataView(buf);
  dv.setUint32(0, receiver >>> 0, true);
  dv.setUint32(4, sampleRate >>> 0, true);
  dv.setUint32(8, STREAM_FORMAT_FLOAT32, true);
  dv.setUint32(12, 0, true);
  dv.setUint32(16, 0, true);
  dv.setUint32(20, length >>> 0, true);
  dv.setUint32(24, STREAM_TX_AUDIO, true);
  dv.setUint32(28, 1, true);
  new Float32Array(buf, STREAM_HEADER_BYTES, length).set(samples);
  return buf;
}

export function parseStreamFrame(buf: ArrayBuffer): StreamFrame | null {
  if (buf.byteLength <= STREAM_HEADER_BYTES) return null;
  const dv = new DataView(buf);
  const length = dv.getUint32(20, true);
  const payloadBytes = buf.byteLength - STREAM_HEADER_BYTES;
  const floatCount = Math.floor(payloadBytes / 4);
  if (payloadBytes < 1) return null;

  return {
    receiver: dv.getUint32(0, true),
    sampleRate: dv.getUint32(4, true),
    format: dv.getUint32(8, true),
    length,
    type: dv.getUint32(24, true),
    channels: dv.getUint32(28, true),
    payload: new Float32Array(buf, STREAM_HEADER_BYTES, Math.max(1, floatCount)),
  };
}
