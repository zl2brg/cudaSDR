#include <QtTest>
#include "QtWDSP/WdspRxChannel.h"
#include "QtWDSP/WdspTxChannel.h"

namespace {
constexpr int TX_ID = 10;
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

QTEST_MAIN(WdspChannelTests)
#include "wdsp_channel_tests.moc"
