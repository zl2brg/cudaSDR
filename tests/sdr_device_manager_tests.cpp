#include <QtTest>
#include <QSignalSpy>
#include <memory>
#include "DataEngine/SdrDeviceManager.h"
#include "DataEngine/Drivers/SimulatedDevice.h"
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

QTEST_MAIN(SdrDeviceManagerTests)
#include "sdr_device_manager_tests.moc"
