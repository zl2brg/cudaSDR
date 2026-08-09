#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <QVector>
#include <algorithm>
#include <cmath>

namespace DisplayUtils {

// WDSP TX analyzer levels run hot vs RX; offset panadapter display (dB).
constexpr float kTxPanadapterDisplayDbOffset = -26.0f;

// TX WDSP analyzer always runs at mic/IQ baseband rate (see Transmitter::create_transmitter).
constexpr int kTxAnalyzerSampleRate = 48000;

inline void applyTxPanadapterDisplayOffset(QVector<float> &spectrum)
{
    for (float &bin : spectrum)
        bin += kTxPanadapterDisplayDbOffset;
}

/**
 * Map TX analyzer pixels (full-span at txAnalyzerRate) onto the RX panadapter
 * frequency axis (panSampleRate), then apply the TX display dB offset.
 *
 * When the pan is wider than the TX analyzer (e.g. 192 kHz pan, 48 kHz TX),
 * stretching TX bins full-width makes a 2.9 kHz SSB passband look ~4× wider.
 * Place the TX spectrum in the centre fraction of the pan and fill the skirts
 * with the trace floor.
 */
inline void prepareTxPanadapterSpectrum(QVector<float> &spectrum,
                                        int panSampleRate,
                                        int txAnalyzerRate = kTxAnalyzerSampleRate)
{
    if (spectrum.isEmpty())
        return;

    if (panSampleRate > 0 && txAnalyzerRate > 0 && panSampleRate != txAnalyzerRate) {
        const int n = spectrum.size();
        const QVector<float> tx = spectrum;
        float floorDb = tx.first();
        for (float v : tx)
            floorDb = std::min(floorDb, v);

        spectrum.fill(floorDb);
        for (int i = 0; i < n; ++i) {
            const double freqHz =
                (static_cast<double>(i) / static_cast<double>(n) - 0.5) * panSampleRate;
            if (std::abs(freqHz) > txAnalyzerRate * 0.5)
                continue;
            const int src = static_cast<int>(std::llround(
                (freqHz / txAnalyzerRate + 0.5) * n));
            if (src >= 0 && src < n)
                spectrum[i] = tx.at(src);
        }
    }

    applyTxPanadapterDisplayOffset(spectrum);
}

} // namespace DisplayUtils

#endif // DISPLAY_UTILS_H
