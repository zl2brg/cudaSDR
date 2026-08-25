#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <QVector>
#include <QtGlobal>
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

/** Inputs for FFT → panadapter / waterfall pixel binning (max-hold downscale). */
struct PanBinParams {
    int spectrumSize = 0;
    int panPixelCount = 0;
    qreal fftMult = 1.0;
    qreal freqScaleZoomFactor = 1.0;
    qreal dBmPanMin = -140.0;
    qreal dBmPanLogGain = 0.0;
    bool mercuryAttenuator = false;
    bool peakHold = false;
};

struct PanBinResult {
    QVector<qreal> panBins;
    QVector<float> waterfallPixels;
    QVector<float> peakHoldBins;
    int sampleSize = 0;
    int panSpectrumBinsLength = 0;
    qreal panScale = 1.0;
    qreal scaleMult = 1.0;
};

/**
 * Max-hold bin FFT spectrum into panadapter display bins and one waterfall row.
 * Pure CPU work — safe to run off the GUI thread.
 *
 * \a waterfallBuffer may alias \a buffer. \a peakHoldIn is used only when
 * params.peakHold is true.
 */
inline PanBinResult binPanadapterSpectrum(const QVector<float>& buffer,
                                          const QVector<float>& waterfallBuffer,
                                          const PanBinParams& params,
                                          const QVector<float>& peakHoldIn = {})
{
    PanBinResult out;
    if (params.spectrumSize <= 0 || params.panPixelCount <= 0)
        return out;
    if (buffer.size() < params.spectrumSize || waterfallBuffer.size() < params.spectrumSize)
        return out;

    out.sampleSize = static_cast<int>(std::floor(params.fftMult * params.spectrumSize
                                                 * params.freqScaleZoomFactor));
    if (out.sampleSize <= 0)
        return out;

    const int deltaSampleSize = params.spectrumSize - out.sampleSize;
    out.panScale = 1.0 * out.sampleSize / params.panPixelCount;

    if (out.panScale < 0.125)
        out.scaleMult = 0.0625;
    else if (out.panScale < 0.25)
        out.scaleMult = 0.125;
    else if (out.panScale < 0.5)
        out.scaleMult = 0.25;
    else if (out.panScale < 1.0)
        out.scaleMult = 0.5;
    else
        out.scaleMult = 1.0;

    const int panPixelCount = qBound(0, params.panPixelCount, 16384);
    out.panSpectrumBinsLength = qBound(0,
                                       static_cast<int>(out.scaleMult * params.panPixelCount),
                                       panPixelCount);
    if (out.panSpectrumBinsLength <= 0)
        return out;

    out.panBins.reserve(out.panSpectrumBinsLength);
    // Quiet floor for any column not written (must sit at/below typical lowerThreshold,
    // not 0.0 which maps near white in the waterfall shader).
    out.waterfallPixels.resize(panPixelCount);
    out.waterfallPixels.fill(-200.0f);

    if (params.peakHold) {
        out.peakHoldBins = peakHoldIn;
        if (out.peakHoldBins.size() != out.panSpectrumBinsLength) {
            out.peakHoldBins.resize(out.panSpectrumBinsLength);
            out.peakHoldBins.fill(-500.0f);
        }
    }

    for (int i = 0; i < out.panSpectrumBinsLength; ++i) {
        int lIdx = static_cast<int>(std::floor(i * out.panScale / out.scaleMult));
        int rIdx = static_cast<int>(std::floor(i * out.panScale / out.scaleMult
                                               + out.panScale / out.scaleMult));
        if (rIdx <= lIdx)
            rIdx = lIdx + 1;

        lIdx = qBound(0, lIdx, buffer.size() - 1);
        rIdx = qBound(lIdx + 1, rIdx, buffer.size());

        int idx = lIdx;
        qreal localMax = buffer.at(lIdx);
        for (int j = lIdx + 1; j < rIdx; ++j) {
            if (buffer.at(j) > localMax) {
                localMax = buffer.at(j);
                idx = j;
            }
        }

        idx = qBound(0, idx + deltaSampleSize / 2, buffer.size() - 1);

        float waterfallDbm;
        if (params.mercuryAttenuator) {
            out.panBins << buffer.at(idx) - params.dBmPanMin - params.dBmPanLogGain - 20.0;
            waterfallDbm = static_cast<float>(waterfallBuffer.at(idx) - params.dBmPanLogGain - 20.0);
        } else {
            out.panBins << buffer.at(idx) - params.dBmPanMin - params.dBmPanLogGain;
            waterfallDbm = static_cast<float>(waterfallBuffer.at(idx) - params.dBmPanLogGain);
        }

        if (params.peakHold && i < out.peakHoldBins.size()
            && out.panBins.at(i) > out.peakHoldBins.at(i)) {
            out.peakHoldBins[i] = static_cast<float>(out.panBins.at(i));
        }

        // Expand pan bins across waterfall columns; ceil so the last bins cover the edge.
        const int x0 = qBound(0, static_cast<int>(std::floor(i / out.scaleMult)), panPixelCount - 1);
        const int x1 = qBound(x0 + 1,
                             static_cast<int>(std::ceil((i + 1) / out.scaleMult)),
                             panPixelCount);
        for (int x = x0; x < x1; ++x)
            out.waterfallPixels[x] = waterfallDbm;
    }

    return out;
}

} // namespace DisplayUtils

#endif // DISPLAY_UTILS_H
