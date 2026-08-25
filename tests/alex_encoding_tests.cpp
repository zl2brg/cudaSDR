#include <QtTest/QtTest>
#include "DataEngine/protocol_boundary_utils.h"

class AlexEncodingTests : public QObject {
    Q_OBJECT

private slots:
    void manualFilterSelectFlag();
    void manualHpfPacking();
    void manualLpfPacking();
    void autoLpfByFrequency();
    void c4ManualUsesConfigBits();
    void c4ManualEmptyFallsBackOnTx();
    void c4AutoUsesFrequencyOnlyWhileTx();
    void rxAntennaBits();
    void antennaRelayUsesRxWhileReceiving();
    void antennaRelayUsesTxWhileTransmitting();
    void txRelayBitsIgnoreAttenuator();
};

void AlexEncodingTests::manualFilterSelectFlag()
{
    QVERIFY(ProtocolBoundaryUtils::protocol1AlexManualFilterSelect(0x0001));
    QVERIFY(ProtocolBoundaryUtils::protocol1AlexManualFilterSelect(0x0081));
    QVERIFY(!ProtocolBoundaryUtils::protocol1AlexManualFilterSelect(0x0000));
    QVERIFY(!ProtocolBoundaryUtils::protocol1AlexManualFilterSelect(0x0080));
}

void AlexEncodingTests::manualHpfPacking()
{
    using namespace ProtocolBoundaryUtils;
    QCOMPARE(protocol1AlexManualHpfByte(0x0040), quint8(0x01)); // 13 MHz
    QCOMPARE(protocol1AlexManualHpfByte(0x0080), quint8(0x02)); // 20 MHz
    QCOMPARE(protocol1AlexManualHpfByte(0x0020), quint8(0x04)); // 9.5 MHz
    QCOMPARE(protocol1AlexManualHpfByte(0x0010), quint8(0x08)); // 6.5 MHz
    QCOMPARE(protocol1AlexManualHpfByte(0x0008), quint8(0x10)); // 1.5 MHz
    QCOMPARE(protocol1AlexManualHpfByte(0x0002), quint8(0x20)); // bypass
    QCOMPARE(protocol1AlexManualHpfByte(0x0004), quint8(0x40)); // 6m LNA
    QCOMPARE(protocol1AlexManualHpfByte(0x00FE), quint8(0x7F)); // all HPF lines
}

void AlexEncodingTests::manualLpfPacking()
{
    using namespace ProtocolBoundaryUtils;
    QCOMPARE(protocol1AlexManualLpfByte(0x0800), quint8(0x01)); // 30/20
    QCOMPARE(protocol1AlexManualLpfByte(0x0400), quint8(0x02)); // 60/40
    QCOMPARE(protocol1AlexManualLpfByte(0x0200), quint8(0x04)); // 80
    QCOMPARE(protocol1AlexManualLpfByte(0x0100), quint8(0x08)); // 160
    QCOMPARE(protocol1AlexManualLpfByte(0x4000), quint8(0x10)); // 6m
    QCOMPARE(protocol1AlexManualLpfByte(0x2000), quint8(0x20)); // 12/10
    QCOMPARE(protocol1AlexManualLpfByte(0x1000), quint8(0x40)); // 17/15
}

void AlexEncodingTests::autoLpfByFrequency()
{
    using namespace ProtocolBoundaryUtils;
    QCOMPARE(protocol1AlexAutoLpfByte(40000000L), quint8(0x10)); // 6m
    QCOMPARE(protocol1AlexAutoLpfByte(28000000L), quint8(0x20)); // 12/10
    QCOMPARE(protocol1AlexAutoLpfByte(21000000L), quint8(0x40)); // 17/15
    QCOMPARE(protocol1AlexAutoLpfByte(14000000L), quint8(0x01)); // 30/20
    QCOMPARE(protocol1AlexAutoLpfByte(7000000L), quint8(0x02));  // 60/40
    QCOMPARE(protocol1AlexAutoLpfByte(3500000L), quint8(0x04));  // 80
    QCOMPARE(protocol1AlexAutoLpfByte(1800000L), quint8(0x08));  // 160
}

void AlexEncodingTests::c4ManualUsesConfigBits()
{
    const quint16 cfg = 0x0001 | 0x0800; // manual + 30/20m LPF
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexC4LpfByte(cfg, 1800000L, true),
             quint8(0x01));
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexC4LpfByte(cfg, 1800000L, false),
             quint8(0x01));
}

void AlexEncodingTests::c4ManualEmptyFallsBackOnTx()
{
    const quint16 cfg = 0x0001; // manual, no LPF bits
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexC4LpfByte(cfg, 14000000L, true),
             quint8(0x01)); // auto 30/20
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexC4LpfByte(cfg, 14000000L, false),
             quint8(0x00));
}

void AlexEncodingTests::c4AutoUsesFrequencyOnlyWhileTx()
{
    const quint16 cfg = 0x0000;
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexC4LpfByte(cfg, 7000000L, true),
             quint8(0x02));
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexC4LpfByte(cfg, 7000000L, false),
             quint8(0x00));
}

void AlexEncodingTests::rxAntennaBits()
{
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexRxAntennaBits(0x01), quint8(0x01));
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexRxAntennaBits(0x02), quint8(0x02));
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexRxAntennaBits(0x1C), quint8(0x00)); // aux only
    QCOMPARE(ProtocolBoundaryUtils::protocol1AlexRxAntennaBits(0x63), quint8(0x03)); // RX + TX bits
}

void AlexEncodingTests::antennaRelayUsesRxWhileReceiving()
{
    using namespace ProtocolBoundaryUtils;
    // RX Ant2, TX Ant1 — while RX, relay must follow RX Ant2 → wire 1
    QCOMPARE(protocol1AlexAntennaRelayBits(0x22, false), quint8(0x01));
    QCOMPARE(protocol1AlexAntennaRelayBits(0x01, false), quint8(0x00)); // Ant1
    QCOMPARE(protocol1AlexAntennaRelayBits(0x03, false), quint8(0x02)); // Ant3
}

void AlexEncodingTests::antennaRelayUsesTxWhileTransmitting()
{
    using namespace ProtocolBoundaryUtils;
    // RX Ant2, TX Ant3 — while TX, relay must follow TX Ant3 → wire 2
    QCOMPARE(protocol1AlexAntennaRelayBits(0x62, true), quint8(0x02));
    QCOMPARE(protocol1AlexAntennaRelayBits(0x22, true), quint8(0x00)); // TX Ant1
}

void AlexEncodingTests::txRelayBitsIgnoreAttenuator()
{
    using namespace ProtocolBoundaryUtils;
    // TX ANT1 (bit5=1) + 10 dB att (bit7) must not leak into relay field
    QCOMPARE(protocol1AlexTxRelayBits(0xA0), quint8(0x00)); // Tx1 → wire 0
    QCOMPARE(protocol1AlexTxRelayBits(0x40), quint8(0x01)); // Tx2 → wire 1
    QCOMPARE(protocol1AlexTxRelayBits(0x60), quint8(0x02)); // Tx3 → wire 2
    QCOMPARE(protocol1AlexTxRelayBits(0x00), quint8(0x00)); // none → Tx1
}

QTEST_MAIN(AlexEncodingTests)
#include "alex_encoding_tests.moc"
