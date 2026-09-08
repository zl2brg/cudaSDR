#include <QtTest>
#include <memory>
#include <cmath>
#include "DataEngine/ISdrDevice.h"
#include "DataEngine/Drivers/SimulatedDevice.h"

class SdrDeviceTests : public QObject {
    Q_OBJECT

private slots:
    void testDeviceCapabilities();
    void testSimulatedDeviceLifecycle();
    void testSimulatedDeviceSampleRateAndFreq();
    void testSimulatedDeviceGainsAndPtt();
    void testSyntheticSampleGeneration();
    void testTxIqHandling();
    void testPolymorphicInterface();
    void testRxIqIngestAndCallback();
};

void SdrDeviceTests::testDeviceCapabilities()
{
    DeviceCapabilities caps;
    QCOMPARE(caps.supportsTx, false);
    QCOMPARE(caps.supportsFullDuplex, false);
    QCOMPARE(caps.supportsWideband, false);
    QCOMPARE(caps.maxReceivers, 1);
    QCOMPARE(caps.supportedSampleRates.size(), 1);
    QCOMPARE(caps.supportedSampleRates.first(), 48000);

    SimulatedDevice sim;
    DeviceCapabilities simCaps = sim.capabilities();
    QVERIFY(simCaps.supportsTx);
    QVERIFY(simCaps.supportsFullDuplex);
    QVERIFY(!simCaps.supportsWideband);
    QCOMPARE(simCaps.maxReceivers, 4);
    QVERIFY(simCaps.supportedSampleRates.contains(48000));
    QVERIFY(simCaps.supportedSampleRates.contains(96000));
    QVERIFY(simCaps.supportedSampleRates.contains(192000));
    QVERIFY(simCaps.supportedSampleRates.contains(384000));
    QCOMPARE(simCaps.minFrequencyHz, 100000LL);
    QCOMPARE(simCaps.maxFrequencyHz, 60000000LL);
}

void SdrDeviceTests::testSimulatedDeviceLifecycle()
{
    SimulatedDevice sim;
    QVERIFY(!sim.isRunning());

    QVERIFY(sim.start());
    QVERIFY(sim.isRunning());

    // Idempotent start
    QVERIFY(sim.start());
    QVERIFY(sim.isRunning());

    sim.stop();
    QVERIFY(!sim.isRunning());

    // Idempotent stop
    sim.stop();
    QVERIFY(!sim.isRunning());
}

void SdrDeviceTests::testSimulatedDeviceSampleRateAndFreq()
{
    SimulatedDevice sim;
    QCOMPARE(sim.sampleRate(), 48000);

    // Valid sample rates
    QVERIFY(sim.setSampleRate(96000));
    QCOMPARE(sim.sampleRate(), 96000);
    QVERIFY(sim.setSampleRate(192000));
    QCOMPARE(sim.sampleRate(), 192000);
    QVERIFY(sim.setSampleRate(384000));
    QCOMPARE(sim.sampleRate(), 384000);

    // Invalid sample rates
    QVERIFY(!sim.setSampleRate(0));
    QVERIFY(!sim.setSampleRate(-48000));
    QVERIFY(!sim.setSampleRate(12345));
    QCOMPARE(sim.sampleRate(), 384000); // Preserved

    // Frequency tuning
    QVERIFY(sim.setFrequency(0, 14200000LL));
    QCOMPARE(sim.frequency(0), 14200000LL);

    QVERIFY(sim.setFrequency(1, 7100000LL));
    QCOMPARE(sim.frequency(1), 7100000LL);

    // Invalid frequencies
    QVERIFY(!sim.setFrequency(0, -1LL));
    QVERIFY(!sim.setFrequency(0, 70000000LL)); // Exceeds maxFrequencyHz (60MHz)
    QCOMPARE(sim.frequency(0), 14200000LL); // Preserved

    // TX Frequency
    QVERIFY(sim.setTxFrequency(14150000LL));
    QCOMPARE(sim.txFrequency(), 14150000LL);
    QVERIFY(!sim.setTxFrequency(-1LL));
    QVERIFY(!sim.setTxFrequency(70000000LL));
    QCOMPARE(sim.txFrequency(), 14150000LL);
}

void SdrDeviceTests::testSimulatedDeviceGainsAndPtt()
{
    SimulatedDevice sim;

    QVERIFY(sim.setRxGain(0, 10.0));
    QVERIFY(sim.setRxGain(1, -6.0));
    QVERIFY(sim.setTxGain(25.0));

    QVERIFY(!sim.isPtt());
    QVERIFY(sim.setPtt(true));
    QVERIFY(sim.isPtt());
    QVERIFY(sim.setPtt(false));
    QVERIFY(!sim.isPtt());
}

void SdrDeviceTests::testSyntheticSampleGeneration()
{
    SimulatedDevice sim;
    sim.setToneOffsetHz(1000.0);
    sim.setToneAmplitude(0.5);
    sim.setNoiseFloor(0.0); // Pure tone for deterministic verification
    sim.setSampleRate(48000);

    const int sampleCount = 1024;
    std::vector<float> buffer(sampleCount * 2, 0.0f);

    sim.generateSamples(buffer.data(), sampleCount);

    double sumSq = 0.0;
    for (int i = 0; i < sampleCount; ++i) {
        float iSample = buffer[2 * i];
        float qSample = buffer[2 * i + 1];

        QVERIFY(std::isfinite(iSample));
        QVERIFY(std::isfinite(qSample));

        // Each component cannot exceed tone amplitude when noise is zero
        QVERIFY(std::abs(iSample) <= 0.5f + 1e-4f);
        QVERIFY(std::abs(qSample) <= 0.5f + 1e-4f);

        sumSq += (iSample * iSample + qSample * qSample);
    }

    // Complex power: |s(t)|^2 = (A*cos)^2 + (A*sin)^2 = A^2 = 0.25
    double meanPower = sumSq / sampleCount;
    QVERIFY(std::abs(meanPower - 0.25) < 1e-3);

    // Null buffer safety
    sim.generateSamples(nullptr, 100);
    sim.generateSamples(buffer.data(), 0);
    sim.generateSamples(buffer.data(), -10);
}

void SdrDeviceTests::testTxIqHandling()
{
    SimulatedDevice sim;
    QCOMPARE(sim.txSampleCount(), 0ULL);

    std::vector<float> txBuf(512 * 2, 0.1f);
    sim.sendTxIq(txBuf.data(), 512);
    QCOMPARE(sim.txSampleCount(), 512ULL);

    sim.sendTxIq(txBuf.data(), 512);
    QCOMPARE(sim.txSampleCount(), 1024ULL);

    // Edge cases
    sim.sendTxIq(nullptr, 100);
    QCOMPARE(sim.txSampleCount(), 1024ULL);
    sim.sendTxIq(txBuf.data(), 0);
    QCOMPARE(sim.txSampleCount(), 1024ULL);
    sim.sendTxIq(txBuf.data(), -5);
    QCOMPARE(sim.txSampleCount(), 1024ULL);
}

void SdrDeviceTests::testPolymorphicInterface()
{
    std::unique_ptr<ISdrDevice> dev = std::make_unique<SimulatedDevice>();
    QVERIFY(dev != nullptr);

    QCOMPARE(dev->deviceName(), QStringLiteral("Simulated SDR Device"));
    QCOMPARE(dev->deviceType(), DeviceType::Simulated);

    QVERIFY(!dev->isRunning());
    QVERIFY(dev->start());
    QVERIFY(dev->isRunning());

    QVERIFY(dev->setFrequency(0, 14074000LL));
    QCOMPARE(dev->frequency(0), 14074000LL);

    dev->stop();
    QVERIFY(!dev->isRunning());
}

void SdrDeviceTests::testRxIqIngestAndCallback()
{
    std::unique_ptr<ISdrDevice> dev = std::make_unique<SimulatedDevice>();
    QVERIFY(dev != nullptr);

    int callbackCount = 0;
    int callbackRx = -1;
    int callbackSamples = 0;

    dev->setRxIqCallback([&](int rx, const float* data, int count) {
        callbackCount++;
        callbackRx = rx;
        callbackSamples = count;
        QVERIFY(data != nullptr);
    });

    std::vector<float> rxBuf(256 * 2, 0.0f);
    int read = dev->readRxIq(0, rxBuf.data(), 256);
    QCOMPARE(read, 256);
    QCOMPARE(callbackCount, 1);
    QCOMPARE(callbackRx, 0);
    QCOMPARE(callbackSamples, 256);

    // Edge cases
    QCOMPARE(dev->readRxIq(0, nullptr, 100), 0);
    QCOMPARE(dev->readRxIq(0, rxBuf.data(), 0), 0);
    QCOMPARE(dev->readRxIq(0, rxBuf.data(), -10), 0);
}

QTEST_MAIN(SdrDeviceTests)
#include "sdr_device_tests.moc"
