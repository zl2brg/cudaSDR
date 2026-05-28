#include <QtTest/QtTest>

#include "Util/device_identity.h"

class DeviceIdentityTests : public QObject {
    Q_OBJECT

private slots:
    void soapyMatchByDriverAndSerial();
    void soapyMatchFallbackWithoutSerial();
    void findLastConnectedPrefersMatchingSoapy();
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

QTEST_APPLESS_MAIN(DeviceIdentityTests)
#include "device_identity_tests.moc"
