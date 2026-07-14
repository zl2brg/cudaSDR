#include <QtTest/QtTest>

#include "Util/device_identity.h"

class DeviceIdentityTests : public QObject {
    Q_OBJECT

private slots:
    void soapyMatchByDriverAndSerial();
    void soapyMatchFallbackWithoutSerial();
    void findLastConnectedPrefersMatchingSoapy();
    void hpsdrMatchByMac();
    void matchesLastConnectedHpsdr();
    void findLastConnectedPrefersMatchingHpsdr();
};

void DeviceIdentityTests::soapyMatchByDriverAndSerial() {
    TSoapyDevice a;
    a.driver = "lime";
    a.serial = "ABC123";
    a.label = "ignored";

    TSoapyDevice b;
    b.driver = "lime";
    b.serial = "ABC123";
    b.label = "different";

    QVERIFY(sameSoapyDevice(a, b));
}

void DeviceIdentityTests::soapyMatchFallbackWithoutSerial() {
    TSoapyDevice a;
    a.driver = "rtlsdr";
    a.hardware = "RTL2832U";
    a.name = "Generic RTL";
    a.label = "RTL #1";
    a.args.insert("driver", "rtlsdr");

    TSoapyDevice b = a;
    QVERIFY(sameSoapyDevice(a, b));

    b.label = "RTL #2";
    QVERIFY(!sameSoapyDevice(a, b));
}

void DeviceIdentityTests::findLastConnectedPrefersMatchingSoapy() {
    QList<QVariant> discovered;

    TSoapyDevice wrong;
    wrong.driver = "lime";
    wrong.serial = "WRONG";
    discovered.append(QVariant::fromValue(wrong));

    TNetworkDevicecard hpsdr {};
    qstrncpy(hpsdr.mac_address, "00:11:22:33:44:55", sizeof(hpsdr.mac_address));
    discovered.append(QVariant::fromValue(hpsdr));

    TSoapyDevice target;
    target.driver = "lime";
    target.serial = "TARGET";
    discovered.append(QVariant::fromValue(target));

    TSDRDevice last {};
    last.deviceClass = DeviceClass_SoapySDR;
    last.deviceType = "lime";
    last.serialNumber = "TARGET";

    const QVariant picked = findLastConnectedMatch(discovered, last);
    QVERIFY(picked.isValid());
    QVERIFY(picked.canConvert<TSoapyDevice>());
    QCOMPARE(picked.value<TSoapyDevice>().serial, QString("TARGET"));
}

void DeviceIdentityTests::hpsdrMatchByMac() {
    TNetworkDevicecard a {};
    qstrncpy(a.mac_address, "AA:BB:CC:DD:EE:FF", sizeof(a.mac_address));
    TNetworkDevicecard b {};
    qstrncpy(b.mac_address, "AA:BB:CC:DD:EE:FF", sizeof(b.mac_address));
    TNetworkDevicecard c {};
    qstrncpy(c.mac_address, "00:11:22:33:44:55", sizeof(c.mac_address));

    QVERIFY(sameHpsdrDeviceByMac(a, b));
    QVERIFY(!sameHpsdrDeviceByMac(a, c));
}

void DeviceIdentityTests::matchesLastConnectedHpsdr() {
    TNetworkDevicecard card {};
    qstrncpy(card.mac_address, "DE:AD:BE:EF:00:01", sizeof(card.mac_address));

    TSDRDevice last {};
    last.deviceClass = DeviceClass_HPSDR;
    last.serialNumber = QStringLiteral("DE:AD:BE:EF:00:01");

    QVERIFY(matchesLastConnected(QVariant::fromValue(card), last));

    last.serialNumber = QStringLiteral("OTHER");
    QVERIFY(!matchesLastConnected(QVariant::fromValue(card), last));
}

void DeviceIdentityTests::findLastConnectedPrefersMatchingHpsdr() {
    QList<QVariant> discovered;

    TSoapyDevice soapy {};
    soapy.driver = "lime";
    soapy.serial = "IGNORE";
    discovered.append(QVariant::fromValue(soapy));

    TNetworkDevicecard target {};
    qstrncpy(target.mac_address, "11:22:33:44:55:66", sizeof(target.mac_address));
    discovered.append(QVariant::fromValue(target));

    TSDRDevice last {};
    last.deviceClass = DeviceClass_HPSDR;
    last.serialNumber = QStringLiteral("11:22:33:44:55:66");

    const QVariant picked = findLastConnectedMatch(discovered, last);
    QVERIFY(picked.isValid());
    QVERIFY(picked.canConvert<TNetworkDevicecard>());
    QCOMPARE(QString::fromLatin1(picked.value<TNetworkDevicecard>().mac_address),
             QStringLiteral("11:22:33:44:55:66"));
}

QTEST_APPLESS_MAIN(DeviceIdentityTests)
#include "device_identity_tests.moc"
