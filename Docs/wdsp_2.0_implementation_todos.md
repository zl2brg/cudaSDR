# WDSP 2.0 — implementation to-dos

Roadmap derived from `wdsp-2.00/WDSP_Guide, Rev 2.00.pdf` and a gap audit of
cudaSDR’s current WDSP wiring. PureSignal 3.0 is deferred until a TX-feedback
path is first-class.

Check items off as they land. See also the Cursor plan
`wdsp_2.0_improvements` for fuller notes.

## Priority 1 — High value, moderate effort

- [ ] **Restore HB `rsmpin`** — Re-enable WDSP 2.0 half-band input resampler in
      `wdsp-2.00/RXA.c` (replace local polyphase patch). Keep dual-rate
      `input=pan, dsp=48k`. Verify local speakers + TCI web client audio.
      Update stale “HB caused TCI buzz” comments (buzz was the web client fade).

- [ ] **Wire CTCSS UI** — Connect existing TX settings CTCSS control to
      `SetTXACTCSSFreq` / `SetTXACTCSSRun` (`cusdr_transmitter.cpp` currently
      forces off).

- [ ] **Broadcast FM stereo (`RXA_WBFM`)** — Add `DSPMode::WBFM`, map to
      `RXA_WBFM`, expose de-emphasis (`SetRXAWBFMdmph`) and stereo indicator
      (`GetRXAWBFMStereoIndicator`). Prefer ~192 kHz DSP rate in WBFM.

- [ ] **RX graphical EQ (classic)** — Wire `SetRXAEQRun` / `SetRXAGrphEQ`
      (10-band), persist in settings. Keep create-time EQ taps modest (avoid
      16384 min-phase FFTW freeze).

## Priority 2 — Strong DSP upgrades

- [ ] **NR3 (RNNR) / NR4 (SBNR)** — Extend NoiseFilterWidget beyond Off/NR1/NR2;
      call `SetRXARNNR*` / `SetRXASBNR*`; handle RNNR model load if required;
      confirm SBNR/libspecbleach link.

- [ ] **Phase-rotator auto-cal** — Wire `SetTXAPHROTAutoMode`,
      `SetTXAPHROTAutoReset`, surface `GetTXAPHROTAsymmetry` readout.

- [ ] **TX EQ** — Enable the existing stub (`enable_tx_equalizer` is hard-coded
      0); UI + persist bands via `SetTXAGrphEQ` / `SetTXAEQRun`.

## Priority 3 — Full 2.0 surface

- [ ] **NURBS free-curve RX/TX EQ** — `SetRXAEQCurve` / `GetRXAEQDraw`,
      `SetTXAEQCurve` / `GetTXAEQDraw` + curve editor UI.

- [ ] **TX CFC (NURBS)** — `SetTXACFCOMPCompCurve` / `PeqCurve` + draw getters;
      big SSB intelligibility win after classic TX EQ.

- [ ] **EMNR post2 knobs** — Expose `SetRXAEMNRpost2*` only if NR2 users need
      trained/psychoacoustic options beyond current UI.

## Deferred

- [ ] **PureSignal 3.0** — Needs feedback stream + PS UI + expanded `GetPSDisp`
      AmpView. Do not use removed PS 2.x APIs.

## Housekeeping

- [ ] Fix stale HB / dual-rate comments in `cusdr_sliceProcessor.cpp` and RXA.
- [ ] Update docs that still say `wdsp-1.29` (e.g. `Docs/codebase_architecture.md`).
- [ ] Keep wisdom-before-`OpenChannel` and reduced create-time EQ defaults when
      adding more EQ UI.
