#include <QtTest/QtTest>
#include <QSignalSpy>
#include <cmath>

#include "AudioEngine/RttyDemodulator.h"
#include "AudioEngine/RttyBayesianDecoder.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class RttyBayesianDecoderTests : public QObject {
    Q_OBJECT

private:
    // Helper to generate soft symbols from bits
    QVector<RttySoftSymbol> generateSymbolsFromBits(const QVector<int> &bits, float strongLlr = 8.0f);

    // Helper to convert ASCII text to Baudot bits (Start + 5 Data + Stop)
    QVector<int> textToBaudotBits(const QString &text, bool usos = true);

    // Helper to generate 48 kHz synthetic audio for end-to-end test
    QVector<float> generateRttyAudio(
        const QString &text,
        float baudRate = 45.4545f,
        float centerFreq = 2210.0f,
        float shiftHz = 170.0f,
        bool reversePolarity = false,
        int sampleRate = 48000);

private slots:
    void testCleanSoftSymbols();
    void testNoiseGlitchRejection();
    void testWeakBitBayesianRecovery();
    void testFigsShiftAndUsos();
    void testWeatherFigsPersistAcrossSpace();
    void testWeakIdleLtrsRejected();
    void testNegativeStopRejected();
    void testMissedSpaceStopDoesNotResync();
    void testStrongStopLtrsWithModerateStart();
    void testSmartSquelch();
    void testEndToEndIntegration();
    void testEndToEndUsbMode();
    void testEndToEndRySequence();
    void testEndToEndInvertedRequiresReverse();
};

QVector<RttySoftSymbol> RttyBayesianDecoderTests::generateSymbolsFromBits(
    const QVector<int> &bits, float strongLlr)
{
    QVector<RttySoftSymbol> symbols;
    symbols.reserve(bits.size());
    for (int b : bits) {
        RttySoftSymbol sym;
        sym.llr = (b == 1) ? strongLlr : -strongLlr;
        sym.markMag = (b == 1) ? 1.0f : 0.05f;
        sym.spaceMag = (b == 0) ? 1.0f : 0.05f;
        sym.snrDb = 20.0f;
        sym.sampleIdx = 0;
        symbols.append(sym);
    }
    return symbols;
}

QVector<int> RttyBayesianDecoderTests::textToBaudotBits(const QString &text, bool usos) {
    static const struct { char c; quint8 code; bool figs; } CHAR_MAP[] = {
        { 'A', 0x03, false }, { 'B', 0x19, false }, { 'C', 0x0E, false }, { 'D', 0x09, false },
        { 'E', 0x01, false }, { 'F', 0x0D, false }, { 'G', 0x1A, false }, { 'H', 0x14, false },
        { 'I', 0x06, false }, { 'J', 0x0B, false }, { 'K', 0x0F, false }, { 'L', 0x12, false },
        { 'M', 0x1C, false }, { 'N', 0x0C, false }, { 'O', 0x18, false }, { 'P', 0x16, false },
        { 'Q', 0x17, false }, { 'R', 0x0A, false }, { 'S', 0x05, false }, { 'T', 0x10, false },
        { 'U', 0x07, false }, { 'V', 0x1E, false }, { 'W', 0x13, false }, { 'X', 0x1D, false },
        { 'Y', 0x15, false }, { 'Z', 0x11, false }, { ' ', 0x04, false }, { '\n', 0x02, false },
        { '\r', 0x08, false },
        // FIGS
        { '0', 0x16, true }, { '1', 0x17, true }, { '2', 0x13, true }, { '3', 0x01, true },
        { '4', 0x0A, true }, { '5', 0x10, true }, { '6', 0x15, true }, { '7', 0x07, true },
        { '8', 0x06, true }, { '9', 0x18, true }, { '-', 0x03, true }, { '?', 0x19, true },
        { ':', 0x0E, true }, { '$', 0x09, true }, { '!', 0x0D, true }, { '&', 0x1A, true }
    };
    static const int MAP_SIZE = sizeof(CHAR_MAP) / sizeof(CHAR_MAP[0]);

    QVector<int> bitStream;
    // Initial idle Mark bits
    for (int i = 0; i < 5; ++i) {
        bitStream.append(1);
    }

    bool currentFigs = false;

    for (const QChar &qc : text) {
        const char c = qc.toLatin1();
        quint8 code = 0;
        bool needFigs = currentFigs;
        bool found = false;

        for (int i = 0; i < MAP_SIZE; ++i) {
            if (CHAR_MAP[i].c == c) {
                code = CHAR_MAP[i].code;
                needFigs = CHAR_MAP[i].figs;
                found = true;
                break;
            }
        }
        if (!found) continue;

        if (needFigs != currentFigs && code != 0x04 && code != 0x08 && code != 0x02) {
            // Send FIGS or LTRS shift code
            const quint8 shiftCode = needFigs ? 0x1B : 0x1F;
            bitStream.append(0); // Start
            for (int b = 0; b < 5; ++b) {
                bitStream.append((shiftCode >> b) & 1);
            }
            bitStream.append(1); // Stop
            bitStream.append(1);
            currentFigs = needFigs;
        }

        // Send character frame: Start (0) + 5 Data + 2 Stop (1)
        bitStream.append(0);
        for (int b = 0; b < 5; ++b) {
            bitStream.append((code >> b) & 1);
        }
        bitStream.append(1);
        bitStream.append(1);

        if (code == 0x04 && usos) {
            // Space: USOS resets to LTRS (amateur). Weather leaves FIGS armed.
            currentFigs = false;
        }
    }

    // Trailing idle Mark
    for (int i = 0; i < 5; ++i) {
        bitStream.append(1);
    }

    return bitStream;
}

QVector<float> RttyBayesianDecoderTests::generateRttyAudio(
    const QString &text, float baudRate, float centerFreq, float shiftHz,
    bool reversePolarity, int sampleRate)
{
    const QVector<int> bits = textToBaudotBits(text);
    const float markFreq = reversePolarity ? (centerFreq + shiftHz * 0.5f) : (centerFreq - shiftHz * 0.5f);
    const float spaceFreq = reversePolarity ? (centerFreq - shiftHz * 0.5f) : (centerFreq + shiftHz * 0.5f);
    const int samplesPerBit = qRound(static_cast<float>(sampleRate) / baudRate);

    QVector<float> audio;
    audio.reserve(bits.size() * samplesPerBit);

    float phase = 0.0f;
    for (int b : bits) {
        const float freq = (b == 1) ? markFreq : spaceFreq;
        const float phaseInc = 2.0f * static_cast<float>(M_PI) * freq / static_cast<float>(sampleRate);
        for (int s = 0; s < samplesPerBit; ++s) {
            audio.append(0.8f * std::sin(phase));
            phase += phaseInc;
            if (phase > 2.0f * static_cast<float>(M_PI)) {
                phase -= 2.0f * static_cast<float>(M_PI);
            }
        }
    }
    return audio;
}

void RttyBayesianDecoderTests::testCleanSoftSymbols() {
    RttyBayesianDecoder decoder;
    QSignalSpy charSpy(&decoder, &RttyBayesianDecoder::characterDecoded);

    const QString testText = "RYRY";
    const QVector<int> bits = textToBaudotBits(testText);
    const QVector<RttySoftSymbol> symbols = generateSymbolsFromBits(bits, 8.0f);

    decoder.processSymbols(symbols);

    QCOMPARE(decoder.recentText(), testText);
    QVERIFY(charSpy.count() >= 4);

    // Verify confidence metric on emitted characters
    for (int i = 0; i < charSpy.count(); ++i) {
        const float conf = charSpy.at(i).at(2).toFloat();
        const float errProb = charSpy.at(i).at(3).toFloat();
        QVERIFY(conf > 0.95f);
        QVERIFY(errProb < 0.05f);
    }
}

void RttyBayesianDecoderTests::testNoiseGlitchRejection() {
    RttyBayesianDecoder decoder;
    QSignalSpy charSpy(&decoder, &RttyBayesianDecoder::characterDecoded);

    // Stream of idle Mark (1s) with an isolated glitch (0) in the middle
    QVector<int> bits;
    for (int i = 0; i < 15; ++i) bits.append(1);
    bits.append(0); // Isolated noise pulse
    for (int i = 0; i < 15; ++i) bits.append(1);

    // Followed by valid character "A" (code 0x03 = 00011, LSB first: 1, 1, 0, 0, 0)
    // Start (0), Data (1, 1, 0, 0, 0), Stop (1)
    bits.append(0);
    bits.append(1);
    bits.append(1);
    bits.append(0);
    bits.append(0);
    bits.append(0);
    bits.append(1);

    // Trailing idle
    for (int i = 0; i < 5; ++i) bits.append(1);

    const QVector<RttySoftSymbol> symbols = generateSymbolsFromBits(bits, 8.0f);
    decoder.processSymbols(symbols);

    // The noise pulse must NOT produce a bogus character!
    // Only "A" should be decoded.
    QCOMPARE(decoder.recentText(), QString("A"));
    QCOMPARE(charSpy.count(), 1);
}

void RttyBayesianDecoderTests::testWeakBitBayesianRecovery() {
    // Character 'E' has Baudot code 0x01 (binary 00001, LSB first: bit0=1, bits1..4=0)
    // Suppose bit0 is heavily attenuated by fading/noise to near zero or slightly negative:
    // e.g. L0 = -0.1 (weakly space instead of mark!), L1..4 = -5.0 (solid space)
    float dataLlrs[5] = { -0.1f, -5.0f, -5.0f, -5.0f, -5.0f };

    // Under hard slicing:
    // Slicer sees (0, 0, 0, 0, 0) = code 0x00 (Null/Blank), meaning 'E' is lost!
    // But under Bayesian evaluation with English priors:
    // 'E' (code 0x01) has high prior (0.085), while Null (0x00) has tiny prior (0.002)
    BayesianCharResult res = RttyBayesianDecoder::evaluateBaudotPosterior(dataLlrs, false);

    QCOMPARE(res.baudotCode, static_cast<quint8>(0x01));
    QCOMPARE(res.character, QChar('E'));
    QVERIFY(res.confidence > 0.50f);
}

void RttyBayesianDecoderTests::testFigsShiftAndUsos() {
    RttyBayesianDecoder decoder;
    decoder.setUsosEnabled(true);

    const QString testText = "CQ 599 K";
    const QVector<int> bits = textToBaudotBits(testText);
    const QVector<RttySoftSymbol> symbols = generateSymbolsFromBits(bits, 8.0f);

    decoder.processSymbols(symbols);

    QCOMPARE(decoder.recentText(), testText);
}

void RttyBayesianDecoderTests::testWeatherFigsPersistAcrossSpace() {
    RttyBayesianDecoder decoder;
    decoder.setUsosEnabled(false);

    const QString testText = QStringLiteral("12 34");
    const QVector<int> bits = textToBaudotBits(testText, false);
    const QVector<RttySoftSymbol> symbols = generateSymbolsFromBits(bits, 8.0f);

    decoder.processSymbols(symbols);

    QCOMPARE(decoder.recentText(), testText);
}

void RttyBayesianDecoderTests::testWeakIdleLtrsRejected() {
    RttyBayesianDecoder decoder;

    // Tiny Space dip into Mark idle, then five Marks + Mark: looks like LTRS
    // but is not a real start. Must not lock or flip shift state.
    QVector<RttySoftSymbol> symbols;
    for (int i = 0; i < 8; ++i) {
        RttySoftSymbol mark;
        mark.llr = 8.0f;
        mark.snrDb = 12.0f;
        symbols.append(mark);
    }
    RttySoftSymbol dip;
    dip.llr = -1.5f;
    dip.snrDb = 12.0f;
    symbols.append(dip);
    for (int i = 0; i < 6; ++i) {
        RttySoftSymbol mark;
        mark.llr = 8.0f;
        mark.snrDb = 12.0f;
        symbols.append(mark);
    }
    // Real 'A'
    const float aBits[] = { -8.0f, 8.0f, 8.0f, -8.0f, -8.0f, -8.0f, 8.0f };
    for (float llr : aBits) {
        RttySoftSymbol s;
        s.llr = llr;
        s.snrDb = 12.0f;
        symbols.append(s);
    }

    decoder.processSymbols(symbols);
    QCOMPARE(decoder.recentText(), QStringLiteral("A"));
    QCOMPARE(decoder.figsBelief(), 0.0f);
}

void RttyBayesianDecoderTests::testNegativeStopRejected() {
    RttyBayesianDecoder decoder;

    // Well-started 'L' (0x12) whose stop strobe is already the next start.
    const float bits[] = {
        -11.3f, 8.0f, -8.0f, 8.0f, -8.0f, -8.0f, -9.1f
    };
    QVector<RttySoftSymbol> symbols;
    for (float llr : bits) {
        RttySoftSymbol s;
        s.llr = llr;
        s.snrDb = 12.0f;
        symbols.append(s);
    }
    decoder.processSymbols(symbols);
    QVERIFY(decoder.recentText().isEmpty());
}

void RttyBayesianDecoderTests::testMissedSpaceStopDoesNotResync() {
    RttyBayesianDecoder decoder;

    // Rejected 'L' (Space stop) then a clean 'A'. Reusing the Space stop as the
    // next start would swallow A's start bit and print garbage instead of A.
    const float bits[] = {
        -11.3f, 8.0f, -8.0f, 8.0f, -8.0f, -8.0f, -9.1f,
        -8.0f, 8.0f, 8.0f, -8.0f, -8.0f, -8.0f, 8.0f
    };
    QVector<RttySoftSymbol> symbols;
    for (float llr : bits) {
        RttySoftSymbol s;
        s.llr = llr;
        s.snrDb = 12.0f;
        symbols.append(s);
    }
    decoder.processSymbols(symbols);
    QCOMPARE(decoder.recentText(), QStringLiteral("A"));
}

void RttyBayesianDecoderTests::testStrongStopLtrsWithModerateStart() {
    RttyBayesianDecoder decoder;

    // On-air: start≈−4.7, five Marks, stop=+98. That is a real LTRS, not idle.
    const float bits[] = { -4.7f, 20.0f, 20.0f, 20.0f, 20.0f, 20.0f, 98.6f };
    QVector<RttySoftSymbol> symbols;
    for (float llr : bits) {
        RttySoftSymbol s;
        s.llr = llr;
        s.snrDb = 14.0f;
        symbols.append(s);
    }
    decoder.processSymbols(symbols);
    QCOMPARE(decoder.figsBelief(), 0.0f);
    QVERIFY(decoder.isFramingLocked() || decoder.framingConfidence() > 10.0f);
}

void RttyBayesianDecoderTests::testSmartSquelch() {
    RttyBayesianDecoder decoder;
    decoder.setSquelchThreshold(0.40f);

    // Generate random noise symbols around zero LLR
    QVector<RttySoftSymbol> noiseSymbols;
    for (int i = 0; i < 200; ++i) {
        RttySoftSymbol sym;
        // Pseudo-random Gaussian-like noise between -1.0 and +1.0
        sym.llr = (static_cast<float>(i % 7) - 3.0f) * 0.3f;
        sym.markMag = 0.5f;
        sym.spaceMag = 0.5f;
        sym.snrDb = 0.0f;
        sym.sampleIdx = 0;
        noiseSymbols.append(sym);
    }

    decoder.processSymbols(noiseSymbols);

    // Random noise must be squelched (0 characters output)
    QVERIFY(decoder.recentText().isEmpty());
    QVERIFY(!decoder.isFramingLocked());
}

void RttyBayesianDecoderTests::testEndToEndIntegration() {
    RttyDemodulator demod;
    RttyBayesianDecoder decoder;

    connect(&demod, &RttyDemodulator::symbolSampled, &decoder, &RttyBayesianDecoder::processSymbol);

    const QString message = "TEST DE ZL2BRG 73";
    const QVector<float> audio = generateRttyAudio(message);

    demod.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText(), message);
}

void RttyBayesianDecoderTests::testEndToEndUsbMode() {
    RttyDemodulator demod;
    RttyBayesianDecoder decoder;
    demod.setUsbMode(true);
    connect(&demod, &RttyDemodulator::symbolSampled, &decoder, &RttyBayesianDecoder::processSymbol);

    const QString message = "TEST DE ZL2BRG 73";
    const QVector<float> audio = generateRttyAudio(message);
    demod.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText(), message);
}

void RttyBayesianDecoderTests::testEndToEndRySequence() {
    RttyDemodulator demod;
    RttyBayesianDecoder decoder;
    demod.setUsbMode(true);
    connect(&demod, &RttyDemodulator::symbolSampled, &decoder, &RttyBayesianDecoder::processSymbol);

    const QString message = "RYRYRYRY";
    const QVector<float> audio = generateRttyAudio(message);
    demod.processAudio(audio.constData(), audio.size(), 48000);

    QCOMPARE(decoder.recentText(), message);
}

void RttyBayesianDecoderTests::testEndToEndInvertedRequiresReverse() {
    const QString message = "TEST DE ZL2BRG 73";
    const QVector<float> inverted = generateRttyAudio(message, 45.4545f, 2210.0f, 170.0f, true);

    RttyDemodulator demod;
    RttyBayesianDecoder decoder;
    demod.setUsbMode(true);
    connect(&demod, &RttyDemodulator::symbolSampled, &decoder, &RttyBayesianDecoder::processSymbol);
    demod.processAudio(inverted.constData(), inverted.size(), 48000);
    QVERIFY(decoder.recentText() != message);

    RttyDemodulator demodRev;
    RttyBayesianDecoder decoderRev;
    demodRev.setUsbMode(true);
    demodRev.setReversePolarity(true);
    connect(&demodRev, &RttyDemodulator::symbolSampled, &decoderRev, &RttyBayesianDecoder::processSymbol);
    demodRev.processAudio(inverted.constData(), inverted.size(), 48000);
    QCOMPARE(decoderRev.recentText(), message);
}

QTEST_MAIN(RttyBayesianDecoderTests)
#include "rtty_bayesian_decoder_tests.moc"
