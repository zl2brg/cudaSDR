export type TciMessage = {
  name: string;
  args: string[];
};

export type RadioState = {
  connected: boolean;
  device: string;
  /** VFO-A (RX dial) — TCI channel 0 */
  vfoHz: number;
  /** VFO-B (TX route) — TCI channel 1 */
  vfoBHz: number;
  /** Which memory the radio's dial is on (cudaSDR ACTIVE_VFO extension). */
  activeVfo: 'a' | 'b';
  /** ExpertSDR split_enable — TX uses VFO-B when true */
  splitEnabled: boolean;
  /** From CHANNELS_COUNT in the init burst (expect 2 for VFO-A/B). */
  channelsCount: number;
  ddsHz: number;
  ifHz: number;
  modulation: string;
  filterLo: number;
  filterHi: number;
  smeterDbm: number;
  trx: boolean;
  tune: boolean;
  drive: number;
  txPowerWatts: number;
  swr: number;
  iqSampleRate: number;
  audioSampleRate: number;
  rxAudioOn: boolean;
  volume: number;
  muted: boolean;
  fftReady: boolean;
  specAvg: number;
  specAvgMode: number;
  specAvgDomain: 'log' | 'linear';
};

export const defaultRadioState = (): RadioState => ({
  connected: false,
  device: '',
  vfoHz: 14_225_000,
  vfoBHz: 14_225_000,
  activeVfo: 'a',
  splitEnabled: false,
  channelsCount: 2,
  ddsHz: 14_225_000,
  ifHz: 0,
  modulation: 'USB',
  filterLo: 300,
  filterHi: 2_700,
  smeterDbm: -140,
  trx: false,
  tune: false,
  drive: 0,
  txPowerWatts: 0,
  swr: 1,
  iqSampleRate: 192_000,
  audioSampleRate: 48_000,
  rxAudioOn: false,
  volume: 56,
  muted: false,
  fftReady: false,
  specAvg: 4,
  specAvgMode: 1,
  specAvgDomain: 'log',
});

/** Spectrum averaging methods (index = mode value stored in RadioState.specAvgMode). */
export const SPEC_AVG_MODES = ['None', 'Box-car', 'Exponential'] as const;

export const MODULATIONS = [
  'LSB', 'USB', 'AM', 'FM', 'NFM', 'DIGU', 'DIGL', 'CW', 'CWL', 'SAM', 'FDV',
] as const;

export const FFT_SIZE = 4096;

export const DB_MIN = -140;
export const DB_MAX = -30;
