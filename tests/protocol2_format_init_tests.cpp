#include <QtTest/QtTest>
#include "DataEngine/protocol_boundary_utils.h"

class Protocol2FormatInitTests : public QObject {
    Q_OBJECT

private slots:
    void rxGreaterThanZeroReturnsEmptyOnPort1024();
    void rxZeroPacketSizeAndPort();
    void rxZeroByteLayoutAndSequenceIncrement();
};

void Protocol2FormatInitTests::rxGreaterThanZeroReturnsEmptyOnPort1024() {
    uint32_t seq = 7;
    for (int rx : {1, 3, 7}) {
        const auto r = ProtocolBoundaryUtils::protocol2FormatInitFrame(rx, seq);
        QCOMPARE(r.port, static_cast<quint16>(1024));
        QVERIFY(r.datagram.isEmpty());
    }
    QCOMPARE(seq, 7u);
}

void Protocol2FormatInitTests::rxZeroPacketSizeAndPort() {
    uint32_t seq = 0;
    const auto r = ProtocolBoundaryUtils::protocol2FormatInitFrame(0, seq);
    QCOMPARE(r.port, static_cast<quint16>(1024));
    QCOMPARE(r.datagram.size(), 60);
    QCOMPARE(seq, 1u);
}

void Protocol2FormatInitTests::rxZeroByteLayoutAndSequenceIncrement() {
    uint32_t seq = 0x01020304u;
    const auto first = ProtocolBoundaryUtils::protocol2FormatInitFrame(0, seq);
    QCOMPARE(first.datagram.size(), 60);
    QCOMPARE(static_cast<unsigned char>(first.datagram[0]), 0x01u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[1]), 0x02u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[2]), 0x03u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[3]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[4]), 0x00u);

    // 1025, 1026, 1027, 1028, 1029, 1035 — big-endian uint16
    QCOMPARE(static_cast<unsigned char>(first.datagram[5]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[6]), 0x01u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[7]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[8]), 0x02u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[9]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[10]), 0x03u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[11]), 0x04u); // HP status 1060
    QCOMPARE(static_cast<unsigned char>(first.datagram[12]), 0x24u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[13]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[14]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[15]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[16]), 0x05u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[17]), 0x04u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[18]), 0x0Bu);

    QCOMPARE(static_cast<unsigned char>(first.datagram[19]), 0x04u); // mic 1061
    QCOMPARE(static_cast<unsigned char>(first.datagram[20]), 0x25u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[21]), 0x04u); // wideband 1062
    QCOMPARE(static_cast<unsigned char>(first.datagram[22]), 0x26u);
    QCOMPARE(static_cast<unsigned char>(first.datagram[23]), 0x01u);
    for (int i = 24; i < 59; ++i) {
        QCOMPARE(static_cast<unsigned char>(first.datagram[i]), 0x00u);
    }
    QCOMPARE(static_cast<unsigned char>(first.datagram[59]), 0x01u); // Alex0 enable

    QCOMPARE(seq, 0x01020305u);

    const auto second = ProtocolBoundaryUtils::protocol2FormatInitFrame(0, seq);
    QCOMPARE(seq, 0x01020306u);
    QCOMPARE(static_cast<unsigned char>(second.datagram[0]), 0x01u);
    QCOMPARE(static_cast<unsigned char>(second.datagram[1]), 0x02u);
    QCOMPARE(static_cast<unsigned char>(second.datagram[2]), 0x03u);
    QCOMPARE(static_cast<unsigned char>(second.datagram[3]), 0x05u);
}

QTEST_MAIN(Protocol2FormatInitTests)
#include "protocol2_format_init_tests.moc"
