export type TciMessage = {
  name: string;
  args: string[];
};

export type RadioState = {
  connected: boolean;
  device: string;
  vfoHz: number;
  ddsHz: number;
  ifHz: number;
  modulation: string;
  filterLo: number;
  filterHi: number;
  smeterDbm: number;
  trx: boolean;
  tune: boolean;
  drive: number;
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
  ddsHz: 14_225_000,
  ifHz: 0,
  modulation: 'USB',
  filterLo: 300,
  filterHi: 2_700,
  smeterDbm: -140,
  trx: false,
  tune: false,
  drive: 0,
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
