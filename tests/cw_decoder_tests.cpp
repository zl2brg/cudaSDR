#include <QtTest/QtTest>
#include <QSignalSpy>
#include <cmath>

#include "AudioEngine/CwDecoder.h"

class CwDecoderTests : public QObject {
    Q_OBJECT

private:
    // Helper to generate synthetic audio samples for a Morse sequence
    QVector<float> generateMorseTone(const QString &morse, int wpm, int pitchHz = 700, int sampleRate = 48000);

private slots:
    void testSingleCharacterDecoding();
    void testWordDecoding();
    void testCallsignDecoding();
    void testProsignDecoding();
    void testSpeedAdaptation();
    void testPitchTuning();
    void testAutoPitchTracking();
    void testResetAndClear();
    void testBayesianTimingJitter();
    void testBayesianHamExchange();
};

QVector<float> CwDecoderTests::generateMorseTone(const QString &morse, int wpm, int pitchHz, int sampleRate)
{
    const float ditSec = 1.2f / static_cast<float>(wpm);
    const int ditSamples = static_cast<int>(ditSec * static_cast<float>(sampleRate));
    const int dahSamples = ditSamples * 3;
    const int elemSpaceSamples = ditSamples;
    const int charSpaceSamples = ditSamples * 3;
    const int wordSpaceSamples = ditSamples * 7;

    QVector<float> audio;
    float phase = 0.0f;
    const float phaseStep = (2.0f * static_cast<float>(M_PI) * static_cast<float>(pitchHz)) / static_cast<float>(sampleRate);

    auto appendTone = [&](int sampleCount) {
        for (int i = 0; i < sampleCount; ++i) {
            // Apply 5ms cosine ramp to prevent click harmonics
            float env = 1.0f;
            const int rampSamples = sampleRate / 200; // 5ms
            if (i < rampSamples)
                env = 0.5f * (1.0f - std::cos(static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(rampSamples)));
            else if (i > sampleCount - rampSamples)
                env = 0.5f * (1.0f - std::cos(static_cast<float>(M_PI) * static_cast<float>(sampleCount - i) / static_cast<float>(rampSamples)));

            audio.append(0.5f * env * std::sin(phase));
            phase += phaseStep;
            if (phase >= 2.0f * static_cast<float>(M_PI))
                phase -= 2.0f * static_cast<float>(M_PI);
        }
    };

    auto appendSilence = [&](int sampleCount) {
        audio.append(QVector<float>(sampleCount, 0.0f));
    };

    // Lead-in silence
    appendSilence(ditSamples * 2);

    for (int i = 0; i < morse.length(); ++i) {
        const QChar c = morse.at(i);
        if (c == QLatin1Char('.')) {
            appendTone(ditSamples);
            appendSilence(elemSpaceSamples);
        } else if (c == QLatin1Char('-')) {
            appendTone(dahSamples);
            appendSilence(elemSpaceSamples);
        } else if (c == QLatin1Char(' ')) {
            // Inter-character space (extra 2 dits, since 1 dit space was already added after element)
            appendSilence(charSpaceSamples - elemSpaceSamples);
        } else if (c == QLatin1Char('/')) {
            // Inter-word space (extra 6 dits)
            appendSilence(wordSpaceSamples - elemSpaceSamples);
        }
    }

    // Trailing silence to allow state machine to finalize character & word
    appendSilence(wordSpaceSamples * 2);

    return audio;
}

void CwDecoderTests::testSingleCharacterDecoding()
{
    CwDecoder decoder(0);
    decoder.setPitch(700);

    // Morse for 'A' is '.-'
    const QVector<float> audio = generateMorseTone(QStringLiteral(".-"), 20, 700);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("A"));
}

void CwDecoderTests::testWordDecoding()
{
    CwDecoder decoder(0);
    decoder.setPitch(700);

    // Morse for 'CQ' is '-.-. --.-'
    const QVector<float> audio = generateMorseTone(QStringLiteral("-.-. --.-"), 20, 700);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("CQ"));
}

void CwDecoderTests::testCallsignDecoding()
{
    CwDecoder decoder(0);
    decoder.setPitch(700);

    // Morse for '73' is '--... ...--'
    const QVector<float> audio = generateMorseTone(QStringLiteral("--... ...--"), 22, 700);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("73"));
}

void CwDecoderTests::testProsignDecoding()
{
    CwDecoder decoder(0);
    decoder.setPitch(700);

    // Prosign <SK> is '...-.-'
    const QVector<float> audio = generateMorseTone(QStringLiteral("...-.-"), 20, 700);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("<SK>"));
}

void CwDecoderTests::testSpeedAdaptation()
{
    CwDecoder decoder(0);
    decoder.setPitch(700);

    // Test at 30 WPM
    const QVector<float> audio = generateMorseTone(QStringLiteral("... --- ..."), 30, 700);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("SOS"));
    QVERIFY(decoder.wpm() >= 25 && decoder.wpm() <= 35);
}

void CwDecoderTests::testPitchTuning()
{
    CwDecoder decoder(0);
    decoder.setPitch(600); // Tuned to 600 Hz

    // Transmit at matching 600 Hz
    const QVector<float> audio = generateMorseTone(QStringLiteral(".-"), 20, 600);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("A"));
}

void CwDecoderTests::testAutoPitchTracking()
{
    CwDecoder decoder(0);
    decoder.setPitch(700); // Nominal pitch 700 Hz
    decoder.setAutoTrackEnabled(true);

    // Transmit off-pitch at 780 Hz (80 Hz off)
    const QVector<float> audio = generateMorseTone(QStringLiteral("-.-. --.-"), 20, 780);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    // Auto-tracker should have adapted towards 780 Hz and decoded "CQ"
    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("CQ"));
    QEXPECT_FAIL("", "CW auto pitch tracking still drifts on synthetic CQ", Continue);
    QVERIFY(decoder.trackedPitch() >= 750 && decoder.trackedPitch() <= 810);
}

void CwDecoderTests::testResetAndClear()
{
    CwDecoder decoder(0);
    decoder.setPitch(700);

    const QVector<float> audio = generateMorseTone(QStringLiteral(".-"), 20, 700);
    decoder.processAudio(audio.constData(), audio.size(), 48000);
    QVERIFY(!decoder.recentText().isEmpty());

    decoder.clearText();
    QVERIFY(decoder.recentText().isEmpty());

    decoder.reset();
    QCOMPARE(decoder.recentText(), QString());
}

void CwDecoderTests::testBayesianTimingJitter()
{
    // Test operator "swing" and timing jitter
    // Send "TEST" (- . ... -) where element durations vary by +/- 20%
    const int wpm = 20;
    const float ditSec = 1.2f / static_cast<float>(wpm);
    const int sampleRate = 48000;
    const int ditSamples = static_cast<int>(ditSec * static_cast<float>(sampleRate));
    const int dahSamples = ditSamples * 3;
    const int charSpaceSamples = ditSamples * 3;
    const int elemSpaceSamples = ditSamples;

    QVector<float> audio;
    float phase = 0.0f;
    const float phaseStep = (2.0f * static_cast<float>(M_PI) * 700.0f) / static_cast<float>(sampleRate);

    auto appendTone = [&](int sampleCount) {
        for (int i = 0; i < sampleCount; ++i) {
            float env = 1.0f;
            const int rampSamples = sampleRate / 200;
            if (i < rampSamples)
                env = 0.5f * (1.0f - std::cos(static_cast<float>(M_PI) * static_cast<float>(i) / static_cast<float>(rampSamples)));
            else if (i > sampleCount - rampSamples)
                env = 0.5f * (1.0f - std::cos(static_cast<float>(M_PI) * static_cast<float>(sampleCount - i) / static_cast<float>(rampSamples)));

            audio.append(0.5f * env * std::sin(phase));
            phase += phaseStep;
            if (phase >= 2.0f * static_cast<float>(M_PI))
                phase -= 2.0f * static_cast<float>(M_PI);
        }
    };

    auto appendSilence = [&](int sampleCount) {
        audio.append(QVector<float>(sampleCount, 0.0f));
    };

    appendSilence(ditSamples * 2);

    // Letter 'T' (-): Shortened dah (2.4x dit instead of 3.0x)
    appendTone(static_cast<int>(dahSamples * 0.80f));
    appendSilence(charSpaceSamples);

    // Letter 'E' (.): Lengthened dit (1.3x dit instead of 1.0x)
    appendTone(static_cast<int>(ditSamples * 1.30f));
    appendSilence(charSpaceSamples);

    // Letter 'S' (...): 3 dits with alternating jitter
    appendTone(static_cast<int>(ditSamples * 0.90f));
    appendSilence(elemSpaceSamples);
    appendTone(static_cast<int>(ditSamples * 1.25f));
    appendSilence(elemSpaceSamples);
    appendTone(static_cast<int>(ditSamples * 1.05f));
    appendSilence(charSpaceSamples);

    // Letter 'T' (-): Standard dah
    appendTone(dahSamples);
    appendSilence(charSpaceSamples * 2);

    CwDecoder decoder(0);
    decoder.setPitch(700);
    decoder.processAudio(audio.constData(), audio.size(), sampleRate);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("TEST"));
}

void CwDecoderTests::testBayesianHamExchange()
{
    // Test full contest exchange with priors: "CQ DE ZL2BRG 599"
    // Morse: "-.-. --.- / -.. . / --.. .-.. ..--- -... .-. --. / ..... ----. ----."
    const QString exchange = QStringLiteral("-.-. --.- / -.. . / --.. .-.. ..--- -... .-. --. / ..... ----. ----.");
    const QVector<float> audio = generateMorseTone(exchange, 22, 700);

    CwDecoder decoder(0);
    decoder.setPitch(700);
    decoder.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText().trimmed(), QStringLiteral("CQ DE ZL2BRG 599"));
}

QTEST_MAIN(CwDecoderTests)
#include "cw_decoder_tests.moc"
