#include <QtTest/QtTest>
#include <QWebSocket>
#include <QSignalSpy>

#include "Util/cusdr_tciserver.h"
#include "Util/cusdr_queue.h"
#include "Util/tci_protocol_utils.h"
#include "cusdr_settings.h"

using namespace TciProtocol;

class TciServerWsTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void connectReceivesReadyBurst();
    void trxCommandSetsMoxState();
    void trxCommandStartsTxChrono();
    void iqStartAdvertisesDistinctSampleRate();
    void txAudioEnqueuedWhileMox();
    void txAudioIgnoredWhileRx();
    void txAudioWsjtOversizedFrameYieldsOneBlock();
    void vfoOutOfRangeRejected();
    void vfoInRangeAccepted();
    void modulationCommandUpdatesMode();
    void iqStartStopTogglesActiveHint();
    void txAudioAccumulatesAcrossFrames();
    void nonTxBinaryStreamIgnored();
    void remoteControlChangedOnConnectDisconnect();
    void driveCommandClampsLevel();
    void tuneCommandSetsTuneState();
    void audioStartStopGatesRxBinaryStream();
    void compatStubCommandsAccepted();

private:
    Settings *m_settings = nullptr;
    TciServer *m_server = nullptr;
    QHQueue<QVector<double>> m_txQueue;
    QWebSocket m_client;
    QStringList m_textMessages;
    qint64 m_baselineVfoHz = 7'050'000;

    bool waitForConnected(int timeoutMs = 5000);
    bool waitForMessageContaining(const QString &needle, int timeoutMs = 5000);
    void drainTxQueue();
};

void TciServerWsTests::initTestCase()
{
    m_settings = Settings::instance();
    m_settings->setTciServerEnabled(true);
    m_settings->setVFOFrequency(0, 0, m_baselineVfoHz);
    m_settings->setCtrFrequency(0, 0, m_baselineVfoHz);
    m_settings->setDSPMode(0, USB);

    m_server = new TciServer();
    m_server->setTransmitAudioQueue(&m_txQueue);
    QVERIFY(m_server->startListening(0));
    QVERIFY(m_server->port() > 0);
}

void TciServerWsTests::cleanupTestCase()
{
    m_client.close();
    if (m_server) {
        m_server->stopListening();
        delete m_server;
        m_server = nullptr;
    }
    Settings::delete_instance();
    m_settings = nullptr;
}

void TciServerWsTests::init()
{
    m_textMessages.clear();
    drainTxQueue();
    m_settings->setRadioState(RadioState::RX);

    QObject::disconnect(&m_client, nullptr, this, nullptr);
    connect(&m_client, &QWebSocket::textMessageReceived, this, [this](const QString &msg) {
        m_textMessages.append(msg);
    });

    const QUrl url(QStringLiteral("ws://127.0.0.1:%1").arg(m_server->port()));
    m_client.open(url);
    QVERIFY2(waitForConnected(), "WebSocket client failed to connect to TciServer");
    QVERIFY2(waitForMessageContaining(QStringLiteral("ready;")),
             qPrintable(QStringLiteral("Missing ready; in init burst: ") + m_textMessages.join('|')));
}

void TciServerWsTests::cleanup()
{
    m_client.close();
    QVERIFY(QTest::qWaitFor([this]() { return m_client.state() == QAbstractSocket::UnconnectedState; }, 3000));
    m_settings->setRadioState(RadioState::RX);
    drainTxQueue();
}

bool TciServerWsTests::waitForConnected(int timeoutMs)
{
    return QTest::qWaitFor([this]() { return m_client.state() == QAbstractSocket::ConnectedState; }, timeoutMs);
}

bool TciServerWsTests::waitForMessageContaining(const QString &needle, int timeoutMs)
{
    return QTest::qWaitFor([this, &needle]() {
        for (const QString &msg : std::as_const(m_textMessages)) {
            if (msg.contains(needle))
                return true;
        }
        return false;
    }, timeoutMs);
}

void TciServerWsTests::drainTxQueue()
{
    while (m_txQueue.count() > 0)
        m_txQueue.dequeue();
}

void TciServerWsTests::connectReceivesReadyBurst()
{
    QVERIFY(m_textMessages.join('|').contains(QStringLiteral("device:cudaSDR")));
    QVERIFY(m_textMessages.join('|').contains(QStringLiteral("protocol:ExpertSDR3,1.5")));
    QVERIFY(m_textMessages.join('|').contains(QStringLiteral("audio_samplerate:48000")));
    QVERIFY(m_textMessages.join('|').contains(QStringLiteral("ready;")));
    QVERIFY(m_textMessages.join('|').contains(QStringLiteral("start;")));
}

void TciServerWsTests::trxCommandSetsMoxState()
{
    m_client.sendTextMessage(QStringLiteral("TRX:0,true;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::MOX; }, 3000));
    QCOMPARE(m_settings->getRadioState(), RadioState::MOX);

    m_client.sendTextMessage(QStringLiteral("TRX:0,false;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::RX; }, 3000));
}

void TciServerWsTests::trxCommandStartsTxChrono()
{
    QSignalSpy binarySpy(&m_client, &QWebSocket::binaryMessageReceived);
    QVERIFY(binarySpy.isValid());

    m_client.sendTextMessage(QStringLiteral("TRX:0,true;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::MOX; }, 3000));
    QVERIFY(QTest::qWaitFor([&binarySpy]() { return binarySpy.count() > 0; }, 3000));

    bool sawChrono = false;
    for (const QVariantList &args : binarySpy) {
        const QByteArray frame = args.at(0).toByteArray();
        StreamHeader hdr;
        if (!parseStreamHeader(frame, hdr))
            continue;
        if (hdr.streamType == kTxChronoStreamType) {
            QCOMPARE(hdr.length, static_cast<quint32>(kTxChronoSamples));
            QCOMPARE(frame.size(), kStreamHeaderBytes);
            sawChrono = true;
            break;
        }
    }
    QVERIFY2(sawChrono, "Expected TX_CHRONO binary frame after TRX:0,true");

    m_client.sendTextMessage(QStringLiteral("TRX:0,false;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::RX; }, 3000));
}

void TciServerWsTests::iqStartAdvertisesDistinctSampleRate()
{
    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("IQ_START:0,0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("iq_samplerate:")));

    QString samplerateLine;
    for (const QString &line : std::as_const(m_textMessages)) {
        if (line.contains(QStringLiteral("iq_samplerate:")))
            samplerateLine = line;
    }
    QVERIFY(!samplerateLine.isEmpty());
    const int colon = samplerateLine.indexOf(':');
    QVERIFY(colon >= 0);
    const int advertisedRate = samplerateLine.mid(colon + 1).remove(';').toInt();
    QVERIFY(advertisedRate > static_cast<int>(kRxAudioRateHz));
}

void TciServerWsTests::txAudioEnqueuedWhileMox()
{
    m_client.sendTextMessage(QStringLiteral("TRX:0,true;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::MOX; }, 3000));

    QVector<float> samples(DSP_SAMPLE_SIZE);
    for (int i = 0; i < DSP_SAMPLE_SIZE; ++i)
        samples[i] = static_cast<float>(i) / static_cast<float>(DSP_SAMPLE_SIZE);

    const QByteArray frame = buildTxAudioFrame(samples.constData(), samples.size());
    m_client.sendBinaryMessage(frame);

    QVERIFY(QTest::qWaitFor([this]() { return m_txQueue.count() > 0; }, 3000));
    const QVector<double> block = m_txQueue.dequeue();
    QCOMPARE(block.size(), DSP_SAMPLE_SIZE);
    QVERIFY(block.at(0) >= 0.0);
    QVERIFY(block.at(DSP_SAMPLE_SIZE - 1) < 1.0);
}

void TciServerWsTests::txAudioIgnoredWhileRx()
{
    QCOMPARE(m_settings->getRadioState(), RadioState::RX);

    QVector<float> samples(DSP_SAMPLE_SIZE, 0.5f);
    const QByteArray frame = buildTxAudioFrame(samples.constData(), samples.size());
    m_client.sendBinaryMessage(frame);

    QTest::qWait(200);
    QCOMPARE(m_txQueue.count(), 0);
}

void TciServerWsTests::txAudioWsjtOversizedFrameYieldsOneBlock()
{
    m_client.sendTextMessage(QStringLiteral("TRX:0,true;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::MOX; }, 3000));

    // WSJT-X layout: hdr.length=2048, payload capacity 16384. L≠R on purpose —
    // geometry alone must yield one DSP block (1024 mono), not sample matching.
    constexpr int length = kTxChronoSamples;
    QVector<float> payload(length * 2, 0.0f);
    for (int i = 0; i < length / 2; ++i) {
        payload[i * 2] = 0.25f;
        payload[i * 2 + 1] = -0.1f;
    }

    StreamHeader hdr;
    hdr.receiver = 0;
    hdr.sampleRate = 48000;
    hdr.format = kStreamFormatFloat32;
    hdr.length = static_cast<quint32>(length);
    hdr.streamType = kTxAudioStreamType;
    hdr.channels = 2;

    QByteArray frame = buildStreamHeader(hdr);
    frame.append(reinterpret_cast<const char *>(payload.constData()),
                 payload.size() * static_cast<int>(sizeof(float)));
    QCOMPARE(frame.size() - kStreamHeaderBytes, 16384);

    m_client.sendBinaryMessage(frame);
    QVERIFY(QTest::qWaitFor([this]() { return m_txQueue.count() > 0; }, 3000));
    QCOMPARE(m_txQueue.count(), 1);
    const QVector<double> block = m_txQueue.dequeue();
    QCOMPARE(block.size(), DSP_SAMPLE_SIZE);
    QVERIFY(qAbs(block.at(0) - 0.25) < 1e-5);
}

void TciServerWsTests::vfoOutOfRangeRejected()
{
    m_client.sendTextMessage(QStringLiteral("VFO:0,0,1000;"));
    QTest::qWait(100);
    QCOMPARE(m_settings->getVfoFrequency(0), m_baselineVfoHz);
}

void TciServerWsTests::vfoInRangeAccepted()
{
    const qint64 target = 14'200'000;
    m_client.sendTextMessage(QStringLiteral("VFO:0,0,%1;").arg(target));
    QTest::qWait(100);
    QCOMPARE(m_settings->getVfoFrequency(0), target);
}

void TciServerWsTests::modulationCommandUpdatesMode()
{
    m_client.sendTextMessage(QStringLiteral("MODULATION:0,LSB;"));
    QTest::qWait(100);
    QCOMPARE(m_settings->getDSPMode(0), LSB);
}

void TciServerWsTests::iqStartStopTogglesActiveHint()
{
    QVERIFY(!m_settings->tciIqActive());

    m_client.sendTextMessage(QStringLiteral("IQ_START:0,0;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->tciIqActive(); }, 3000));

    m_client.sendTextMessage(QStringLiteral("IQ_STOP:0,0;"));
    QVERIFY(QTest::qWaitFor([this]() { return !m_settings->tciIqActive(); }, 3000));
}

void TciServerWsTests::txAudioAccumulatesAcrossFrames()
{
    m_client.sendTextMessage(QStringLiteral("TRX:0,true;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::MOX; }, 3000));

    QVector<float> first(DSP_SAMPLE_SIZE / 2, 0.25f);
    QVector<float> second(DSP_SAMPLE_SIZE / 2, 0.75f);
    m_client.sendBinaryMessage(buildTxAudioFrame(first.constData(), first.size()));
    m_client.sendBinaryMessage(buildTxAudioFrame(second.constData(), second.size()));

    QVERIFY(QTest::qWaitFor([this]() { return m_txQueue.count() > 0; }, 3000));
    const QVector<double> block = m_txQueue.dequeue();
    QCOMPARE(block.size(), DSP_SAMPLE_SIZE);
}

void TciServerWsTests::nonTxBinaryStreamIgnored()
{
    m_client.sendTextMessage(QStringLiteral("TRX:0,true;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::MOX; }, 3000));

    StreamHeader hdr;
    hdr.streamType = kIqStreamType;
    hdr.format = 3;
    hdr.channels = 2;
    hdr.length = 2;
    hdr.sampleRate = 96'000;
    QByteArray frame = buildStreamHeader(hdr);
    const float iq[] = {0.1f, 0.2f, 0.3f, 0.4f};
    frame.append(reinterpret_cast<const char *>(iq), sizeof(iq));
    m_client.sendBinaryMessage(frame);

    QTest::qWait(200);
    QCOMPARE(m_txQueue.count(), 0);
}

void TciServerWsTests::remoteControlChangedOnConnectDisconnect()
{
    QSignalSpy spy(m_server, &TciServer::remoteControlChanged);
    QVERIFY(spy.isValid());

    QWebSocket extra;
    extra.open(QUrl(QStringLiteral("ws://127.0.0.1:%1").arg(m_server->port())));
    QVERIFY(QTest::qWaitFor([&extra]() { return extra.state() == QAbstractSocket::ConnectedState; }, 3000));

    QCOMPARE(spy.count(), 0);

    m_client.close();
    extra.close();
    QVERIFY(QTest::qWaitFor([&extra]() { return extra.state() == QAbstractSocket::UnconnectedState; }, 3000));

    QVERIFY(spy.count() >= 1);
    QCOMPARE(spy.last().at(0).toBool(), false);
}

void TciServerWsTests::driveCommandClampsLevel()
{
    m_client.sendTextMessage(QStringLiteral("DRIVE:0,150;"));
    QTest::qWait(100);
    QCOMPARE(m_settings->getDriveLevel(), 100);

    m_client.sendTextMessage(QStringLiteral("DRIVE:0,-5;"));
    QTest::qWait(100);
    QCOMPARE(m_settings->getDriveLevel(), 0);

    m_client.sendTextMessage(QStringLiteral("DRIVE:0,42;"));
    QTest::qWait(100);
    QCOMPARE(m_settings->getDriveLevel(), 42);
}

void TciServerWsTests::tuneCommandSetsTuneState()
{
    m_client.sendTextMessage(QStringLiteral("TUNE:0,true;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::TUNE; }, 3000));

    m_client.sendTextMessage(QStringLiteral("TUNE:0,false;"));
    QVERIFY(QTest::qWaitFor([this]() { return m_settings->getRadioState() == RadioState::RX; }, 3000));
}

void TciServerWsTests::audioStartStopGatesRxBinaryStream()
{
    QSignalSpy binarySpy(&m_client, &QWebSocket::binaryMessageReceived);
    QVERIFY(binarySpy.isValid());

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("AUDIO_START:0,0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("audio_start:")));
    QVERIFY(waitForMessageContaining(QStringLiteral("audio_samplerate:")));

    const QVector<float> stereo = {0.25f, 0.0f, -0.25f, 0.0f};
    m_server->onRxAudioSamples(0, stereo, 48'000);
    QVERIFY(QTest::qWaitFor([&binarySpy]() { return binarySpy.count() > 0; }, 3000));

    const QByteArray frame = binarySpy.last().at(0).toByteArray();
    StreamHeader hdr;
    QVERIFY(parseStreamHeader(frame, hdr));
    QCOMPARE(hdr.streamType, kRxAudioStreamType);

    binarySpy.clear();
    m_client.sendTextMessage(QStringLiteral("AUDIO_STOP:0,0;"));
    QTest::qWait(100);
    m_server->onRxAudioSamples(0, stereo, 48'000);
    QTest::qWait(200);
    QCOMPARE(binarySpy.count(), 0);
}

void TciServerWsTests::compatStubCommandsAccepted()
{
    // Non-WSJT ExpertSDR clients query/set these during handshake; stubs must
    // reply (or silently accept) so the connection is not filled with warnings.
    m_settings->setDriveLevel(42);
    m_textMessages.clear();

    m_client.sendTextMessage(QStringLiteral("sql_enable:0;"));
    QVERIFY2(waitForMessageContaining(QStringLiteral("sql_enable:0,false;")),
             qPrintable(m_textMessages.join('|')));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("sql_level:0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("sql_level:0,0;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("rx_anf_enable:0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("rx_anf_enable:0,false;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("mon_enable:0,false;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("mon_enable:0,false;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("rx_volume:0,0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("rx_volume:0,0;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("tune_drive:0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("tune_drive:0,42;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("rx_nb_enable:0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("rx_nb_enable:0,false;")));
    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("rx_nb2_enable:0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("rx_nb2_enable:0,false;")));
    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("rx_bin_enable:0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("rx_bin_enable:0,false;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("agc_auto_ex:0;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("agc_auto_ex:0,true;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("tx_profiles_ex;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("tx_profiles_ex:Default;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("tx_profile_ex;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("tx_profile_ex:Default;")));

    m_textMessages.clear();
    m_client.sendTextMessage(QStringLiteral("tx_stream_audio_buffering:50;"));
    QVERIFY(waitForMessageContaining(QStringLiteral("tx_stream_audio_buffering:50;")));

    // Silent accept (same as rx_sensors_enable).
    m_client.sendTextMessage(QStringLiteral("tx_sensors_enable:true;"));
    QTest::qWait(100);
}

QTEST_MAIN(TciServerWsTests)
#include "tci_server_ws_tests.moc"
