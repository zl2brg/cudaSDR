/**
 * @file  panadapter_overlay_freq_tests.cpp
 * @brief Regression tests for panadapter VFO/LO overlay offset after A/B switches.
 */

#include <QtTest/QtTest>

#include "GL/PanadapterOverlayFreq.h"

using PanadapterOverlayFreq::State;
using PanadapterOverlayFreq::applyVfo;
using PanadapterOverlayFreq::applyCenter;

class PanadapterOverlayFreqTests : public QObject {
    Q_OBJECT

private slots:
    void inSpanAbSwitchKeepsCenterAndOffset();
    void justOffSpanAbSwitchZerosDelta();
    void farBandAbSwitchZerosDelta();
};

void PanadapterOverlayFreqTests::inSpanAbSwitchKeepsCenterAndOffset()
{
    const qint64 sampleRate = 192000;
    const qint64 center = 14100000;
    const qint64 vfoB = center + sampleRate / 4;

    State s{ center, center, sampleRate };
    applyVfo(s, vfoB);

    QCOMPARE(s.centerHz, center);
    QCOMPARE(s.vfoHz, vfoB);
    QCOMPARE(s.deltaFrequency(), center - vfoB);
    QCOMPARE(s.deltaF(), qreal(center - vfoB) / qreal(sampleRate));
}

void PanadapterOverlayFreqTests::justOffSpanAbSwitchZerosDelta()
{
    // VFO B is just beyond the current span. The old panel path clamped the
    // cursor to the span edge; after the LO jumped to B that edge still sat
    // inside the new window, so recovery skipped and deltaF stayed non-zero.
    const qint64 sampleRate = 192000;
    const qint64 halfSpan = sampleRate / 2;
    const qint64 centerA = 7100000;
    const qint64 vfoB = centerA + halfSpan + 4000;

    State s{ centerA, centerA, sampleRate };
    applyVfo(s, vfoB);
    applyCenter(s, vfoB, vfoB);

    QCOMPARE(s.centerHz, vfoB);
    QCOMPARE(s.vfoHz, vfoB);
    QCOMPARE(s.deltaFrequency(), 0);
    QCOMPARE(s.deltaF(), 0.0);
}

void PanadapterOverlayFreqTests::farBandAbSwitchZerosDelta()
{
    const qint64 sampleRate = 192000;
    const qint64 centerA = 14100000;
    const qint64 vfoB = 7074000;

    State s{ centerA, centerA, sampleRate };
    applyVfo(s, vfoB);
    applyCenter(s, vfoB, vfoB);

    QCOMPARE(s.centerHz, vfoB);
    QCOMPARE(s.vfoHz, vfoB);
    QCOMPARE(s.deltaFrequency(), 0);
    QCOMPARE(s.deltaF(), 0.0);
}

QTEST_APPLESS_MAIN(PanadapterOverlayFreqTests)
#include "panadapter_overlay_freq_tests.moc"
