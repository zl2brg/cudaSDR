# WDSP 2.0 — implementation to-dos

Roadmap derived from `wdsp-2.00/WDSP_Guide, Rev 2.00.pdf` and a gap audit of
cudaSDR’s current WDSP wiring. PureSignal 3.0 is deferred until a TX-feedback
path is first-class.

Check items off as they land. See also the Cursor plan
`wdsp_2.0_improvements` for fuller notes.

## Priority 1 — High value, moderate effort

- [x] **Restore HB `rsmpin`** — Re-enable WDSP 2.0 half-band input resampler in
      `wdsp-2.00/RXA.c` (replace local polyphase patch). Keep dual-rate
      `input=pan, dsp=48k`. Verify local speakers + TCI web client audio.
      Update stale “HB caused TCI buzz” comments (buzz was the web client fade).

- [x] **Wire CTCSS UI** — Connect existing TX settings CTCSS control to
      `SetTXACTCSSFreq` / `SetTXACTCSSRun` (`cusdr_transmitter.cpp` currently
      forces off).

- [x] ~~**Broadcast FM stereo (`RXA_WBFM`)**~~ — Removed from the app (not
      essential). Stock WDSP `wbfm.c` remains in the tree unused by UI.

- [x] **RX graphical EQ (classic)** — Wire `SetRXAEQRun` / `SetRXAGrphEQ`
      (10-band), persist in settings. Keep create-time EQ taps modest (avoid
      16384 min-phase FFTW freeze).

## Priority 2 — Strong DSP upgrades

- [x] **NR3 (RNNR) / NR4 (SBNR)** — NoiseFilterWidget Off/NR1–NR4; exclusive
      `SetRXAANR`/`EMNR`/`RNNR`/`SBNR` runs. Built-in rnnoise model (no custom
      load). libspecbleach already linked into `wdsp`.

- [x] **Phase-rotator auto-cal** — `SetTXAPHROTAutoMode` / `AutoReset` +
      asymmetry/fc readout in TX settings while auto-cal runs.

- [x] **TX EQ** — Settings-backed 10-band (`SetTXAGrphEQ10` / `SetTXAEQRun`)
      with TX settings UI (mirrors RX EQ).

## Priority 3 — Full 2.0 surface

- [x] **NURBS free-curve RX/TX EQ** — `SetRXAEQCurve` / `GetRXAEQDraw`,
      `SetTXAEQCurve` / `GetTXAEQDraw` + curve degree + draw plot UI
      (profile sliders + NURBS deg; not freehand).

- [x] **TX CFC (NURBS)** — `SetTXACFCOMPCompCurve` / `PeqCurve` + draw getters;
      CFC/Post-EQ levels, preamps, NURBS deg in TX settings.

- [x] **EMNR post2 knobs** — `SetRXAEMNRpost2*` under Noise Filter NR2 Post2
      (factor/nlevel/taper % + rate).

## Deferred

- [ ] **PureSignal 3.0** — Needs feedback stream + PS UI + expanded `GetPSDisp`
      AmpView. Do not use removed PS 2.x APIs.

## Housekeeping

- [ ] Fix stale HB / dual-rate comments in `cusdr_sliceProcessor.cpp` and RXA.
- [ ] Update docs that still say `wdsp-1.29` (e.g. `Docs/codebase_architecture.md`).
- [ ] Keep wisdom-before-`OpenChannel` and reduced create-time EQ defaults when
      adding more EQ UI.
