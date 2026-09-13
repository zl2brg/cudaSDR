#include <QtTest/QtTest>
#include <cstdio>

#include "Util/device_identity.h"
#include "DataEngine/protocol_boundary_utils.h"

class DeviceIdentityTests : public QObject {
    Q_OBJECT

private slots:
    void soapyMatchByDriverAndSerial();
    void soapyMatchFallbackWithoutSerial();
    void findLastConnectedPrefersMatchingSoapy();
    void hpsdrMatchByMac();
    void matchesLastConnectedHpsdr();
    void findLastConnectedPrefersMatchingHpsdr();
    void hpsdrProtocol1DeviceDecoding();
    void hpsdrProtocol2DeviceDecoding();
    void hpsdrHermesLite2Versioning();
    void anan10P1DiscoveryIsHermesNotLite();
    void anan10P1WinsOverBogusP2Lite();
    void hermes2IsAnan10eNotGriffin();
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
    QCOMPARE(hpsdr.adcs, 1);
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

void DeviceIdentityTests::hpsdrProtocol1DeviceDecoding() {
    using namespace ProtocolBoundaryUtils;

    // Metis (0)
    auto metis = decodeHpsdrDevice(0, 1);
    QCOMPARE(metis.deviceType, HpsdrDeviceType::Metis);
    QCOMPARE(metis.adcs, 1);
    QCOMPARE(metis.boardName, QString("Metis"));

    // Hermes (1) — ANAN-10 / ANAN-100. Firmware must never reclassify this as Lite.
    auto hermes = decodeHpsdrDevice(1, 1);
    QCOMPARE(hermes.deviceType, HpsdrDeviceType::Hermes);
    QCOMPARE(hermes.adcs, 1);
    QCOMPARE(hermes.boardName, QString("Hermes"));

    auto hermesFw6 = decodeHpsdrDevice(1, 1, 6);
    QCOMPARE(hermesFw6.deviceType, HpsdrDeviceType::Hermes);
    QVERIFY(!isHermesLiteDeviceType(hermesFw6.deviceType));

    auto hermesFw73 = decodeHpsdrDevice(1, 1, 73);
    QCOMPARE(hermesFw73.deviceType, HpsdrDeviceType::Hermes);
    QVERIFY(!isHermesLiteDeviceType(hermesFw73.deviceType));

    // Angelia (4)
    auto angelia = decodeHpsdrDevice(4, 1);
    QCOMPARE(angelia.deviceType, HpsdrDeviceType::Angelia);
    QCOMPARE(angelia.adcs, 2);
    QCOMPARE(angelia.boardName, QString("Angelia"));

    // Orion (5)
    auto orion = decodeHpsdrDevice(5, 1);
    QCOMPARE(orion.deviceType, HpsdrDeviceType::Orion);
    QCOMPARE(orion.adcs, 2);
    QCOMPARE(orion.boardName, QString("Orion"));

    // Orion2 (10)
    auto orion2 = decodeHpsdrDevice(10, 1);
    QCOMPARE(orion2.deviceType, HpsdrDeviceType::Orion2);
    QCOMPARE(orion2.adcs, 2);
    QCOMPARE(orion2.boardName, QString("Orion2"));

    // STEMlab (100) & STEMlab Z20 (101)
    auto stemlab = decodeHpsdrDevice(100, 1);
    QCOMPARE(stemlab.deviceType, HpsdrDeviceType::StemLab);
    QCOMPARE(stemlab.adcs, 2);

    auto stemlabZ20 = decodeHpsdrDevice(101, 1);
    QCOMPARE(stemlabZ20.deviceType, HpsdrDeviceType::StemLabZ20);
    QCOMPARE(stemlabZ20.adcs, 2);
}

void DeviceIdentityTests::hpsdrProtocol2DeviceDecoding() {
    using namespace ProtocolBoundaryUtils;

    // Atlas (0)
    auto atlas = decodeHpsdrDevice(0, 2);
    QCOMPARE(atlas.deviceType, HpsdrDeviceType::Metis);
    QCOMPARE(atlas.boardName, QString("Atlas"));

    // Hermes (1)
    auto hermes = decodeHpsdrDevice(1, 2);
    QCOMPARE(hermes.deviceType, HpsdrDeviceType::Hermes);

    // Angelia (3)
    auto angelia = decodeHpsdrDevice(3, 2);
    QCOMPARE(angelia.deviceType, HpsdrDeviceType::Angelia);
    QCOMPARE(angelia.adcs, 2);

    // Orion (4)
    auto orion = decodeHpsdrDevice(4, 2);
    QCOMPARE(orion.deviceType, HpsdrDeviceType::Orion);
    QCOMPARE(orion.adcs, 2);

    // Orion2 (5)
    auto orion2 = decodeHpsdrDevice(5, 2);
    QCOMPARE(orion2.deviceType, HpsdrDeviceType::Orion2);
    QCOMPARE(orion2.adcs, 2);

    // Saturn / G2 (10)
    auto saturn = decodeHpsdrDevice(10, 2);
    QCOMPARE(saturn.deviceType, HpsdrDeviceType::SaturnG2);
    QCOMPARE(saturn.adcs, 2);
    QCOMPARE(saturn.boardName, QString("Saturn/G2"));
}

void DeviceIdentityTests::hpsdrHermesLite2Versioning() {
    using namespace ProtocolBoundaryUtils;

    // HL1 (version < 40)
    auto hl1 = decodeHpsdrDevice(6, 1, 25);
    QCOMPARE(hl1.deviceType, HpsdrDeviceType::HermesLite);
    QCOMPARE(hl1.boardName, QString("HermesLite V1"));
    QCOMPARE(hl1.frequencyMax, 30720000.0);

    // HL2 (version >= 40, major 73, minor 2)
    auto hl2 = decodeHpsdrDevice(6, 1, 73, 2);
    QCOMPARE(hl2.deviceType, HpsdrDeviceType::HermesLite2);
    QCOMPARE(hl2.boardName, QString("HermesLite V2"));
    QCOMPARE(hl2.frequencyMax, 38400000.0);
    QCOMPARE(hl2.firmwareString, QString("v73.2"));

    // HL2 on Protocol 2
    auto hl2_p2 = decodeHpsdrDevice(6, 2, 74, 1);
    QCOMPARE(hl2_p2.deviceType, HpsdrDeviceType::HermesLite2);
    QCOMPARE(hl2_p2.boardName, QString("HermesLite V2"));
    QCOMPARE(hl2_p2.frequencyMax, 38400000.0);
    QCOMPARE(hl2_p2.firmwareString, QString("v74.1"));
}

void DeviceIdentityTests::anan10P1DiscoveryIsHermesNotLite() {
    using namespace ProtocolBoundaryUtils;

    unsigned char pkt[60] = {};
    pkt[0] = 0xEF;
    pkt[1] = 0xFE;
    pkt[2] = 0x02;
    pkt[3] = 0x00;
    pkt[4] = 0x1C;
    pkt[5] = 0xC0;
    pkt[6] = 0x00;
    pkt[7] = 0x00;
    pkt[8] = 0x01;
    pkt[9] = 6;   // firmware 6 — must not be read as board ID
    pkt[10] = 1;  // Hermes / ANAN-10
    pkt[11] = 6;  // leftover / HL2-style config bits, not board ID

    const auto parsed = parseHpsdrDiscoveryDatagram(pkt, int(sizeof(pkt)));
    QVERIFY(parsed.valid);
    QCOMPARE(parsed.protocol, 1);
    QCOMPARE(parsed.boardId, 1);
    QCOMPARE(parsed.swVersion, 6);

    const auto info = decodeHpsdrDevice(parsed.boardId, parsed.protocol, parsed.swVersion);
    QCOMPARE(info.deviceType, HpsdrDeviceType::Hermes);
    QCOMPARE(info.boardName, QString("Hermes"));
    QVERIFY(!isHermesLiteDeviceType(info.deviceType));

    unsigned char simHermes[60] = {
        0xef, 0xfe, 0x02, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 31, 1, 1
    };
    const auto simParsed = parseHpsdrDiscoveryDatagram(simHermes, int(sizeof(simHermes)));
    QVERIFY(simParsed.valid);
    QCOMPARE(simParsed.protocol, 1);
    QCOMPARE(simParsed.boardId, 1);
    QCOMPARE(simParsed.swVersion, 31);
    QCOMPARE(decodeHpsdrDevice(simParsed.boardId, simParsed.protocol, simParsed.swVersion).deviceType,
             HpsdrDeviceType::Hermes);
}

void DeviceIdentityTests::anan10P1WinsOverBogusP2Lite() {
    using namespace ProtocolBoundaryUtils;

    HpsdrDiscoveryReply p2Lite;
    p2Lite.valid = true;
    p2Lite.protocol = 2;
    p2Lite.boardId = 6;
    p2Lite.swVersion = 6;
    std::snprintf(p2Lite.mac_address, sizeof(p2Lite.mac_address), "00:1C:C0:00:00:01");

    HpsdrDiscoveryReply p1Hermes;
    p1Hermes.valid = true;
    p1Hermes.protocol = 1;
    p1Hermes.boardId = 1;
    p1Hermes.swVersion = 6;
    std::snprintf(p1Hermes.mac_address, sizeof(p1Hermes.mac_address), "00:1C:C0:00:00:01");

    const auto merged = mergeHpsdrDiscoveryReplies({p2Lite, p1Hermes});
    QCOMPARE(merged.size(), 1);
    QCOMPARE(merged.first().protocol, 1);
    QCOMPARE(merged.first().boardId, 1);

    const auto info = decodeHpsdrDevice(merged.first().boardId, merged.first().protocol,
                                        merged.first().swVersion);
    QCOMPARE(info.deviceType, HpsdrDeviceType::Hermes);
}

void DeviceIdentityTests::hermes2IsAnan10eNotGriffin() {
    using namespace ProtocolBoundaryUtils;
    const auto hermes2 = decodeHpsdrDevice(2, 1);
    QCOMPARE(hermes2.deviceType, HpsdrDeviceType::Hermes2);
    QCOMPARE(hermes2.boardName, QString("Hermes2"));
    QVERIFY(hermes2.modelName.contains(QStringLiteral("ANAN-10E")));
}

QTEST_APPLESS_MAIN(DeviceIdentityTests)
#include "device_identity_tests.moc"
