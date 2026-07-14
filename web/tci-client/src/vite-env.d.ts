/// <reference types="svelte" />
/// <reference types="vite/client" />

declare module 'fft.js' {
  export default class FFT {
    constructor(size: number);
    createComplexArray(): Float64Array;
    transform(out: Float64Array, input: Float64Array): void;
  }
}
