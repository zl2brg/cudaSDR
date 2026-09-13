#ifndef PANADAPTER_OVERLAY_FREQ_H
#define PANADAPTER_OVERLAY_FREQ_H

#include <QtGlobal>

/**
 * Panel-local VFO / LO state used to place the blue cursor and RX filter.
 *
 * A VFO A/B switch emits the new dial first, then (when off-span) recenters
 * the LO. The overlay must store the true dial and, after the centre moves,
 * always adopt the live SliceModel frequency — never a value previously
 * clamped to the span the LO just left.
 */
namespace PanadapterOverlayFreq {

struct State {
    qint64 centerHz = 0;
    qint64 vfoHz = 0;
    qint64 sampleRateHz = 1;

    qint64 deltaFrequency() const { return centerHz - vfoHz; }

    qreal deltaF() const
    {
        return sampleRateHz > 0
            ? qreal(centerHz - vfoHz) / qreal(sampleRateHz)
            : 0.0;
    }
};

// Store the true dial frequency. Do not clamp to the current span.
inline void applyVfo(State &s, qint64 freqHz)
{
    s.vfoHz = freqHz;
}

// After the LO moves, always adopt the live dial (SliceModel frequency).
inline void applyCenter(State &s, qint64 newCenterHz, qint64 modelVfoHz)
{
    s.centerHz = newCenterHz;
    s.vfoHz = modelVfoHz;
}

} // namespace PanadapterOverlayFreq

#endif
