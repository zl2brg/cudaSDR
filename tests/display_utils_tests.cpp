#include <QtTest/QtTest>

#include <algorithm>
#include <iterator>

#include "Util/display_utils.h"

using namespace DisplayUtils;

class DisplayUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void appliesFixedDbOffset();
    void emptySpectrumIsNoOp();
    void remapsTxSpectrumOntoWiderPan();
    void equalRatesSkipRemap();
    void binPanadapterRejectsEmptyInputs();
    void binPanadapterSelectsPeakInBin();
    void binPanadapterAppliesAttenuatorOffset();
};

void DisplayUtilsTests::appliesFixedDbOffset()
{
    QVector<float> spectrum { -80.0f, -40.0f, 0.0f };
    applyTxPanadapterDisplayOffset(spectrum);
    QCOMPARE(spectrum.at(0), -80.0f + kTxPanadapterDisplayDbOffset);
    QCOMPARE(spectrum.at(1), -40.0f + kTxPanadapterDisplayDbOffset);
    QCOMPARE(spectrum.at(2), 0.0f + kTxPanadapterDisplayDbOffset);
}

void DisplayUtilsTests::emptySpectrumIsNoOp()
{
    QVector<float> spectrum;
    applyTxPanadapterDisplayOffset(spectrum);
    QVERIFY(spectrum.isEmpty());
    prepareTxPanadapterSpectrum(spectrum, 192000);
    QVERIFY(spectrum.isEmpty());
}

void DisplayUtilsTests::remapsTxSpectrumOntoWiderPan()
{
    // 8-bin TX spectrum: energy only in the centre two bins (~±1/8 of ±txNyquist).
    QVector<float> spectrum(8, -100.0f);
    spectrum[3] = -10.0f;
    spectrum[4] = -10.0f;

    prepareTxPanadapterSpectrum(spectrum, 192000, 48000);

    // TX span is 1/4 of the pan → centre two of eight bins land near pan centre.
    QCOMPARE(spectrum.size(), 8);
    QVERIFY(spectrum.at(0) < -100.0f); // floor + offset
    QVERIFY(spectrum.at(7) < -100.0f);
    // Peak should sit near the middle quarter, not at the outer edges.
    const int peakIdx = static_cast<int>(
        std::distance(spectrum.cbegin(), std::max_element(spectrum.cbegin(), spectrum.cend())));
    QVERIFY(peakIdx >= 2 && peakIdx <= 5);
    QCOMPARE(spectrum.at(peakIdx), -10.0f + kTxPanadapterDisplayDbOffset);
}

void DisplayUtilsTests::equalRatesSkipRemap()
{
    QVector<float> spectrum { -80.0f, -10.0f, -80.0f };
    prepareTxPanadapterSpectrum(spectrum, 48000, 48000);
    QCOMPARE(spectrum.at(0), -80.0f + kTxPanadapterDisplayDbOffset);
    QCOMPARE(spectrum.at(1), -10.0f + kTxPanadapterDisplayDbOffset);
    QCOMPARE(spectrum.at(2), -80.0f + kTxPanadapterDisplayDbOffset);
}

void DisplayUtilsTests::binPanadapterRejectsEmptyInputs()
{
    PanBinParams params;
    params.spectrumSize = 8;
    params.panPixelCount = 4;
    QCOMPARE(binPanadapterSpectrum({}, {}, params).panBins.size(), 0);

    QVector<float> shortBuf(4, -90.0f);
    QCOMPARE(binPanadapterSpectrum(shortBuf, shortBuf, params).panBins.size(), 0);
}

void DisplayUtilsTests::binPanadapterSelectsPeakInBin()
{
    // Full-zoom (fftMult=1, zoom=1): sampleSize == spectrumSize → one FFT bin per
    // display region; max-hold must pick the loudest bin in each pan column.
    constexpr int N = 8;
    QVector<float> spectrum(N, -100.0f);
    spectrum[3] = -20.0f;

    PanBinParams params;
    params.spectrumSize = N;
    params.panPixelCount = 4;
    params.fftMult = 1.0;
    params.freqScaleZoomFactor = 1.0;
    params.dBmPanMin = -140.0;
    params.dBmPanLogGain = 0.0;

    const PanBinResult out = binPanadapterSpectrum(spectrum, spectrum, params);
    QVERIFY(out.panSpectrumBinsLength > 0);
    QVERIFY(!out.panBins.isEmpty());
    QVERIFY(!out.waterfallPixels.isEmpty());

    const qreal expected = -20.0 - params.dBmPanMin - params.dBmPanLogGain;
    const qreal peak = *std::max_element(out.panBins.cbegin(), out.panBins.cend());
    QCOMPARE(peak, expected);
}

void DisplayUtilsTests::binPanadapterAppliesAttenuatorOffset()
{
    constexpr int N = 8;
    QVector<float> spectrum(N, -80.0f);

    PanBinParams params;
    params.spectrumSize = N;
    params.panPixelCount = 4;
    params.fftMult = 1.0;
    params.freqScaleZoomFactor = 1.0;
    params.dBmPanMin = -140.0;
    params.dBmPanLogGain = 0.0;
    params.mercuryAttenuator = true;

    const PanBinResult withAtt = binPanadapterSpectrum(spectrum, spectrum, params);
    params.mercuryAttenuator = false;
    const PanBinResult withoutAtt = binPanadapterSpectrum(spectrum, spectrum, params);

    QVERIFY(!withAtt.panBins.isEmpty());
    QVERIFY(!withoutAtt.panBins.isEmpty());
    QCOMPARE(withAtt.panBins.at(0), withoutAtt.panBins.at(0) - 20.0);
}

QTEST_APPLESS_MAIN(DisplayUtilsTests)
#include "display_utils_tests.moc"
