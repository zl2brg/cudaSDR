#include <QtTest/QtTest>

#include "Util/tci_protocol_utils.h"

using namespace TciProtocol;

class TciProtocolUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void tciMessageFormatting();
    void parseBoolArgVariants();
    void dspModeRoundTrip();
    void effectiveIqSampleRateAvoidsAudioCollision();
    void effectiveIqSampleRateHonorsOverride();
    void streamHeaderRoundTrip();
    void buildTxAudioFrameLayout();
    void buildTxChronoFrameLayout();
    void duplicatedStereoDetection();
    void resolveWsjtOversizedTxAudioLayout();
    void resolveWsjtOversizedIgnoresLrMismatch();
    void resolveAndroidTightDuplicatedStereoLayout();
    void decodeTxAudioFloat32Mono();
    void decodeTxAudioInt16StereoKeepsLeft();
    void chunkTxAudioResidualProducesDspBlocks();
    void chunkTxAudioResidualDropsWhenQueueFull();
    void acceptsTxAudioOnlyDuringMoxOrTune();
    void parseAudioFormatVariants();
    void parseStreamHeaderRejectsShortBuffer();
    void decodeTxAudioUnsupportedFormat();
    void resolveWdspModeFdvSideband();
    void multiChunkTxAudioResidual();
    void isValidVfoHzBounds();
    void buildRxAudioFrameFloat32Stereo();
    void buildRxAudioFrameInt16Mono();
    void buildIqFrameFloat32();
};

void TciProtocolUtilsTests::tciMessageFormatting()
{
    QCOMPARE(tciMessage(QStringLiteral("READY")), QStringLiteral("ready;"));
    QCOMPARE(tciMessage(QStringLiteral("VFO"), {QStringLiteral("0"), QStringLiteral("1"), QStringLiteral("7050000")}),
             QStringLiteral("vfo:0,1,7050000;"));
}

void TciProtocolUtilsTests::parseBoolArgVariants()
{
    QVERIFY(parseBoolArg(QStringLiteral("true")));
    QVERIFY(parseBoolArg(QStringLiteral("1")));
    QVERIFY(parseBoolArg(QStringLiteral("on")));
    QVERIFY(parseBoolArg(QStringLiteral("tx")));
    QVERIFY(!parseBoolArg(QStringLiteral("false")));
    QVERIFY(!parseBoolArg(QStringLiteral("0")));
    QVERIFY(!parseBoolArg(QStringLiteral("off")));
}

void TciProtocolUtilsTests::dspModeRoundTrip()
{
    QCOMPARE(dspModeToTci(USB), QStringLiteral("usb"));
    QCOMPARE(dspModeToTci(CWU), QStringLiteral("cw"));
    QCOMPARE(tciModeToDsp(QStringLiteral("NFM")), FMN);
    QCOMPARE(tciModeToDsp(QStringLiteral("FREEDV")), FDV);
    QCOMPARE(tciModeToDsp(QStringLiteral("unknown-mode")), USB);
}

void TciProtocolUtilsTests::effectiveIqSampleRateAvoidsAudioCollision()
{
    QCOMPARE(effectiveIqSampleRate(48000), 96000);
    QCOMPARE(effectiveIqSampleRate(96000), 96000);
    QCOMPARE(effectiveIqSampleRate(192000), 192000);
    QCOMPARE(effectiveIqSampleRate(0), 96000);
}

void TciProtocolUtilsTests::effectiveIqSampleRateHonorsOverride()
{
    QCOMPARE(effectiveIqSampleRate(48000, 48000, 120000), 120000);
}

void TciProtocolUtilsTests::streamHeaderRoundTrip()
{
    StreamHeader in;
    in.receiver = 0;
    in.sampleRate = 48000;
    in.format = 3;
    in.length = 1024;
    in.streamType = kTxAudioStreamType;
    in.channels = 1;

    QByteArray frame = buildStreamHeader(in);
    QCOMPARE(frame.size(), kStreamHeaderBytes);

    StreamHeader out;
    QVERIFY(parseStreamHeader(frame, out));
    QCOMPARE(out.receiver, in.receiver);
    QCOMPARE(out.sampleRate, in.sampleRate);
    QCOMPARE(out.format, in.format);
    QCOMPARE(out.length, in.length);
    QCOMPARE(out.streamType, in.streamType);
    QCOMPARE(out.channels, in.channels);
}

void TciProtocolUtilsTests::buildTxAudioFrameLayout()
{
    const float samples[] = {0.25f, -0.5f, 0.75f};
    const QByteArray frame = buildTxAudioFrame(samples, 3, 48000, 0);

    QVERIFY(frame.size() > kStreamHeaderBytes);

    StreamHeader hdr;
    QVERIFY(parseStreamHeader(frame, hdr));
    QCOMPARE(hdr.streamType, kTxAudioStreamType);
    QCOMPARE(hdr.format, 3u);
    QCOMPARE(hdr.channels, 1u);
    QCOMPARE(hdr.length, 3u);
    QCOMPARE(hdr.sampleRate, 48000u);

    const float *payload = reinterpret_cast<const float *>(frame.constData() + kStreamHeaderBytes);
    QCOMPARE(payload[0], 0.25f);
    QCOMPARE(payload[1], -0.5f);
    QCOMPARE(payload[2], 0.75f);
}

void TciProtocolUtilsTests::buildTxChronoFrameLayout()
{
    const QByteArray frame = buildTxChronoFrame(0, 48000, kTxChronoSamples);
    QCOMPARE(frame.size(), kStreamHeaderBytes);

    StreamHeader hdr;
    QVERIFY(parseStreamHeader(frame, hdr));
    QCOMPARE(hdr.streamType, kTxChronoStreamType);
    QCOMPARE(hdr.format, static_cast<quint32>(kStreamFormatFloat32));
    QCOMPARE(hdr.channels, 2u);
    QCOMPARE(hdr.length, static_cast<quint32>(kTxChronoSamples));
    QCOMPARE(hdr.sampleRate, 48000u);
    QCOMPARE(hdr.receiver, 0u);
}

void TciProtocolUtilsTests::duplicatedStereoDetection()
{
    // WSJT-X style: L=R pairs must be detected as duplicated stereo.
    QVector<float> dup(8);
    for (int i = 0; i < 4; ++i) {
        dup[i * 2] = 0.1f * static_cast<float>(i + 1);
        dup[i * 2 + 1] = dup[i * 2];
    }
    QVERIFY(isDuplicatedStereoFloat32(dup.constData(), dup.size()));

    // True mono (adjacent samples differ) must not be treated as stereo.
    QVector<float> mono = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f};
    QVERIFY(!isDuplicatedStereoFloat32(mono.constData(), mono.size()));

    // Decoding duplicated stereo as channels=2 yields half the floats (correct timebase).
    QVector<double> out;
    QVERIFY(decodeTxAudioMonoSamples(
        QByteArray(reinterpret_cast<const char *>(dup.constData()),
                   dup.size() * static_cast<int>(sizeof(float))),
        kStreamFormatFloat32, 2, out));
    QCOMPARE(out.size(), 4);
    QVERIFY(qAbs(out.at(0) - 0.1) < 1e-5);
    QVERIFY(qAbs(out.at(3) - 0.4) < 1e-5);
}

void TciProtocolUtilsTests::resolveWsjtOversizedTxAudioLayout()
{
    // WSJT-X: length=2048, payload capacity 16384 (4096 floats), first 2048 floats
    // written as L=R pairs, trailing capacity left silent.
    constexpr int length = kTxChronoSamples; // 2048
    QVector<float> payload(length * 2, 0.0f);
    for (int i = 0; i < length / 2; ++i) {
        const float v = 0.01f * static_cast<float>((i % 50) + 1);
        payload[i * 2] = v;
        payload[i * 2 + 1] = v;
    }

    const TxAudioFloatLayout layout =
        resolveTxAudioFloat32Layout(payload.constData(), payload.size(), length, 2);
    QVERIFY(layout.ok);
    QCOMPARE(layout.useFloats, length);
    QCOMPARE(layout.channels, 2);

    QVector<double> mono;
    QVERIFY(decodeTxAudioMonoSamples(
        QByteArray(reinterpret_cast<const char *>(payload.constData()),
                   layout.useFloats * static_cast<int>(sizeof(float))),
        kStreamFormatFloat32, layout.channels, mono));
    QCOMPARE(mono.size(), length / 2); // 1024 mono samples
    QCOMPARE(mono.size(), kDspSampleSize);

    const TxAudioChunkResult chunked =
        chunkTxAudioResidual(mono, kDspSampleSize, /*maxQueueBlocks=*/16, /*currentQueueCount=*/0);
    QCOMPARE(chunked.blocks.size(), 1);
    QCOMPARE(chunked.residual.size(), 0);
    QCOMPARE(chunked.droppedBlocks, 0);
}

void TciProtocolUtilsTests::resolveWsjtOversizedIgnoresLrMismatch()
{
    // Same geometric signature as WSJT, but L≠R (tones/ramps) — must still
    // yield 1024 mono from geometry alone, not fall through to 2048.
    constexpr int length = kTxChronoSamples;
    QVector<float> payload(length * 2, 0.0f);
    for (int i = 0; i < length / 2; ++i) {
        payload[i * 2] = 0.1f * static_cast<float>((i % 30) + 1);
        payload[i * 2 + 1] = -0.05f * static_cast<float>((i % 17) + 1);
    }
    // Trailing half deliberately non-silent noise so silence heuristics cannot help.
    for (int i = length; i < length * 2; ++i)
        payload[i] = 0.02f * static_cast<float>((i % 11) + 1);

    QVERIFY(!isDuplicatedStereoFloat32(payload.constData(), length));

    const TxAudioFloatLayout layout =
        resolveTxAudioFloat32Layout(payload.constData(), payload.size(), length, 2);
    QVERIFY(layout.ok);
    QCOMPARE(layout.useFloats, length);
    QCOMPARE(layout.channels, 2);

    QVector<double> mono;
    QVERIFY(decodeTxAudioMonoSamples(
        QByteArray(reinterpret_cast<const char *>(payload.constData()),
                   layout.useFloats * static_cast<int>(sizeof(float))),
        kStreamFormatFloat32, layout.channels, mono));
    QCOMPARE(mono.size(), length / 2);
    QCOMPARE(mono.size(), kDspSampleSize);
}

void TciProtocolUtilsTests::resolveAndroidTightDuplicatedStereoLayout()
{
    // Android / other ExpertSDR apps: tight payload of length floats as L=R
    // (8192 bytes for length=2048) — must stay at 1024 mono, not regress.
    constexpr int length = kTxChronoSamples;
    QVector<float> payload(length);
    for (int i = 0; i < length / 2; ++i) {
        const float v = 0.02f * static_cast<float>((i % 40) + 1);
        payload[i * 2] = v;
        payload[i * 2 + 1] = v;
    }

    const TxAudioFloatLayout layout =
        resolveTxAudioFloat32Layout(payload.constData(), payload.size(), length, 2);
    QVERIFY(layout.ok);
    QCOMPARE(layout.useFloats, length);
    QCOMPARE(layout.channels, 2);

    QVector<double> mono;
    QVERIFY(decodeTxAudioMonoSamples(
        QByteArray(reinterpret_cast<const char *>(payload.constData()),
                   layout.useFloats * static_cast<int>(sizeof(float))),
        kStreamFormatFloat32, layout.channels, mono));
    QCOMPARE(mono.size(), length / 2);
}

void TciProtocolUtilsTests::decodeTxAudioFloat32Mono()
{
    const float samples[] = {0.1f, -0.2f};
    QByteArray payload(reinterpret_cast<const char *>(samples), sizeof(samples));

    QVector<double> mono;
    QVERIFY(decodeTxAudioMonoSamples(payload, 3, 1, mono));
    QCOMPARE(mono.size(), 2);
    QVERIFY(qAbs(mono.at(0) - 0.1) < 1e-5);
    QVERIFY(qAbs(mono.at(1) + 0.2) < 1e-5);
}

void TciProtocolUtilsTests::decodeTxAudioInt16StereoKeepsLeft()
{
    const qint16 stereo[] = {16384, -16384, -8192, 8192};
    QByteArray payload(reinterpret_cast<const char *>(stereo), sizeof(stereo));

    QVector<double> mono;
    QVERIFY(decodeTxAudioMonoSamples(payload, 0, 2, mono));
    QCOMPARE(mono.size(), 2);
    QCOMPARE(mono.at(0), 0.5);
    QCOMPARE(mono.at(1), -0.25);
}

void TciProtocolUtilsTests::chunkTxAudioResidualProducesDspBlocks()
{
    QVector<double> residual(kDspSampleSize + 128, 0.125);

    const TxAudioChunkResult result = chunkTxAudioResidual(residual, kDspSampleSize, 48, 0);
    QCOMPARE(result.blocks.size(), 1);
    QCOMPARE(result.blocks.at(0).size(), kDspSampleSize);
    QCOMPARE(result.residual.size(), 128);
    QCOMPARE(result.droppedBlocks, 0);
}

void TciProtocolUtilsTests::chunkTxAudioResidualDropsWhenQueueFull()
{
    QVector<double> residual(kDspSampleSize * 3, 1.0);

    const TxAudioChunkResult result = chunkTxAudioResidual(residual, kDspSampleSize, 2, 2);
    QCOMPARE(result.blocks.size(), 0);
    QCOMPARE(result.droppedBlocks, 3);
    QCOMPARE(result.residual.size(), 0);
}

void TciProtocolUtilsTests::acceptsTxAudioOnlyDuringMoxOrTune()
{
    QVERIFY(acceptsTxAudio(RadioState::MOX));
    QVERIFY(acceptsTxAudio(RadioState::TUNE));
    QVERIFY(!acceptsTxAudio(RadioState::RX));
    QVERIFY(!acceptsTxAudio(RadioState::DUPLEX));
    QVERIFY(isTrxActive(RadioState::DUPLEX));
    QVERIFY(!isTrxActive(RadioState::RX));
}

void TciProtocolUtilsTests::parseAudioFormatVariants()
{
    QCOMPARE(parseAudioFormat(QStringLiteral("float32")), 3);
    QCOMPARE(parseAudioFormat(QStringLiteral("int16")), 0);
    QCOMPARE(parseAudioFormat(QStringLiteral("int24")), 1);
    QCOMPARE(parseAudioFormat(QStringLiteral("unknown")), 3);
}

void TciProtocolUtilsTests::parseStreamHeaderRejectsShortBuffer()
{
    StreamHeader hdr;
    const QByteArray shortBuf(63, '\0');
    QVERIFY(!parseStreamHeader(shortBuf, hdr));
}

void TciProtocolUtilsTests::decodeTxAudioUnsupportedFormat()
{
    QVector<double> mono;
    const QByteArray payload(4, '\0');
    QVERIFY(!decodeTxAudioMonoSamples(payload, 2, 1, mono));
    QCOMPARE(mono.size(), 0);
}

void TciProtocolUtilsTests::resolveWdspModeFdvSideband()
{
    QCOMPARE(resolveWDSPMode(FDV, 7'050'000), LSB);
    QCOMPARE(resolveWDSPMode(FDV, 14'200'000), USB);
    QCOMPARE(resolveWDSPMode(USB, 7'050'000), USB);
    QCOMPARE(resolveWDSPMode(LSB, 14'200'000), LSB);
}

void TciProtocolUtilsTests::multiChunkTxAudioResidual()
{
    QVector<double> residual;
    const float first[] = {0.1f, 0.2f};
    const float second[] = {0.3f, 0.4f};
    decodeTxAudioMonoSamples(QByteArray(reinterpret_cast<const char *>(first), sizeof(first)), 3, 1, residual);
    decodeTxAudioMonoSamples(QByteArray(reinterpret_cast<const char *>(second), sizeof(second)), 3, 1, residual);
    QCOMPARE(residual.size(), 4);

    TxAudioChunkResult chunked = chunkTxAudioResidual(residual, kDspSampleSize, 48, 0);
    QCOMPARE(chunked.blocks.size(), 0);
    QCOMPARE(chunked.residual.size(), 4);
}

void TciProtocolUtilsTests::isValidVfoHzBounds()
{
    QVERIFY(isValidVfoHz(kVfoMinHz));
    QVERIFY(isValidVfoHz(kVfoMaxHz));
    QVERIFY(isValidVfoHz(7'050'000));
    QVERIFY(!isValidVfoHz(kVfoMinHz - 1));
    QVERIFY(!isValidVfoHz(kVfoMaxHz + 1));
}

void TciProtocolUtilsTests::buildRxAudioFrameFloat32Stereo()
{
    const float stereo[] = {0.5f, -0.5f, 0.25f, -0.25f};
    const QByteArray frame = buildRxAudioFrame(0, 48'000, kStreamFormatFloat32, 2,
                                             stereo, 4);
    QVERIFY(!frame.isEmpty());

    StreamHeader hdr;
    QVERIFY(parseStreamHeader(frame, hdr));
    QCOMPARE(hdr.streamType, kRxAudioStreamType);
    QCOMPARE(hdr.format, static_cast<quint32>(kStreamFormatFloat32));
    QCOMPARE(hdr.channels, 2u);
    QCOMPARE(hdr.length, 4u);

    const float *payload = reinterpret_cast<const float *>(frame.constData() + kStreamHeaderBytes);
    QCOMPARE(payload[0], 0.5f);
    QCOMPARE(payload[3], -0.25f);
}

void TciProtocolUtilsTests::buildRxAudioFrameInt16Mono()
{
    const float stereo[] = {1.0f, 0.0f, -1.0f, 0.0f};
    const QByteArray frame = buildRxAudioFrame(0, 48'000, kStreamFormatInt16, 1,
                                             stereo, 4);
    QVERIFY(!frame.isEmpty());

    StreamHeader hdr;
    QVERIFY(parseStreamHeader(frame, hdr));
    QCOMPARE(hdr.streamType, kRxAudioStreamType);
    QCOMPARE(hdr.format, static_cast<quint32>(kStreamFormatInt16));
    QCOMPARE(hdr.channels, 1u);
    QCOMPARE(hdr.length, 2u);

    const qint16 *payload = reinterpret_cast<const qint16 *>(frame.constData() + kStreamHeaderBytes);
    QCOMPARE(payload[0], static_cast<qint16>(32767));
    QCOMPARE(payload[1], static_cast<qint16>(-32767));
}

void TciProtocolUtilsTests::buildIqFrameFloat32()
{
    const float iq[] = {0.1f, 0.2f, 0.3f, 0.4f};
    const QByteArray frame = buildIqFrame(0, 96'000, kStreamFormatFloat32, 2, iq, 4);
    QVERIFY(!frame.isEmpty());

    StreamHeader hdr;
    QVERIFY(parseStreamHeader(frame, hdr));
    QCOMPARE(hdr.streamType, kIqStreamType);
    QCOMPARE(hdr.sampleRate, 96'000u);
    QCOMPARE(hdr.length, 4u);
}

QTEST_APPLESS_MAIN(TciProtocolUtilsTests)
#include "tci_protocol_utils_tests.moc"
