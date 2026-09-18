#include <QtTest>
#include <cmath>
#include <vector>
#include "AudioEngine/RttyAutoClassifier.h"

class RttyAutoClassifierTests : public QObject {
    Q_OBJECT

private slots:
    void testInitialState();
    void testShiftDetection170Hz();
    void testShiftDetection850Hz();
    void testBaudRateDetection45Baud();
    void testBaudRateDetection100Baud();
    void testPolarityInversionDetection();
    void testNoiseImmunityNoFalseLock();
};

void RttyAutoClassifierTests::testInitialState()
{
    RttyAutoClassifier classifier(0);
    QCOMPARE(classifier.rxId(), 0);
    QVERIFY(classifier.isEnabled());
    QVERIFY(!classifier.isShiftLocked());
    QVERIFY(!classifier.isBaudLocked());
}

void RttyAutoClassifierTests::testShiftDetection170Hz()
{
    RttyAutoClassifier classifier(0);
    QSignalSpy shiftSpy(&classifier, &RttyAutoClassifier::shiftDetected);

    // Generate ~0.3s of 48 kHz audio containing 2125 Hz and 2295 Hz tones (170 Hz shift, center 2210 Hz)
    const int sampleRate = 48000;
    const int numSamples = 48000 * 3 / 10; // 0.3s
    std::vector<float> audio(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        // Toggling tones every 22 ms (representing RTTY symbols)
        const bool bit = ((i / (sampleRate * 22 / 1000)) % 2) == 0;
        const float freq = bit ? 2125.0f : 2295.0f;
        audio[i] = 0.6f * std::sin(2.0f * static_cast<float>(M_PI) * freq * t);
    }

    classifier.feedAudio(audio.data(), numSamples, sampleRate);

    QVERIFY(shiftSpy.count() >= 1);
    const QList<QVariant> args = shiftSpy.last();
    const float detectedShift = args.at(0).toFloat();
    const float detectedCenter = args.at(1).toFloat();

    QCOMPARE(detectedShift, 170.0f);
    QVERIFY(std::abs(detectedCenter - 2210.0f) < 25.0f);
}

void RttyAutoClassifierTests::testShiftDetection850Hz()
{
    RttyAutoClassifier classifier(0);
    QSignalSpy shiftSpy(&classifier, &RttyAutoClassifier::shiftDetected);

    // Generate ~0.3s of audio with 1500 Hz and 2350 Hz (850 Hz shift, center 1925 Hz)
    const int sampleRate = 48000;
    const int numSamples = 48000 * 3 / 10;
    std::vector<float> audio(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        const bool bit = ((i / (sampleRate * 20 / 1000)) % 2) == 0;
        const float freq = bit ? 1500.0f : 2350.0f;
        audio[i] = 0.6f * std::sin(2.0f * static_cast<float>(M_PI) * freq * t);
    }

    classifier.feedAudio(audio.data(), numSamples, sampleRate);

    QVERIFY(shiftSpy.count() >= 1);
    const QList<QVariant> args = shiftSpy.last();
    const float detectedShift = args.at(0).toFloat();
    const float detectedCenter = args.at(1).toFloat();

    QCOMPARE(detectedShift, 850.0f);
    QVERIFY(std::abs(detectedCenter - 1925.0f) < 30.0f);
}

void RttyAutoClassifierTests::testBaudRateDetection45Baud()
{
    RttyAutoClassifier classifier(0);
    QSignalSpy baudSpy(&classifier, &RttyAutoClassifier::baudRateDetected);

    // At 2000 Hz, 45.45 baud has symbol length ~44 samples.
    // Feed 30 transitions of typical 1, 2, or 3 symbols length (44, 88, 132 samples)
    const int intervals[] = {44, 44, 88, 44, 132, 44, 88, 44, 44, 88, 44, 132, 44, 88, 44, 44, 88, 44, 132, 44, 44, 88, 44};
    for (int intv : intervals) {
        classifier.feedTransition(intv, 2000.0f);
    }

    QVERIFY(baudSpy.count() >= 1);
    const float detectedBaud = baudSpy.last().at(0).toFloat();
    QCOMPARE(detectedBaud, 45.4545f);
}

void RttyAutoClassifierTests::testBaudRateDetection100Baud()
{
    RttyAutoClassifier classifier(0);
    QSignalSpy baudSpy(&classifier, &RttyAutoClassifier::baudRateDetected);

    // At 2000 Hz, 100 baud has symbol length 20 samples.
    // Feed transitions of 20, 40, 60 samples
    const int intervals[] = {20, 20, 40, 20, 60, 20, 40, 20, 20, 40, 20, 60, 20, 40, 20, 20, 40, 20, 60, 20, 20, 40, 20};
    for (int intv : intervals) {
        classifier.feedTransition(intv, 2000.0f);
    }

    QVERIFY(baudSpy.count() >= 1);
    const float detectedBaud = baudSpy.last().at(0).toFloat();
    QCOMPARE(detectedBaud, 100.0f);
}

void RttyAutoClassifierTests::testPolarityInversionDetection()
{
    RttyAutoClassifier classifier(0);
    QSignalSpy polaritySpy(&classifier, &RttyAutoClassifier::polarityInversionSuggested);

    // Feed normal positive stop bit LLRs -> no inversion suggested
    for (int i = 0; i < 6; ++i) {
        classifier.feedFramingResult(+3.5f, 0.95f);
    }
    QCOMPARE(polaritySpy.count(), 0);

    // Feed negative stop bit LLRs (indicating inverted polarity: Space decoded as Stop)
    for (int i = 0; i < 6; ++i) {
        classifier.feedFramingResult(-3.5f, 0.95f);
    }
    QCOMPARE(polaritySpy.count(), 1);
}

void RttyAutoClassifierTests::testNoiseImmunityNoFalseLock()
{
    RttyAutoClassifier classifier(0);
    QSignalSpy shiftSpy(&classifier, &RttyAutoClassifier::shiftDetected);

    // Feed white noise
    const int numSamples = 48000 * 3 / 10;
    std::vector<float> noise(numSamples);
    for (int i = 0; i < numSamples; ++i) {
        noise[i] = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5f) * 0.1f;
    }

    classifier.feedAudio(noise.data(), numSamples, 48000);

    QCOMPARE(shiftSpy.count(), 0);
    QVERIFY(!classifier.isShiftLocked());
}

QTEST_MAIN(RttyAutoClassifierTests)
#include "rtty_auto_classifier_tests.moc"
