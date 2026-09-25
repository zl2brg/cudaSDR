/**
 * @file spectral_painter_tests.cpp
 * @brief Unit tests for SpectralPainter SDR spectral callsign synthesis.
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <cmath>
#include <numeric>

#include "AudioEngine/SpectralPainter.h"

class SpectralPainterTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testDefaultsAndSetters();
    void testSynthesisOutputLengthAndRange();
    void testRaisedCosineTaper();
    void testFallbackCallsign();
    void testStreamingReadSamplesAndSignals();
    void testStopAbort();
    void testInvertFrequencyAndOrientation();
    void testPaintStyles();
    void testDirectIqSynthesisUSB();
    void testDirectIqSynthesisLSB();
    void testDirectIqStreamingRead();
};

void SpectralPainterTests::initTestCase() {
    qputenv("QT_QPA_PLATFORM", "offscreen");
}

void SpectralPainterTests::testDefaultsAndSetters() {
    SpectralPainter painter;
    QCOMPARE(painter.sampleRate(), 48000);
    QCOMPARE(painter.lowFrequencyHz(), 600.0f);
    QCOMPARE(painter.highFrequencyHz(), 2400.0f);
    QCOMPARE(painter.durationMs(), 2000);
    QVERIFY(!painter.isActive());
    QVERIFY(!painter.isAutoTail());

    painter.setFrequencyRange(500.0f, 2500.0f);
    QCOMPARE(painter.lowFrequencyHz(), 500.0f);
    QCOMPARE(painter.highFrequencyHz(), 2500.0f);

    painter.setDurationMs(1500);
    QCOMPARE(painter.durationMs(), 1500);

    painter.setCustomText(QStringLiteral("ZL2BRG"));
    QCOMPARE(painter.customText(), QStringLiteral("ZL2BRG"));
}

void SpectralPainterTests::testSynthesisOutputLengthAndRange() {
    SpectralPainter painter;
    painter.setDurationMs(1000); // 1.0 second = 48000 samples

    std::vector<float> samples = painter.synthesize(QStringLiteral("TEST"));
    QCOMPARE(static_cast<int>(samples.size()), 48000);

    float maxAbs = 0.0f;
    for (float s : samples) {
        float a = std::abs(s);
        if (a > maxAbs)
            maxAbs = a;
    }

    // Peak amplitude should be normalized near 0.70 (-3 dBFS)
    QVERIFY(maxAbs >= 0.65f && maxAbs <= 0.75f);
}

void SpectralPainterTests::testRaisedCosineTaper() {
    SpectralPainter painter;
    painter.setDurationMs(1000);
    std::vector<float> samples = painter.synthesize(QStringLiteral("W1AW"));
    QVERIFY(samples.size() > 1000);

    // First and last samples should be tapered down to 0
    QVERIFY(std::abs(samples.front()) < 0.01f);
    QVERIFY(std::abs(samples.back()) < 0.01f);
}

void SpectralPainterTests::testFallbackCallsign() {
    SpectralPainter painter;
    painter.setDurationMs(500);

    // Empty text with fallback
    std::vector<float> s1 = painter.synthesize(QString(), QStringLiteral("ZL2BRG"));
    QVERIFY(!s1.empty());

    // Empty text with empty fallback -> defaults to NOCALL
    std::vector<float> s2 = painter.synthesize(QString(), QString());
    QVERIFY(!s2.empty());
}

void SpectralPainterTests::testStreamingReadSamplesAndSignals() {
    SpectralPainter painter;
    painter.setDurationMs(500); // 0.5s = 24000 samples

    QSignalSpy spyStarted(&painter, &SpectralPainter::paintStarted);
    QSignalSpy spyFinished(&painter, &SpectralPainter::paintFinished);

    bool ok = painter.start(true /* autoTail */, QStringLiteral("HELLO"));
    QVERIFY(ok);
    QVERIFY(painter.isActive());
    QVERIFY(painter.isAutoTail());
    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyStarted.at(0).at(0).toBool(), true);

    std::vector<float> readBuf(1024);
    size_t totalRead = 0;

    while (painter.isActive()) {
        size_t n = painter.readSamples(readBuf.data(), readBuf.size());
        if (n == 0)
            break;
        totalRead += n;
    }

    QCOMPARE(static_cast<int>(totalRead), 24000);
    QVERIFY(!painter.isActive());
    QCOMPARE(spyFinished.count(), 1);
    QCOMPARE(spyFinished.at(0).at(0).toBool(), true);
}

void SpectralPainterTests::testStopAbort() {
    SpectralPainter painter;
    painter.setDurationMs(2000);

    QSignalSpy spyFinished(&painter, &SpectralPainter::paintFinished);

    painter.start(false, QStringLiteral("CANCEL_ME"));
    QVERIFY(painter.isActive());

    float buf[512];
    size_t n = painter.readSamples(buf, 512);
    QCOMPARE(n, static_cast<size_t>(512));

    painter.stop();
    QVERIFY(!painter.isActive());
    QCOMPARE(spyFinished.count(), 1);
    QCOMPARE(spyFinished.at(0).at(0).toBool(), false);
}

void SpectralPainterTests::testInvertFrequencyAndOrientation() {
    SpectralPainter painter;
    QVERIFY(!painter.invertFrequency());
    QVERIFY(!painter.invertTime());

    painter.setInvertFrequency(true);
    QVERIFY(painter.invertFrequency());

    painter.setInvertTime(true);
    QVERIFY(painter.invertTime());

    std::vector<float> sNorm = painter.synthesize(QStringLiteral("ZL2BRG"), QString(), false);
    std::vector<float> sInv = painter.synthesize(QStringLiteral("ZL2BRG"), QString(), true);
    QCOMPARE(sNorm.size(), sInv.size());
    QVERIFY(!sNorm.empty());
    QVERIFY(!sInv.empty());
}

void SpectralPainterTests::testPaintStyles() {
    SpectralPainter painter;
    QCOMPARE(painter.paintStyle(), SpectralPainter::PaintStyle::Banner);

    std::vector<float> bannerSamples = painter.synthesize(QStringLiteral("ZL2BRG/P"));
    QVERIFY(!bannerSamples.empty());

    painter.setPaintStyle(SpectralPainter::PaintStyle::Ticker);
    QCOMPARE(painter.paintStyle(), SpectralPainter::PaintStyle::Ticker);

    std::vector<float> tickerSamples = painter.synthesize(QStringLiteral("ZL2BRG/P"));
    QVERIFY(!tickerSamples.empty());
    QCOMPARE(tickerSamples.size(), bannerSamples.size());
}

void SpectralPainterTests::testDirectIqSynthesisUSB() {
    SpectralPainter painter;
    painter.setDurationMs(500); // 0.5s = 24000 samples
    painter.setFrequencyRange(600.0f, 2400.0f);

    std::vector<std::complex<float>> iq = painter.synthesizeIq(QStringLiteral("ZL2BRG"), QString(), false);
    QCOMPARE(static_cast<int>(iq.size()), 24000);

    size_t peakIdx = 0;
    float maxMag = 0.0f;
    for (size_t i = 0; i < iq.size(); ++i) {
        float m = std::abs(iq[i]);
        if (m > maxMag) {
            maxMag = m;
            peakIdx = i;
        }
    }
    // Normalized near 0.70 (-3 dBFS)
    QVERIFY(maxMag >= 0.65f && maxMag <= 0.75f);

    // Verify analytic USB signal: positive frequency power must vastly dominate negative frequency power.
    // Analyze active samples around peakIdx across all 80 passband frequencies.
    const double fs = painter.sampleRate();
    const size_t N = 4800; // 100 ms window
    const size_t startIdx = (peakIdx > N / 2) ? std::min(peakIdx - N / 2, iq.size() - N) : 0;
    constexpr int numBands = 80;

    double posPower = 0.0;
    double negPower = 0.0;

    for (int x = 0; x < numBands; ++x) {
        const double frac = static_cast<double>(x) / static_cast<double>(numBands - 1);
        const double f = 600.0 + frac * (2400.0 - 600.0);
        std::complex<double> X_pos(0.0, 0.0);
        std::complex<double> X_neg(0.0, 0.0);
        for (size_t n = 0; n < N; ++n) {
            const double t = static_cast<double>(n) / fs;
            const double phi_pos = -2.0 * M_PI * f * t;
            const double phi_neg = -2.0 * M_PI * (-f) * t;
            const std::complex<double> s(iq[startIdx + n].real(), iq[startIdx + n].imag());
            X_pos += s * std::complex<double>(std::cos(phi_pos), std::sin(phi_pos));
            X_neg += s * std::complex<double>(std::cos(phi_neg), std::sin(phi_neg));
        }
        posPower += std::norm(X_pos);
        negPower += std::norm(X_neg);
    }

    QVERIFY(posPower > 1.0);
    const double rejectionDb = 10.0 * std::log10(posPower / std::max(negPower, 1e-12));
    QVERIFY2(rejectionDb > 35.0, qPrintable(QString("USB rejection %1 dB < 35 dB").arg(rejectionDb)));
}

void SpectralPainterTests::testDirectIqSynthesisLSB() {
    SpectralPainter painter;
    painter.setDurationMs(500);
    painter.setFrequencyRange(600.0f, 2400.0f);

    // invertFrequency = true triggers LSB negative-baseband mapping (-2400 Hz .. -600 Hz)
    std::vector<std::complex<float>> iq = painter.synthesizeIq(QStringLiteral("ZL2BRG"), QString(), true);
    QCOMPARE(static_cast<int>(iq.size()), 24000);

    size_t peakIdx = 0;
    float maxMag = 0.0f;
    for (size_t i = 0; i < iq.size(); ++i) {
        float m = std::abs(iq[i]);
        if (m > maxMag) {
            maxMag = m;
            peakIdx = i;
        }
    }
    QVERIFY(maxMag >= 0.65f && maxMag <= 0.75f);

    const double fs = painter.sampleRate();
    const size_t N = 4800; // 100 ms window
    const size_t startIdx = (peakIdx > N / 2) ? std::min(peakIdx - N / 2, iq.size() - N) : 0;
    constexpr int numBands = 80;

    double posPower = 0.0;
    double negPower = 0.0;

    for (int x = 0; x < numBands; ++x) {
        const double frac = static_cast<double>(x) / static_cast<double>(numBands - 1);
        const double f = 600.0 + frac * (2400.0 - 600.0);
        std::complex<double> X_pos(0.0, 0.0);
        std::complex<double> X_neg(0.0, 0.0);
        for (size_t n = 0; n < N; ++n) {
            const double t = static_cast<double>(n) / fs;
            const double phi_pos = -2.0 * M_PI * f * t;
            const double phi_neg = -2.0 * M_PI * (-f) * t;
            const std::complex<double> s(iq[startIdx + n].real(), iq[startIdx + n].imag());
            X_pos += s * std::complex<double>(std::cos(phi_pos), std::sin(phi_pos));
            X_neg += s * std::complex<double>(std::cos(phi_neg), std::sin(phi_neg));
        }
        posPower += std::norm(X_pos);
        negPower += std::norm(X_neg);
    }

    QVERIFY(negPower > 1.0);
    const double rejectionDb = 10.0 * std::log10(negPower / std::max(posPower, 1e-12));
    QVERIFY2(rejectionDb > 35.0, qPrintable(QString("LSB rejection %1 dB < 35 dB").arg(rejectionDb)));
}

void SpectralPainterTests::testDirectIqStreamingRead() {
    SpectralPainter painter;
    painter.setDurationMs(500); // 24000 samples

    QSignalSpy spyStarted(&painter, &SpectralPainter::paintStarted);
    QSignalSpy spyFinished(&painter, &SpectralPainter::paintFinished);

    bool ok = painter.start(false, QStringLiteral("TEST"));
    QVERIFY(ok);
    QVERIFY(painter.isActive());
    QCOMPARE(spyStarted.count(), 1);

    // Read using cpx buffer
    std::vector<cpx> cpxBuf(1024);
    size_t totalRead = 0;
    while (painter.isActive()) {
        size_t n = painter.readIqSamples(cpxBuf.data(), cpxBuf.size());
        if (n == 0)
            break;
        totalRead += n;
    }

    QCOMPARE(static_cast<int>(totalRead), 24000);
    QVERIFY(!painter.isActive());
    QCOMPARE(spyFinished.count(), 1);

    // Also test readIqSamples(std::complex<float>*, count)
    ok = painter.start(false, QStringLiteral("TEST2"));
    QVERIFY(ok);
    std::vector<std::complex<float>> cFloatBuf(512);
    size_t nFloat = painter.readIqSamples(cFloatBuf.data(), 512);
    QCOMPARE(nFloat, static_cast<size_t>(512));
    painter.stop();
    QVERIFY(!painter.isActive());
}

QTEST_MAIN(SpectralPainterTests)
#include "spectral_painter_tests.moc"
