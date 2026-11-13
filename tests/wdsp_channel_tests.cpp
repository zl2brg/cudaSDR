#include <QtTest>
#include <QElapsedTimer>
#include <atomic>
#include <cmath>
#include <functional>
#include <thread>
#include "QtWDSP/WdspRxChannel.h"
#include "QtWDSP/WdspTxChannel.h"

namespace {
constexpr int TX_ID = 10;
constexpr int kBlock = 1024;
constexpr int kBlockMs = 21; // 1024 samples at 48 kHz

double blockPeak(const QVector<double> &iq)
{
    double peak = 0.0;
    for (double v : iq)
        peak = std::max(peak, std::fabs(v));
    return peak;
}

// Stands in for DataProcessor::get_tx_iqData on its own thread: one silent
// mic block per RX audio block, at real-time cadence.
class TxPump {
public:
    TxPump(WdspTxChannel &tx, std::function<bool()> keepPumping)
        : m_tx(tx), m_keepPumping(std::move(keepPumping)),
          m_thread([this] { run(); }) {}
    ~TxPump() { stop(); }

    void stop()
    {
        m_quit.store(true);
        if (m_thread.joinable())
            m_thread.join();
    }
    double lastPeak() const { return m_lastPeak.load(); }

private:
    void run()
    {
        QVector<double> mic(2 * kBlock, 0.0);
        QVector<double> iq(2 * kBlock, 0.0);
        while (!m_quit.load()) {
            if (m_keepPumping()) {
                int error = 0;
                m_tx.process(mic.data(), iq.data(), error);
                m_lastPeak.store(blockPeak(iq));
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(kBlockMs));
        }
    }

    WdspTxChannel &m_tx;
    std::function<bool()> m_keepPumping;
    std::atomic<bool> m_quit{false};
    std::atomic<double> m_lastPeak{0.0};
    std::thread m_thread;
};

void keyTune(WdspTxChannel &tx)
{
    tx.setPostGen(0, 1000.0, 0.99999, true);
    tx.setTxRun(true);
}
}

class WdspChannelTests : public QObject {
    Q_OBJECT

private slots:
    void testRxChannelLifecycle();
    void testRxChannelParameterSafetyBeforeOpen();
    void testTxChannelLifecycle();
    void testTxChannelParameterSafetyBeforeOpen();
    void testMultiChannelCoexistence();
    void testStaticHelpers();
    void testCfirOffForProtocol1OnForProtocol2();
    void testTxProcessZeroesOutputWhenStopped();
    void testTxUnkeyDrainsWhenPumpContinues();
    void testTxUnkeyTimesOutWhenPumpStops();
    void testTuneAfterStopWithoutPumpIsNotSilent();
};

void WdspChannelTests::testRxChannelLifecycle()
{
    WdspRxChannel rx(0);
    QVERIFY(!rx.isOpen());
    QVERIFY(!rx.isRunning());

    // Open channel
    bool ok = rx.open(1024, 48000, 48000, 48000, USB);
    QVERIFY(ok);
    QVERIFY(rx.isOpen());
    QVERIFY(rx.isRunning());
    QCOMPARE(rx.bufferSize(), 1024);
    QCOMPARE(rx.inputSampleRate(), 48000);
    QCOMPARE(rx.mode(), USB);

    // Process a block of samples
    CPX in(1024);
    CPX out(1024);
    InitCPX(in, 1024, 0.05);
    InitCPX(out, 1024, 0.0);
    rx.process(in, out);

    // Parameter mutations on live channel
    rx.setFilter(300.0, 2700.0);
    rx.setMode(LSB);
    QCOMPARE(rx.mode(), LSB);
    rx.setAgcMode(agcFAST);
    QCOMPARE(rx.agcMode(), agcFAST);
    rx.setVolume(75.0f);

    // Meters
    double inst = rx.getSMeterInstValue();
    double peak = rx.getSMeterPeakValue();
    QVERIFY(!std::isnan(inst));
    QVERIFY(!std::isnan(peak));

    // Reconfiguration
    bool reconfigOk = rx.reconfigure(1024, 96000, 48000, 48000, CWU);
    QVERIFY(reconfigOk);
    QVERIFY(rx.isOpen());
    QCOMPARE(rx.inputSampleRate(), 96000);
    QCOMPARE(rx.mode(), CWU);

    // Close channel
    rx.close();
    QVERIFY(!rx.isOpen());
    QVERIFY(!rx.isRunning());

    // Idempotent double close
    rx.close();
    QVERIFY(!rx.isOpen());
}

void WdspChannelTests::testRxChannelParameterSafetyBeforeOpen()
{
    WdspRxChannel rx(1);
    QVERIFY(!rx.isOpen());

    // None of these should crash even when channel is closed
    rx.setFilter(100.0, 3000.0);
    rx.setFilterSlope(1);
    rx.setNcoFrequency(5000);
    rx.setMode(AM);
    rx.setVolume(20.0f);
    rx.setFmSquelch(true, 50.0);
    rx.setAgcMode(agcSLOW);
    rx.setAgcAttack(5);
    rx.setAgcDecay(100);
    rx.setNoiseBlankerMode(1);
    rx.setNoiseFilterMode(2);
    rx.setAnf(true);
    rx.setSnb(true);

    CPX in(512);
    CPX out(512);
    InitCPX(in, 512, 0.1);
    InitCPX(out, 512, 1.0);
    rx.process(in, out);
    // Out buffer should be zeroed when closed
    QCOMPARE(out[0].re, 0.0);
    QCOMPARE(out[0].im, 0.0);

    QCOMPARE(rx.getSMeterInstValue(), -140.0);
    QCOMPARE(rx.getSMeterPeakValue(), -140.0);
}

void WdspChannelTests::testTxChannelLifecycle()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(!tx.isOpen());
    QVERIFY(!tx.isRunning());

    // Open TX channel
    bool ok = tx.open(1024, 2048, 48000, 48000, 48000, 0, false);
    QVERIFY(ok);
    QVERIFY(tx.isOpen());
    QCOMPARE(tx.bufferSize(), 1024);
    QCOMPARE(tx.micSampleRate(), 48000);

    // Process TX audio
    QVector<double> audioIn(1024, 0.02);
    QVector<double> iqOut(2048, 0.0);
    int error = 0;
    tx.process(audioIn.data(), iqOut.data(), error);
    tx.pushSpectrum(iqOut.data());

    // Test tone generation
    tx.setPostGen(0, 1000.0, 0.5, true);
    tx.setTxRun(true);
    QVERIFY(tx.isRunning());
    tx.setTxRun(false);
    QVERIFY(!tx.isRunning());

    // Processing controls
    tx.setMicGain(1.5);
    tx.setAudioCompression(10, true);
    tx.setLeveler(true, 1.0, 500.0, 1.0);
    tx.setFmDeviation(3000.0);
    tx.setAmCarrierLevel(0.8);
    tx.setCtcss(100.0, true);
    tx.setMode(USB);
    QCOMPARE(tx.mode(), USB);

    // Close
    tx.close();
    QVERIFY(!tx.isOpen());
    QVERIFY(!tx.isRunning());

    // Double close
    tx.close();
    QVERIFY(!tx.isOpen());
}

void WdspChannelTests::testTxChannelParameterSafetyBeforeOpen()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(!tx.isOpen());

    // None of these should crash when closed
    tx.setMode(LSB);
    tx.setFilter(150.0, 2800.0);
    tx.setMicGain(2.0);
    tx.setAudioCompression(15, true);
    tx.setFmDeviation(5000.0);
    tx.setAmCarrierLevel(0.5);
    tx.setCtcss(67.0, true);
    tx.setPostGen(0, 1000.0, 0.5, true);

    QVector<double> audioIn(1024, 0.1);
    QVector<double> iqOut(2048, 1.0);
    int error = 0;
    tx.process(audioIn.data(), iqOut.data(), error);
    QCOMPARE(error, -1);
}

void WdspChannelTests::testMultiChannelCoexistence()
{
    WdspRxChannel rx0(0);
    WdspRxChannel rx1(1);
    WdspTxChannel tx(TX_ID);

    QVERIFY(rx0.open(1024, 48000, 48000, 48000, USB));
    QVERIFY(rx1.open(1024, 96000, 48000, 48000, LSB));
    QVERIFY(tx.open(1024, 2048, 48000, 48000, 48000, 0, false));

    QVERIFY(rx0.isOpen());
    QVERIFY(rx1.isOpen());
    QVERIFY(tx.isOpen());

    // Close in reverse order
    tx.close();
    rx1.close();
    rx0.close();

    QVERIFY(!tx.isOpen());
    QVERIFY(!rx1.isOpen());
    QVERIFY(!rx0.isOpen());
}

void WdspChannelTests::testStaticHelpers()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(tx.open(1024, 2048, 48000, 48000, 48000, 0, false));

    // Test static draw methods
    QVector<double> X(1024, 0.0);
    QVector<double> Y(1024, 0.0);
    QVERIFY(WdspTxChannel::drawEq(TX_ID, X.data(), Y.data()));
    QVERIFY(WdspTxChannel::drawCfcompComp(TX_ID, X.data(), Y.data()));
    QVERIFY(WdspTxChannel::drawCfcompPeq(TX_ID, X.data(), Y.data()));

    // Phase rotator auto reset
    tx.resetPhaseRotatorAuto();

    // Test spectrum pixels
    QVector<float> pixels(4096, 0.0f);
    int ready = 0;
    WdspTxChannel::getSpectrumPixels(TX_ID, pixels.data(), ready);

    // Static setChannelState
    WdspChannel::setChannelStateById(TX_ID, 0, 0);

    tx.close();
}

void WdspChannelTests::testCfirOffForProtocol1OnForProtocol2()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(tx.open(1024, 2048, 48000, 48000, 48000, 0, false));
    QVERIFY(!tx.cfirRun());
    tx.close();

    QVERIFY(tx.open(1024, 2048, 48000, 96000, 192000, 1, false));
    QVERIFY(tx.cfirRun());
    tx.close();
}

void WdspChannelTests::testTxProcessZeroesOutputWhenStopped()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(tx.open(kBlock, 2048, 48000, 48000, 48000, 0, false));
    QVector<double> mic(2 * kBlock, 0.1);
    QVector<double> iq(2 * kBlock, 1.0);
    int error = 0;
    tx.process(mic.data(), iq.data(), error);
    QCOMPARE(blockPeak(iq), 0.0);
    tx.close();
}

// Settings flips to RX before Transmitter::setTxRun(false); the DataProcessor
// keeps pumping while the channel still reports running, so the ramp-down
// completes and the GUI thread is released well before WDSP's 100 ms timeout.
void WdspChannelTests::testTxUnkeyDrainsWhenPumpContinues()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(tx.open(kBlock, 2048, 48000, 48000, 48000, 0, false));

    for (int cycle = 0; cycle < 5; ++cycle) {
        std::atomic<bool> transmitting{true};
        keyTune(tx);
        TxPump pump(tx, [&] { return transmitting.load() || tx.isRunning(); });
        QTest::qWait(250);
        QVERIFY2(pump.lastPeak() > 0.3, qPrintable(QString::number(pump.lastPeak())));

        transmitting.store(false);
        QElapsedTimer t;
        t.start();
        tx.setPostGen(0, 0.0, 0.0, false);
        tx.setTxRun(false);
        const qint64 ms = t.elapsed();
        qInfo() << "cycle" << cycle << "unkey with pump took" << ms << "ms";
        QVERIFY(!tx.isRunning());
        QVERIFY2(ms < 80, qPrintable(QString("unkey took %1 ms").arg(ms)));
    }
    tx.close();
}

// The old P1 behaviour: pumping stops as soon as Settings is RX, so every
// unkey sits in SetChannelState's 100 ms poll on the GUI thread.
void WdspChannelTests::testTxUnkeyTimesOutWhenPumpStops()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(tx.open(kBlock, 2048, 48000, 48000, 48000, 0, false));

    std::atomic<bool> transmitting{true};
    keyTune(tx);
    TxPump pump(tx, [&] { return transmitting.load(); });
    QTest::qWait(250);

    transmitting.store(false);
    QTest::qWait(2 * kBlockMs);
    QElapsedTimer t;
    t.start();
    tx.setTxRun(false);
    const qint64 ms = t.elapsed();
    qInfo() << "unkey without pump took" << ms << "ms";
    QVERIFY(ms >= 90);
    tx.close();
}

// Engine Stop tears the pump down first. WdspTxChannel::stopChannel must not
// leave a pending ramp-down, or the next TUNE ramps itself straight back off.
void WdspChannelTests::testTuneAfterStopWithoutPumpIsNotSilent()
{
    WdspTxChannel tx(TX_ID);
    QVERIFY(tx.open(kBlock, 2048, 48000, 48000, 48000, 0, false));

    {
        std::atomic<bool> transmitting{true};
        keyTune(tx);
        TxPump pump(tx, [&] { return transmitting.load(); });
        QTest::qWait(250);
        QVERIFY(pump.lastPeak() > 0.3);
        transmitting.store(false);
    }
    tx.stopChannel();
    QVERIFY(!tx.isRunning());

    std::atomic<bool> transmitting{true};
    keyTune(tx);
    TxPump pump(tx, [&] { return transmitting.load() || tx.isRunning(); });
    QTest::qWait(300);
    QVERIFY2(pump.lastPeak() > 0.3,
             qPrintable(QString("TUNE after stop peak %1").arg(pump.lastPeak())));
    transmitting.store(false);
    tx.setTxRun(false);
    pump.stop();
    tx.close();
}

QTEST_MAIN(WdspChannelTests)
#include "wdsp_channel_tests.moc"
