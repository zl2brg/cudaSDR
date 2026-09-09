#include <QtTest>
#include <QSignalSpy>
#include <memory>
#include "DataEngine/SdrDeviceManager.h"
#include "DataEngine/Drivers/SimulatedDevice.h"
#include "DataEngine/Drivers/HpsdrDevice.h"
#include "DataEngine/Drivers/SoapyDevice.h"
#include "cusdr_settings.h"
#include "Settings/SoapyConfig.h"
#include "DataEngine/cusdr_dataIO.h"

void DataIO::networkDeviceStartStop(char) {}

#ifdef HAVE_SOAPYSDR
#include "DataEngine/SoapySDRDataSource.h"
void SoapySDRDataSource::setSampleRate(int) {}
void SoapySDRDataSource::setFrequency(int, qint64) {}
#endif

class SdrDeviceManagerTests : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void testSingleton();
    void testRegisterAndQuery();
    void testDevicesByType();
    void testNetworkCardConversion();
    void testSoapyDeviceConversion();
    void testSimulatedDeviceFactory();
    void testDiscoverySignals();
    void testClearAndUnregister();
    void testHpsdrDeviceRxIngest();
    void testSoapyDeviceRxIngest();
    void testDeviceSelection();
    void testUnifiedAsyncDiscovery();
    void testFindNetworkCardAndSoapyDevice();
};

void SdrDeviceManagerTests::init()
{
    SdrDeviceManager::instance()->clearDevices();
}

void SdrDeviceManagerTests::cleanup()
{
    SdrDeviceManager::instance()->clearDevices();
}

void SdrDeviceManagerTests::testSingleton()
{
    SdrDeviceManager* mgr1 = SdrDeviceManager::instance();
    SdrDeviceManager* mgr2 = SdrDeviceManager::instance();
    QVERIFY(mgr1 != nullptr);
    QCOMPARE(mgr1, mgr2);
}

void SdrDeviceManagerTests::testRegisterAndQuery()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();
    QCOMPARE(mgr->deviceCount(), 0);

    SdrDeviceInfo dev1;
    dev1.id = QStringLiteral("00:01:02:03:04:05");
    dev1.name = QStringLiteral("Hermes SDR");
    dev1.type = DeviceType::HpsdrP1;
    dev1.address = QStringLiteral("192.168.1.100");
    dev1.port = 1024;

    QSignalSpy spyDiscovered(mgr, &SdrDeviceManager::deviceDiscovered);
    QSignalSpy spyChanged(mgr, &SdrDeviceManager::deviceListChanged);

    mgr->registerDevice(dev1);

    QCOMPARE(mgr->deviceCount(), 1);
    QVERIFY(mgr->hasDevice(dev1.id));
    QCOMPARE(spyDiscovered.count(), 1);
    QCOMPARE(spyChanged.count(), 1);

    SdrDeviceInfo fetched = mgr->deviceById(dev1.id);
    QCOMPARE(fetched.id, dev1.id);
    QCOMPARE(fetched.name, dev1.name);
    QCOMPARE(fetched.type, DeviceType::HpsdrP1);
    QCOMPARE(fetched.address, dev1.address);
    QCOMPARE(fetched.port, static_cast<quint16>(1024));

    // Update existing device
    dev1.name = QStringLiteral("Hermes Transceiver");
    mgr->registerDevice(dev1);
    QCOMPARE(mgr->deviceCount(), 1);
    QCOMPARE(mgr->deviceById(dev1.id).name, QStringLiteral("Hermes Transceiver"));

    // Invalid device ignored
    SdrDeviceInfo invalid;
    mgr->registerDevice(invalid);
    QCOMPARE(mgr->deviceCount(), 1);
}

void SdrDeviceManagerTests::testDevicesByType()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();

    SdrDeviceInfo h1;
    h1.id = QStringLiteral("p1:1");
    h1.name = QStringLiteral("HPSDR P1");
    h1.type = DeviceType::HpsdrP1;

    SdrDeviceInfo h2;
    h2.id = QStringLiteral("p2:1");
    h2.name = QStringLiteral("HPSDR P2");
    h2.type = DeviceType::HpsdrP2;

    SdrDeviceInfo s1;
    s1.id = QStringLiteral("soapy:lime");
    s1.name = QStringLiteral("LimeSDR");
    s1.type = DeviceType::SoapySDR;

    mgr->registerDevice(h1);
    mgr->registerDevice(h2);
    mgr->registerDevice(s1);
    mgr->registerDevice(SdrDeviceManager::simulatedDeviceInfo());

    QCOMPARE(mgr->deviceCount(), 4);
    QCOMPARE(mgr->devicesByType(DeviceType::HpsdrP1).size(), 1);
    QCOMPARE(mgr->devicesByType(DeviceType::HpsdrP2).size(), 1);
    QCOMPARE(mgr->devicesByType(DeviceType::SoapySDR).size(), 1);
    QCOMPARE(mgr->devicesByType(DeviceType::Simulated).size(), 1);
}

void SdrDeviceManagerTests::testNetworkCardConversion()
{
    TNetworkDevicecard card;
    qstrncpy(card.mac_address, "00:11:22:33:44:55", sizeof(card.mac_address));
    card.ip_address = QHostAddress(QStringLiteral("192.168.2.50"));
    card.boardName = QStringLiteral("Hermes P2");
    card.protocol = 2;
    card.max_receivers = 7;
    card.frequency_min = 100000;
    card.frequency_max = 60000000;
    card.sw_version = 42;
    card.boardID = 2;

    SdrDeviceInfo info = SdrDeviceManager::fromNetworkCard(card);
    QCOMPARE(info.id, QStringLiteral("00:11:22:33:44:55"));
    QCOMPARE(info.name, QStringLiteral("Hermes P2"));
    QCOMPARE(info.type, DeviceType::HpsdrP2);
    QCOMPARE(info.address, QStringLiteral("192.168.2.50"));
    QCOMPARE(info.port, static_cast<quint16>(1024));
    QCOMPARE(info.capabilities.maxReceivers, 7);
    QVERIFY(info.capabilities.supportsTx);
    QVERIFY(info.capabilities.supportsFullDuplex);
    QCOMPARE(info.properties.value(QStringLiteral("sw_version")), QStringLiteral("42"));
}

void SdrDeviceManagerTests::testSoapyDeviceConversion()
{
    TSoapyDevice sdev;
    sdev.driver = QStringLiteral("lime");
    sdev.hardware = QStringLiteral("LimeSDR-USB");
    sdev.name = QStringLiteral("LimeSDR");
    sdev.serial = QStringLiteral("0009072C00000000");
    sdev.label = QStringLiteral("LimeSDR-USB [0009072C00000000]");
    sdev.args[QStringLiteral("channel")] = QStringLiteral("0");

    SdrDeviceInfo info = SdrDeviceManager::fromSoapyDevice(sdev);
    QCOMPARE(info.id, QStringLiteral("0009072C00000000"));
    QCOMPARE(info.name, QStringLiteral("LimeSDR-USB [0009072C00000000]"));
    QCOMPARE(info.type, DeviceType::SoapySDR);
    QCOMPARE(info.serialNumber, QStringLiteral("0009072C00000000"));
    QCOMPARE(info.properties.value(QStringLiteral("driver")), QStringLiteral("lime"));
    QCOMPARE(info.properties.value(QStringLiteral("channel")), QStringLiteral("0"));
    QVERIFY(info.capabilities.supportsTx);
}

void SdrDeviceManagerTests::testSimulatedDeviceFactory()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();

    std::unique_ptr<ISdrDevice> dev = mgr->createSimulatedDevice();
    QVERIFY(dev != nullptr);
    QCOMPARE(dev->deviceType(), DeviceType::Simulated);
    QCOMPARE(dev->deviceName(), QStringLiteral("Simulated SDR Device"));

    QVERIFY(dev->start());
    QVERIFY(dev->isRunning());

    // Test RX IQ via HAL interface
    bool rxCalled = false;
    dev->setRxIqCallback([&](int rx, const float* data, int count) {
        Q_UNUSED(data);
        QCOMPARE(rx, 0);
        QCOMPARE(count, 128);
        rxCalled = true;
    });

    std::vector<float> buf(128 * 2);
    int read = dev->readRxIq(0, buf.data(), 128);
    QCOMPARE(read, 128);
    QVERIFY(rxCalled);

    dev->stop();
    QVERIFY(!dev->isRunning());
}

void SdrDeviceManagerTests::testDiscoverySignals()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();

    QSignalSpy spyStarted(mgr, &SdrDeviceManager::discoveryStarted);
    QSignalSpy spyFinished(mgr, &SdrDeviceManager::discoveryFinished);

    mgr->startDiscovery();

    QCOMPARE(spyStarted.count(), 1);
    QCOMPARE(spyFinished.count(), 1);
    QVERIFY(!mgr->isDiscovering());
    QVERIFY(mgr->deviceCount() >= 1); // Simulated device registered
}

void SdrDeviceManagerTests::testClearAndUnregister()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();

    SdrDeviceInfo d1;
    d1.id = QStringLiteral("dev:1");
    d1.name = QStringLiteral("Dev 1");
    d1.type = DeviceType::Simulated;

    SdrDeviceInfo d2;
    d2.id = QStringLiteral("dev:2");
    d2.name = QStringLiteral("Dev 2");
    d2.type = DeviceType::Simulated;

    mgr->registerDevice(d1);
    mgr->registerDevice(d2);
    QCOMPARE(mgr->deviceCount(), 2);

    QSignalSpy spyRemoved(mgr, &SdrDeviceManager::deviceRemoved);
    mgr->unregisterDevice(d1.id);
    QCOMPARE(mgr->deviceCount(), 1);
    QCOMPARE(spyRemoved.count(), 1);
    QCOMPARE(spyRemoved.first().first().toString(), d1.id);

    mgr->clearDevices();
    QCOMPARE(mgr->deviceCount(), 0);
}

void SdrDeviceManagerTests::testHpsdrDeviceRxIngest()
{
    HpsdrDevice hpsdr(nullptr, nullptr, false);

    int callbackCount = 0;
    int callbackRx = -1;
    int callbackSamples = 0;

    hpsdr.setRxIqCallback([&](int rx, const float* data, int count) {
        callbackCount++;
        callbackRx = rx;
        callbackSamples = count;
        QVERIFY(data != nullptr);
    });

    const int count = 256;
    std::vector<float> inData(count * 2, 0.42f);

    // Ingest for RX 0
    hpsdr.notifyRxIq(0, inData.data(), count);
    QCOMPARE(callbackCount, 1);
    QCOMPARE(callbackRx, 0);
    QCOMPARE(callbackSamples, count);

    // Ingest for RX 1
    hpsdr.notifyRxIq(1, inData.data(), count);
    QCOMPARE(callbackCount, 2);
    QCOMPARE(callbackRx, 1);
    QCOMPARE(callbackSamples, count);

    // Read back RX 0 (triggers callback)
    std::vector<float> outData(count * 2, 0.0f);
    int read = hpsdr.readRxIq(0, outData.data(), count);
    QCOMPARE(read, count);
    QCOMPARE(callbackCount, 3);
    for (int i = 0; i < count * 2; ++i) {
        QCOMPARE(outData[i], 0.42f);
    }

    // Read back RX 1 (triggers callback)
    read = hpsdr.readRxIq(1, outData.data(), count);
    QCOMPARE(read, count);
    QCOMPARE(callbackCount, 4);
    for (int i = 0; i < count * 2; ++i) {
        QCOMPARE(outData[i], 0.42f);
    }

    // Second read on RX 0 returns 0 (drained, no callback)
    QCOMPARE(hpsdr.readRxIq(0, outData.data(), count), 0);
    QCOMPARE(callbackCount, 4);

    // Polymorphic interface verification
    ISdrDevice* sdr = &hpsdr;
    sdr->notifyRxIq(0, inData.data(), 128);
    QCOMPARE(callbackCount, 5);
    QCOMPARE(callbackSamples, 128);
    QCOMPARE(sdr->readRxIq(0, outData.data(), 128), 128);
    QCOMPARE(callbackCount, 6);
}

void SdrDeviceManagerTests::testSoapyDeviceRxIngest()
{
    SoapyDevice soapy(nullptr, nullptr);

    int callbackCount = 0;
    int callbackRx = -1;
    int callbackSamples = 0;

    soapy.setRxIqCallback([&](int rx, const float* data, int count) {
        callbackCount++;
        callbackRx = rx;
        callbackSamples = count;
        QVERIFY(data != nullptr);
    });

    const int count = 128;
    std::vector<float> inData(count * 2, -0.65f);

    soapy.notifyRxIq(0, inData.data(), count);
    QCOMPARE(callbackCount, 1);
    QCOMPARE(callbackRx, 0);
    QCOMPARE(callbackSamples, count);

    std::vector<float> outData(count * 2, 0.0f);
    int read = soapy.readRxIq(0, outData.data(), count);
    QCOMPARE(read, count);
    QCOMPARE(callbackCount, 2);
    for (int i = 0; i < count * 2; ++i) {
        QCOMPARE(outData[i], -0.65f);
    }

    // Second read: drained
    QCOMPARE(soapy.readRxIq(0, outData.data(), count), 0);
    QCOMPARE(callbackCount, 2);

    // Polymorphic interface verification
    ISdrDevice* sdr = &soapy;
    sdr->notifyRxIq(0, inData.data(), 64);
    QCOMPARE(callbackCount, 3);
    QCOMPARE(callbackSamples, 64);
    QCOMPARE(sdr->readRxIq(0, outData.data(), 64), 64);
    QCOMPARE(callbackCount, 4);
}

void SdrDeviceManagerTests::testDeviceSelection()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();

    SdrDeviceInfo d1;
    d1.id = QStringLiteral("00:11:22:33:44:55");
    d1.name = QStringLiteral("Hermes SDR");
    d1.type = DeviceType::HpsdrP1;

    SdrDeviceInfo d2;
    d2.id = QStringLiteral("lime:1234");
    d2.name = QStringLiteral("LimeSDR");
    d2.type = DeviceType::SoapySDR;

    mgr->registerDevice(d1);
    mgr->registerDevice(d2);

    QSignalSpy spySelected(mgr, &SdrDeviceManager::selectedDeviceChanged);

    // Initial default selected device is first available
    QCOMPARE(mgr->selectedDevice().id, d1.id);

    // Select LimeSDR
    mgr->selectDevice(d2.id);
    QCOMPARE(mgr->selectedDeviceId(), d2.id);
    QCOMPARE(mgr->selectedDevice().name, QStringLiteral("LimeSDR"));
    QCOMPARE(spySelected.count(), 1);

    // Invalid selection ignored
    mgr->selectDevice(QStringLiteral("invalid:none"));
    QCOMPARE(mgr->selectedDeviceId(), d2.id);
    QCOMPARE(spySelected.count(), 1);
}

void SdrDeviceManagerTests::testUnifiedAsyncDiscovery()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();

    QSignalSpy spyStarted(mgr, &SdrDeviceManager::discoveryStarted);
    QSignalSpy spyFinished(mgr, &SdrDeviceManager::discoveryFinished);

    mgr->startDiscovery(true);

    QCOMPARE(spyStarted.count(), 1);
    QVERIFY(mgr->isDiscovering());

    // Step 1: HPSDR finishes
    mgr->notifyDiscoveryStepFinished(QStringLiteral("HPSDR"), 1);
#ifdef HAVE_SOAPYSDR
    // Still discovering because Soapy is pending
    QVERIFY(mgr->isDiscovering());
    QCOMPARE(spyFinished.count(), 0);

    // Step 2: Soapy finishes
    mgr->notifyDiscoveryStepFinished(QStringLiteral("SoapySDR"), 2);
#endif
    QVERIFY(!mgr->isDiscovering());
    QCOMPARE(spyFinished.count(), 1);

    // Test stopDiscovery aborts cleanly
    mgr->startDiscovery(true);
    QVERIFY(mgr->isDiscovering());
    mgr->stopDiscovery();
    QVERIFY(!mgr->isDiscovering());
    QCOMPARE(spyFinished.count(), 2);
}

void SdrDeviceManagerTests::testFindNetworkCardAndSoapyDevice()
{
    SdrDeviceManager* mgr = SdrDeviceManager::instance();
    Settings* set = Settings::instance();

    TNetworkDevicecard card;
    qstrncpy(card.mac_address, "aa:bb:cc:dd:ee:ff", sizeof(card.mac_address));
    card.ip_address = QHostAddress(QStringLiteral("192.168.1.150"));
    card.boardName = QStringLiteral("Metis Card");
    card.protocol = 1;
    card.adcs = 1;
    card.dacs = 1;

    set->setMetisCardList({card});

    TNetworkDevicecard foundCard;
    QVERIFY(mgr->findNetworkCard(QStringLiteral("aa:bb:cc:dd:ee:ff"), foundCard));
    QCOMPARE(foundCard.boardName, QStringLiteral("Metis Card"));
    QCOMPARE(foundCard.ip_address, QHostAddress(QStringLiteral("192.168.1.150")));

    TNetworkDevicecard notFound;
    QVERIFY(!mgr->findNetworkCard(QStringLiteral("00:00:00:00:00:00"), notFound));

    // Test DataEngine binding
    QVERIFY(mgr->dataEngine() == nullptr);
}

QTEST_MAIN(SdrDeviceManagerTests)
#include "sdr_device_manager_tests.moc"
