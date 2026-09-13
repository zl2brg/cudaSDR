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
    void testSimulatedDeviceStartPushesIq();
    void testSimulatedDeviceSampleRateAndFreq();
    void testSimulatedDeviceGainsAndPtt();
    void testSyntheticSampleGeneration();
    void testTxIqHandling();
    void testPolymorphicInterface();
    void testRxIqIngestAndCallback();
    void testNotifyRxIqAndBufferedRead();
    void testDecoupledMultiRxIngest();
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

void SdrDeviceTests::testSimulatedDeviceStartPushesIq()
{
    SimulatedDevice sim;
    int callbackCount = 0;
    int callbackSamples = 0;
    sim.setRxIqCallback([&](int rx, const float* data, int count) {
        Q_UNUSED(rx)
        QVERIFY(data != nullptr);
        callbackCount++;
        callbackSamples = count;
    });

    QVERIFY(sim.start());
    QVERIFY(QTest::qWaitFor([&]() { return callbackCount > 0; }, 500));
    QVERIFY(callbackCount >= 1);
    QCOMPARE(callbackSamples, 1024);
    sim.stop();
    const int afterStop = callbackCount;
    QTest::qWait(50);
    QCOMPARE(callbackCount, afterStop);
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
    QCOMPARE(callbackCount, 0);

    std::vector<float> ingest(64 * 2, 0.4f);
    dev->notifyRxIq(0, ingest.data(), 64);
    QCOMPARE(callbackCount, 1);
    QCOMPARE(callbackRx, 0);
    QCOMPARE(callbackSamples, 64);

    // Push mode does not buffer; synthetic pull must not fire the callback
    QCOMPARE(dev->readRxIq(0, rxBuf.data(), 32), 32);
    QCOMPARE(callbackCount, 1);

    QCOMPARE(dev->readRxIq(0, nullptr, 100), 0);
    QCOMPARE(dev->readRxIq(0, rxBuf.data(), 0), 0);
    QCOMPARE(dev->readRxIq(0, rxBuf.data(), -10), 0);
}

void SdrDeviceTests::testNotifyRxIqAndBufferedRead()
{
    SimulatedDevice sim;

    const int sampleCount = 64;
    std::vector<float> inputIq(sampleCount * 2);
    for (int i = 0; i < sampleCount; ++i) {
        inputIq[2 * i]     = 0.25f * static_cast<float>(i);
        inputIq[2 * i + 1] = -0.25f * static_cast<float>(i);
    }

    sim.notifyRxIq(0, inputIq.data(), sampleCount);

    std::vector<float> readBuf(sampleCount * 2, 0.0f);
    int read = sim.readRxIq(0, readBuf.data(), sampleCount);
    QCOMPARE(read, sampleCount);
    for (int i = 0; i < sampleCount * 2; ++i) {
        QCOMPARE(readBuf[i], inputIq[i]);
    }

    std::vector<float> synthBuf(32 * 2, 0.0f);
    int synthRead = sim.readRxIq(0, synthBuf.data(), 32);
    QCOMPARE(synthRead, 32);

    int callbackCount = 0;
    sim.setRxIqCallback([&](int, const float*, int) { callbackCount++; });
    sim.notifyRxIq(0, inputIq.data(), sampleCount);
    QCOMPARE(callbackCount, 1);
    QCOMPARE(sim.readRxIq(0, readBuf.data(), sampleCount), sampleCount);
    QCOMPARE(callbackCount, 1);

    sim.notifyRxIq(0, nullptr, 64);
    sim.notifyRxIq(0, inputIq.data(), 0);
    sim.notifyRxIq(0, inputIq.data(), -10);
    QCOMPARE(callbackCount, 1);
}

void SdrDeviceTests::testDecoupledMultiRxIngest()
{
    SimulatedDevice sim;

    // Simulated per-receiver ingestion queues (emulating SliceProcessor::enqueueRxIq)
    std::map<int, std::vector<float>> receiverQueues;

    sim.setRxIqCallback([&](int rx, const float* interleavedIq, int numComplexSamples) {
        if (!interleavedIq || numComplexSamples <= 0) return;
        auto& q = receiverQueues[rx];
        q.insert(q.end(), interleavedIq, interleavedIq + numComplexSamples * 2);
    });

    // Generate test data for 3 independent receiver slices
    constexpr int kSamples = 128;
    std::vector<float> rx0Data(kSamples * 2);
    std::vector<float> rx1Data(kSamples * 2);
    std::vector<float> rx2Data(kSamples * 2);

    for (int i = 0; i < kSamples; ++i) {
        rx0Data[2 * i]     = static_cast<float>(i) * 0.01f;
        rx0Data[2 * i + 1] = static_cast<float>(-i) * 0.01f;

        rx1Data[2 * i]     = 10.0f + static_cast<float>(i) * 0.05f;
        rx1Data[2 * i + 1] = 20.0f + static_cast<float>(i) * 0.05f;

        rx2Data[2 * i]     = -50.0f + static_cast<float>(i) * 0.1f;
        rx2Data[2 * i + 1] = -100.0f + static_cast<float>(i) * 0.1f;
    }

    // Ingest data into independent receivers via polymorphic HAL
    ISdrDevice* dev = &sim;
    dev->notifyRxIq(0, rx0Data.data(), kSamples);
    dev->notifyRxIq(1, rx1Data.data(), kSamples);
    dev->notifyRxIq(2, rx2Data.data(), kSamples);

    // Verify channel separation and exact sample fidelity
    QCOMPARE(receiverQueues[0].size(), static_cast<size_t>(kSamples * 2));
    QCOMPARE(receiverQueues[1].size(), static_cast<size_t>(kSamples * 2));
    QCOMPARE(receiverQueues[2].size(), static_cast<size_t>(kSamples * 2));

    for (int i = 0; i < kSamples * 2; ++i) {
        QCOMPARE(receiverQueues[0][i], rx0Data[i]);
        QCOMPARE(receiverQueues[1][i], rx1Data[i]);
        QCOMPARE(receiverQueues[2][i], rx2Data[i]);
    }

    // Verify 24-bit full-scale normalization mapping
    const double scale = 1.0 / 8388607.0;
    int32_t maxPos = 8388607;
    int32_t maxNeg = -8388607;
    float maxPosFloat = static_cast<float>(maxPos * scale);
    float maxNegFloat = static_cast<float>(maxNeg * scale);
    QVERIFY(std::abs(maxPosFloat - 1.0f) < 1e-6f);
    QVERIFY(std::abs(maxNegFloat - (-1.0f)) < 1e-6f);
}

QTEST_MAIN(SdrDeviceTests)
#include "sdr_device_tests.moc"
