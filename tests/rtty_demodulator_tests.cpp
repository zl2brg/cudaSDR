#include <QtTest/QtTest>
#include <QSignalSpy>
#include <cmath>

#include "AudioEngine/RttyDemodulator.h"
#include "AudioEngine/RttyAutoClassifier.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class RttyDemodulatorTests : public QObject {
    Q_OBJECT

private:
    // Helper to generate 48 kHz synthetic continuous-phase FSK audio from text
    QVector<float> generateRttyAudio(
        const QString &text,
        float baudRate = 45.4545f,
        float centerFreq = 2210.0f,
        float shiftHz = 170.0f,
        bool reversePolarity = false,
        float snrDb = 100.0f,
        int sampleRate = 48000,
        float stopBits = 2.0f);

private slots:
    void testCleanRySequence();
    void testTextAndCallsign();
    void testFigsShift();
    void testSoftSymbolMetrics();
    void testLowTones();
    void testReversePolarity();
    void testUsbModeUsesAmateurHighTones();
    void testInvertedTonesRequireReverse();
    void testRyWithOnePointFiveStopBits();
    void testWeather50BaudFigsPersist();
    void testBaudRateTolerance();
    void testAutoBaudDetects50Not45();
    void testWeatherShiftIgnoresAmateurAutoBaud();
    void testResetAndClear();
    void testTuningScopeCrossedEllipses();
};

QVector<float> RttyDemodulatorTests::generateRttyAudio(
    const QString &text,
    float baudRate,
    float centerFreq,
    float shiftHz,
    bool reversePolarity,
    float snrDb,
    int sampleRate,
    float stopBits)
{
    const float markFreq = reversePolarity ? (centerFreq + shiftHz * 0.5f) : (centerFreq - shiftHz * 0.5f);
    const float spaceFreq = reversePolarity ? (centerFreq - shiftHz * 0.5f) : (centerFreq + shiftHz * 0.5f);

    const int samplesPerBit = qRound(static_cast<float>(sampleRate) / baudRate);

    // Convert text to sequence of bits (1 = Mark, 0 = Space)
    // Baudot mapping
    static const struct { char c; quint8 code; bool figs; } CHAR_MAP[] = {
        { 'A', 0x03, false }, { 'B', 0x19, false }, { 'C', 0x0E, false }, { 'D', 0x09, false },
        { 'E', 0x01, false }, { 'F', 0x0D, false }, { 'G', 0x1A, false }, { 'H', 0x14, false },
        { 'I', 0x06, false }, { 'J', 0x0B, false }, { 'K', 0x0F, false }, { 'L', 0x12, false },
        { 'M', 0x1C, false }, { 'N', 0x0C, false }, { 'O', 0x18, false }, { 'P', 0x16, false },
        { 'Q', 0x17, false }, { 'R', 0x0A, false }, { 'S', 0x05, false }, { 'T', 0x10, false },
        { 'U', 0x07, false }, { 'V', 0x1E, false }, { 'W', 0x13, false }, { 'X', 0x1D, false },
        { 'Y', 0x15, false }, { 'Z', 0x11, false }, { ' ', 0x04, false }, { '\n', 0x02, false },
        { '\r', 0x08, false },
        // Figures
        { '0', 0x16, true },  { '1', 0x17, true },  { '2', 0x13, true },  { '3', 0x01, true },
        { '4', 0x0A, true },  { '5', 0x10, true },  { '6', 0x15, true },  { '7', 0x07, true },
        { '8', 0x06, true },  { '9', 0x18, true },  { '-', 0x03, true },  { '?', 0x19, true },
        { ':', 0x0E, true },  { '(', 0x0F, true },  { ')', 0x12, true },  { '.', 0x1C, true },
        { ',', 0x0C, true },  { '/', 0x1D, true }
    };

    QVector<quint8> codes;
    bool curFigs = false;

    // Start with LTRS shift
    codes.append(0x1F);

    for (const QChar qc : text) {
        const char c = qc.toUpper().toLatin1();
        bool found = false;
        for (const auto &entry : CHAR_MAP) {
            if (entry.c == c) {
                if (entry.figs != curFigs) {
                    codes.append(entry.figs ? 0x1B : 0x1F); // FIGS or LTRS code
                    curFigs = entry.figs;
                }
                codes.append(entry.code);
                found = true;
                break;
            }
        }
        if (!found && c == ' ') {
            codes.append(0x04);
        }
    }

    // Build bit stream: 1=Mark, 0=Space
    QVector<int> bits;

    // Idle preamble: 8 Mark bits
    for (int i = 0; i < 8; ++i) {
        bits.append(1);
    }

    for (const quint8 code : codes) {
        // Start bit: Space (0)
        bits.append(0);

        // 5 data bits (LSB first)
        for (int b = 0; b < 5; ++b) {
            bits.append((code >> b) & 1);
        }

        // Stop duration is synthesized below (1.0 / 1.5 / 2.0 mark bits)
        bits.append(-1);
    }

    // Trailing idle: 8 Mark bits
    for (int i = 0; i < 8; ++i) {
        bits.append(1);
    }

    // Synthesize continuous-phase FSK audio
    QVector<float> audio;
    const int stopSamples = qMax(1, qRound(static_cast<float>(samplesPerBit) * stopBits));
    audio.reserve(bits.size() * samplesPerBit);

    float phase = 0.0f;
    const float twoPi = 2.0f * static_cast<float>(M_PI);

    auto emitTone = [&](float toneFreq, int count) {
        const float phaseStep = twoPi * toneFreq / static_cast<float>(sampleRate);
        for (int s = 0; s < count; ++s) {
            audio.append(0.5f * std::sin(phase));
            phase += phaseStep;
            if (phase >= twoPi) phase -= twoPi;
        }
    };

    for (const int bit : bits) {
        if (bit < 0) {
            emitTone(markFreq, stopSamples);
            continue;
        }
        emitTone((bit == 1) ? markFreq : spaceFreq, samplesPerBit);
    }

    // Optional AWGN noise injection
    if (snrDb < 50.0f) {
        const float sigPower = 0.5f * 0.5f * 0.5f; // 0.125
        const float noisePower = sigPower / std::pow(10.0f, snrDb / 10.0f);
        const float noiseStdDev = std::sqrt(noisePower);

        for (float &val : audio) {
            // Box-Muller transform
            const float u1 = qMax(1e-6f, static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
            const float u2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            const float n = std::sqrt(-2.0f * std::log(u1)) * std::cos(twoPi * u2) * noiseStdDev;
            val += n;
        }
    }

    return audio;
}

void RttyDemodulatorTests::testCleanRySequence() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);
    demod.setBaudRate(45.4545f);

    const QString testStr = "RYRYRY";
    const QVector<float> audio = generateRttyAudio(testStr);

    // Feed in chunks of 512 samples to simulate real audio callbacks
    const int chunkSize = 512;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("RYRYRY"),
             qPrintable(QString("Expected 'RYRYRY' in decoded text, got: '%1'").arg(demod.recentText())));
}

void RttyDemodulatorTests::testTextAndCallsign() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);

    const QString testStr = "CQ CQ DE ZL2BRG K";
    const QVector<float> audio = generateRttyAudio(testStr);

    const int chunkSize = 480;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("CQ CQ DE ZL2BRG K"),
             qPrintable(QString("Expected 'CQ CQ DE ZL2BRG K', got: '%1'").arg(demod.recentText())));
}

void RttyDemodulatorTests::testFigsShift() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);

    const QString testStr = "TEST 12345 HELLO";
    const QVector<float> audio = generateRttyAudio(testStr);

    const int chunkSize = 512;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("TEST 12345 HELLO"),
             qPrintable(QString("Expected 'TEST 12345 HELLO', got: '%1'").arg(demod.recentText())));
}

void RttyDemodulatorTests::testSoftSymbolMetrics() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);

    QSignalSpy spySymbol(&demod, &RttyDemodulator::symbolSampled);

    const QString testStr = "R";
    const QVector<float> audio = generateRttyAudio(testStr);

    demod.processAudio(audio.constData(), audio.size(), 48000);

    QVERIFY(spySymbol.count() > 5);

    // Verify soft symbols contain valid LLR values
    bool hasPositiveLLR = false;
    bool hasNegativeLLR = false;

    for (int i = 0; i < spySymbol.count(); ++i) {
        const RttySoftSymbol sym = spySymbol.at(i).at(0).value<RttySoftSymbol>();
        if (sym.llr > 0.0f) hasPositiveLLR = true;
        if (sym.llr < 0.0f) hasNegativeLLR = true;
        QVERIFY(sym.markMag >= 0.0f);
        QVERIFY(sym.spaceMag >= 0.0f);
    }

    QVERIFY(hasPositiveLLR);
    QVERIFY(hasNegativeLLR);
}

void RttyDemodulatorTests::testLowTones() {
    RttyDemodulator demod;
    // European low tones: Center = 1360 Hz (Mark 1275, Space 1445)
    demod.setCenterFreqHz(1360.0f);
    demod.setShiftHz(170.0f);

    const QString testStr = "LOW TONE TEST";
    const QVector<float> audio = generateRttyAudio(testStr, 45.4545f, 1360.0f, 170.0f);

    const int chunkSize = 512;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("LOW TONE TEST"),
             qPrintable(QString("Expected 'LOW TONE TEST', got: '%1'").arg(demod.recentText())));
}

void RttyDemodulatorTests::testReversePolarity() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);
    demod.setReversePolarity(true);

    const QString testStr = "REVERSE POLARITY";
    // Generate with reversePolarity = true
    const QVector<float> audio = generateRttyAudio(testStr, 45.4545f, 2210.0f, 170.0f, true);

    const int chunkSize = 512;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("REVERSE POLARITY"),
             qPrintable(QString("Expected 'REVERSE POLARITY', got: '%1'").arg(demod.recentText())));
}

void RttyDemodulatorTests::testUsbModeUsesAmateurHighTones() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);
    demod.setUsbMode(true);

    QCOMPARE(demod.markFreqHz(), 2125.0f);
    QCOMPARE(demod.spaceFreqHz(), 2295.0f);

    const QString testStr = "CQ CQ DE ZL2BRG K";
    const QVector<float> audio = generateRttyAudio(testStr);

    const int chunkSize = 480;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("CQ CQ DE ZL2BRG K"),
             qPrintable(QString("USB mode should decode amateur high tones without REV, got: '%1'")
                            .arg(demod.recentText())));
}

void RttyDemodulatorTests::testInvertedTonesRequireReverse() {
    const QString testStr = "NEED REV";
    const QVector<float> inverted = generateRttyAudio(testStr, 45.4545f, 2210.0f, 170.0f, true);

    RttyDemodulator withoutRev;
    withoutRev.setCenterFreqHz(2210.0f);
    withoutRev.setShiftHz(170.0f);
    withoutRev.processAudio(inverted.constData(), inverted.size(), 48000);
    QVERIFY2(!withoutRev.recentText().contains("NEED REV"),
             qPrintable(QString("Inverted tones must not decode without REV, got: '%1'")
                            .arg(withoutRev.recentText())));

    RttyDemodulator withRev;
    withRev.setCenterFreqHz(2210.0f);
    withRev.setShiftHz(170.0f);
    withRev.setReversePolarity(true);
    withRev.setUsbMode(true);
    withRev.processAudio(inverted.constData(), inverted.size(), 48000);
    QVERIFY2(withRev.recentText().contains("NEED REV"),
             qPrintable(QString("Inverted tones should decode with REV, got: '%1'")
                            .arg(withRev.recentText())));
}

void RttyDemodulatorTests::testRyWithOnePointFiveStopBits() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);
    demod.setUsbMode(true);

    const QString testStr = "RYRYRYRY";
    const QVector<float> audio = generateRttyAudio(
        testStr, 45.4545f, 2210.0f, 170.0f, false, 100.0f, 48000, 1.5f);

    const int chunkSize = 512;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("RYRYRYRY"),
             qPrintable(QString("1.5 stop-bit RY should decode, got: '%1'").arg(demod.recentText())));
}

void RttyDemodulatorTests::testWeather50BaudFigsPersist() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2125.0f);
    demod.setShiftHz(450.0f);
    demod.setBaudRate(50.0f);
    demod.setUsosEnabled(false);
    demod.setUsbMode(true);

    const QString testStr = QStringLiteral("20. 17");
    const QVector<float> audio = generateRttyAudio(
        testStr, 50.0f, 2125.0f, 450.0f, false, 100.0f, 48000, 1.5f);

    const int chunkSize = 512;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains(testStr),
             qPrintable(QString("50 baud 1.5-stop weather should copy, got: '%1'")
                            .arg(demod.recentText())));
}

void RttyDemodulatorTests::testBaudRateTolerance() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);
    demod.setBaudRate(45.4545f);

    // Test with slight transmit baud offset (45.0 baud instead of 45.45)
    const QString testStr = "CLOCK JITTER 45";
    const QVector<float> audio = generateRttyAudio(testStr, 45.0f);

    const int chunkSize = 512;
    for (int i = 0; i < audio.size(); i += chunkSize) {
        const int count = qMin(chunkSize, audio.size() - i);
        demod.processAudio(audio.constData() + i, count, 48000);
    }

    QVERIFY2(demod.recentText().contains("CLOCK JITTER 45"),
             qPrintable(QString("Expected 'CLOCK JITTER 45', got: '%1'").arg(demod.recentText())));
}

void RttyDemodulatorTests::testAutoBaudDetects50Not45() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);
    demod.setBaudRate(45.4545f);
    demod.setAutoDetectEnabled(true);

    QSignalSpy baudSpy(demod.classifier(), &RttyAutoClassifier::baudRateDetected);

    const QString testStr = QStringLiteral("RY").repeated(24);
    const QVector<float> audio = generateRttyAudio(
        testStr, 50.0f, 2210.0f, 170.0f, false, 100.0f, 48000, 1.5f);
    demod.processAudio(audio.constData(), audio.size(), 48000);

    QVERIFY2(baudSpy.count() >= 1, "auto-baud should lock on a 50 baud RY stream");
    QCOMPARE(baudSpy.last().at(0).toFloat(), 50.0f);
}

void RttyDemodulatorTests::testWeatherShiftIgnoresAmateurAutoBaud() {
    RttyDemodulator demod;
    demod.setShiftHz(450.0f);
    demod.setBaudRate(50.0f);
    demod.setAutoDetectEnabled(true);

    QVERIFY(QMetaObject::invokeMethod(demod.classifier(), "baudRateDetected",
                                     Q_ARG(float, 45.4545f)));
    QCOMPARE(demod.baudRate(), 50.0f);

    QVERIFY(QMetaObject::invokeMethod(demod.classifier(), "baudRateDetected",
                                     Q_ARG(float, 50.0f)));
    QCOMPARE(demod.baudRate(), 50.0f);
}

void RttyDemodulatorTests::testResetAndClear() {
    RttyDemodulator demod;
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);

    const QVector<float> audio = generateRttyAudio("TEST");
    demod.processAudio(audio.constData(), audio.size(), 48000);

    QVERIFY(!demod.recentText().isEmpty());

    demod.clearText();
    QCOMPARE(demod.recentText(), QString());

    demod.reset();
    QCOMPARE(demod.trackedOffsetHz(), 0.0f);
}

void RttyDemodulatorTests::testTuningScopeCrossedEllipses()
{
    auto tone = [](float hz, int n, int sampleRate = 48000) {
        QVector<float> audio(n);
        const float step = 2.0f * static_cast<float>(M_PI) * hz / static_cast<float>(sampleRate);
        float phase = 0.0f;
        for (int i = 0; i < n; ++i) {
            audio[i] = 0.4f * std::sin(phase);
            phase += step;
            if (phase > 2.0f * static_cast<float>(M_PI))
                phase -= 2.0f * static_cast<float>(M_PI);
        }
        return audio;
    };
    auto rms = [](const QVector<float> &v) {
        double sum = 0.0;
        for (float s : v)
            sum += double(s) * double(s);
        return std::sqrt(sum / double(qMax(1, v.size())));
    };

    RttyDemodulator demod;
    demod.setAfcEnabled(false);
    demod.setCenterFreqHz(2210.0f);
    demod.setShiftHz(170.0f);

    const int n = 48000 / 4;
    const QVector<float> markTone = tone(2125.0f, n);
    demod.processAudio(markTone.constData(), markTone.size(), 48000);
    QVERIFY(demod.lastScopeXs().size() >= 256);
    QVERIFY(rms(demod.lastScopeXs()) > 3.0 * rms(demod.lastScopeYs()));

    demod.reset();
    const QVector<float> spaceTone = tone(2295.0f, n);
    demod.processAudio(spaceTone.constData(), spaceTone.size(), 48000);
    QVERIFY(demod.lastScopeYs().size() >= 256);
    QVERIFY(rms(demod.lastScopeYs()) > 3.0 * rms(demod.lastScopeXs()));
}

QTEST_MAIN(RttyDemodulatorTests)
#include "rtty_demodulator_tests.moc"
