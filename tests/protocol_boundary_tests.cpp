#include <QtTest/QtTest>
#include "DataEngine/protocol_boundary_utils.h"

class ProtocolBoundaryTests : public QObject {
    Q_OBJECT

private slots:
    void protocol2PacketTypeByLength();
    void protocol2StartStopDatagramShape();
    void protocol1HeaderAndSequenceParsing();
};

void ProtocolBoundaryTests::protocol2PacketTypeByLength() {
    QCOMPARE(ProtocolBoundaryUtils::protocol2PacketTypeForLength(1040), 0x04);
    QCOMPARE(ProtocolBoundaryUtils::protocol2PacketTypeForLength(1444), 0x06);
    QCOMPARE(ProtocolBoundaryUtils::protocol2PacketTypeForLength(60), 0x05);
    QCOMPARE(ProtocolBoundaryUtils::protocol2PacketTypeForLength(1000), 0xFF);
    QCOMPARE(ProtocolBoundaryUtils::protocol2PacketTypeForLength(0), 0xFF);
}

void ProtocolBoundaryTests::protocol2StartStopDatagramShape() {
    const QByteArray start = ProtocolBoundaryUtils::protocol2StartStopDatagram(1, 0x01020304u);
    QCOMPARE(start.size(), 1444);
    QCOMPARE(static_cast<unsigned char>(start[0]), static_cast<unsigned char>(0x01));
    QCOMPARE(static_cast<unsigned char>(start[1]), static_cast<unsigned char>(0x02));
    QCOMPARE(static_cast<unsigned char>(start[2]), static_cast<unsigned char>(0x03));
    QCOMPARE(static_cast<unsigned char>(start[3]), static_cast<unsigned char>(0x04));
    QCOMPARE(static_cast<unsigned char>(start[4]), static_cast<unsigned char>(0x01));

    const QByteArray stop = ProtocolBoundaryUtils::protocol2StartStopDatagram(0, 0xA0B0C0D0u);
    QCOMPARE(stop.size(), 1444);
    QCOMPARE(static_cast<unsigned char>(stop[4]), static_cast<unsigned char>(0x00));
}

void ProtocolBoundaryTests::protocol1HeaderAndSequenceParsing() {
    QByteArray packet(ProtocolBoundaryUtils::kProtocol1MetisDataSize, '\0');
    packet[0] = static_cast<char>(ProtocolBoundaryUtils::kProtocol1Sig0);
    packet[1] = static_cast<char>(ProtocolBoundaryUtils::kProtocol1Sig1);
    packet[2] = static_cast<char>(ProtocolBoundaryUtils::kProtocol1Sig2);
    packet[4] = static_cast<char>(0x12);
    packet[5] = static_cast<char>(0x34);
    packet[6] = static_cast<char>(0x56);
    packet[7] = static_cast<char>(0x78);

    QVERIFY(ProtocolBoundaryUtils::isProtocol1MetisPacketValid(
        reinterpret_cast<const unsigned char*>(packet.constData()), packet.size()));
    QCOMPARE(ProtocolBoundaryUtils::protocol1Sequence(
                 reinterpret_cast<const unsigned char*>(packet.constData())),
             0x12345678u);

    packet[0] = 0x00;
    QVERIFY(!ProtocolBoundaryUtils::isProtocol1MetisPacketValid(
        reinterpret_cast<const unsigned char*>(packet.constData()), packet.size()));
}

QTEST_MAIN(ProtocolBoundaryTests)
#include "protocol_boundary_tests.moc"
