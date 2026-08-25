#ifndef PROTOCOL_BOUNDARY_UTILS_H
#define PROTOCOL_BOUNDARY_UTILS_H

#include <QByteArray>
#include <QtEndian>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>

namespace ProtocolBoundaryUtils {

constexpr int kProtocol1MetisDataSize = 1032;
constexpr unsigned char kProtocol1Sig0 = 0xEF;
constexpr unsigned char kProtocol1Sig1 = 0xFE;
constexpr unsigned char kProtocol1Sig2 = 0x01;
constexpr unsigned char kSyncByte = 0x7F;

inline bool isProtocol1MetisPacketValid(const unsigned char* data, int len) {
    if (!data || len != kProtocol1MetisDataSize) {
        return false;
    }
    return data[0] == kProtocol1Sig0 && data[1] == kProtocol1Sig1 && data[2] == kProtocol1Sig2;
}

inline uint32_t protocol1Sequence(const unsigned char* data) {
    return ((data[4] & 0xFFu) << 24) |
           ((data[5] & 0xFFu) << 16) |
           ((data[6] & 0xFFu) << 8) |
           (data[7] & 0xFFu);
}

/** Standard Port Definitions */
enum Ports : quint16 {
    DevicePort = 1024,
    P2DdcSpecPort = 1025,
    P2TxSpecPort = 1026,
    P2HighPriorityPort = 1027,
    P2AudioPort = 1028,
    P2DucIqPort = 1029,
    P2Ddc0Port = 1035,
    // SDR->PC source ports.  The protocol defaults (HP status 1025, mic 1026,
    // wideband 1027) are the same numbers as the PC->SDR receive ports, so a
    // device that binds both ends up with two sockets per port and silently
    // drops half the config we send it.  Assign these explicitly instead.
    P2HpStatusSourcePort = 1060,
    P2MicSourcePort = 1061,
    P2WidebandSourcePort = 1062,
    LegacyDataPort = 8886
};

inline int protocol2PacketTypeForLength(int len) {
    if (len == 1040) return 0x04;   // Wideband
    if (len > 1000) return 0x06;    // IQ data
    if (len == 60) return 0x05;     // High-priority status
    return 0xFF;                    // Unknown
}

inline QByteArray protocol2StartStopDatagram(char value, uint32_t sequence) {
    QByteArray commandDatagram(1444, '\0');
    uint32_t seq = qToBigEndian(sequence);
    memcpy(commandDatagram.data(), &seq, sizeof(seq));
    commandDatagram[4] = value ? 0x01 : 0x00;
    return commandDatagram;
}

/** Protocol 2 General Configuration packet (PC → SDR, port 1024, 60 bytes). */
struct Protocol2InitFrameResult {
    quint16 port = 0;
    QByteArray datagram;
};

/**
 * Mirrors CProtocol2::formatInitFrame(): rx>0 yields empty datagram; rx==0 builds
 * the 60-byte general config with default port assignments and wideband enable.
 * \a seq1024 is incremented on each rx==0 build (same as m_sequences[1024]++).
 */
inline Protocol2InitFrameResult protocol2FormatInitFrame(int rx, uint32_t& seq1024) {
    Protocol2InitFrameResult out;
    if (rx > 0) {
        out.port = 1024;
        return out;
    }

    out.port = 1024;
    QByteArray pkt(60, 0);

    uint32_t seq = qToBigEndian(seq1024++);
    memcpy(pkt.data(), &seq, sizeof(seq));

    pkt[4] = 0x00;

    quint16 ddcSpecPort = qToBigEndian(static_cast<quint16>(1025));
    memcpy(pkt.data() + 5, &ddcSpecPort, 2);

    quint16 ducSpecPort = qToBigEndian(static_cast<quint16>(1026));
    memcpy(pkt.data() + 7, &ducSpecPort, 2);

    quint16 hpPcPort = qToBigEndian(static_cast<quint16>(1027));
    memcpy(pkt.data() + 9, &hpPcPort, 2);

    // SDR->PC source ports, named explicitly so they do not land on the
    // PC->SDR receive ports (see Ports::P2HpStatusSourcePort).
    quint16 hpStatusPort = qToBigEndian(static_cast<quint16>(Ports::P2HpStatusSourcePort));
    memcpy(pkt.data() + 11, &hpStatusPort, 2);

    quint16 ddcAudioPort = qToBigEndian(static_cast<quint16>(1028));
    memcpy(pkt.data() + 13, &ddcAudioPort, 2);

    quint16 ducIqPort = qToBigEndian(static_cast<quint16>(1029));
    memcpy(pkt.data() + 15, &ducIqPort, 2);

    quint16 ddc0Port = qToBigEndian(static_cast<quint16>(1035));
    memcpy(pkt.data() + 17, &ddc0Port, 2);

    quint16 micPort = qToBigEndian(static_cast<quint16>(Ports::P2MicSourcePort));
    memcpy(pkt.data() + 19, &micPort, 2);

    quint16 widebandPort = qToBigEndian(static_cast<quint16>(Ports::P2WidebandSourcePort));
    memcpy(pkt.data() + 21, &widebandPort, 2);

    pkt[23] = 1;

    // Byte 59: Alex0 enable (bit 0). Without this hpsdrsim ignores Alex bits.
    pkt[59] = 0x01;

    out.datagram = pkt;
    return out;
}

/** 
 * Decodes a 24-bit signed big-endian value into a 32-bit signed integer. 
 * High bit of the 24-bit value is the sign bit.
 */
inline int32_t decode24BitBE(const unsigned char* p) {
    int32_t val = (static_cast<int8_t>(p[0]) << 16) |
                  (static_cast<uint8_t>(p[1]) << 8) |
                   static_cast<uint8_t>(p[2]);
    return val;
}

/**
 * Decodes a 16-bit signed big-endian value into a 32-bit signed integer.
 */
inline int32_t decode16BitBE(const unsigned char* p) {
    return qFromBigEndian<int16_t>(p);
}

/** Protocol Packet Constants */
constexpr int kProtocol1HeaderSize = 8;
constexpr int kProtocol2HeaderSize = 16;
constexpr int kPacketTypeP1IqPrimary = 0x06;
constexpr int kPacketTypeP1IqLoopback = 0x02;
constexpr int kPacketTypeWideband = 0x04;
constexpr int kPacketTypeP2HighPriorityStatus = 0x05;
constexpr int kProtocol2IqPacketSize = 1444;
constexpr int kProtocol2WidebandPacketSize = 1040;
constexpr int kProtocol2HpStatusPacketSize = 60;

/** HPSDR C&C / P2 status AIN fields are 12-bit values in a 16-bit word. */
constexpr quint16 kAin12BitMask = 0x0FFFu;

/** Decode big-endian 12-bit AIN from two protocol bytes (avoids signed-char shifts). */
inline quint16 decodeAin12BitBE(quint8 hi, quint8 lo)
{
    return static_cast<quint16>((static_cast<quint16>(hi) << 8) | lo) & kAin12BitMask;
}

inline quint16 decodeAin12BitBE(const QByteArray& buffer, int hiIndex)
{
    return decodeAin12BitBE(static_cast<quint8>(buffer.at(hiIndex)),
                            static_cast<quint8>(buffer.at(hiIndex + 1)));
}

/** PA watt-meter bridge constants (pihpsdr / Thetis / hpsdrsim). */
struct PaBridgeCal {
    double vref = 3.3;
    double bridge = 0.09;
    int adcOffset = 6;
};

inline PaBridgeCal paBridgeCalForHermes(bool hermes)
{
    // Metis/Alex default: 3.3 / 0.09. Hermes / Angelia: 3.3 / 0.095.
    return hermes ? PaBridgeCal{3.3, 0.095, 6} : PaBridgeCal{3.3, 0.09, 6};
}

inline double ain12ToVolts(quint16 ain12, double vref = 3.3)
{
    return vref * static_cast<double>(ain12) / 4095.0;
}

/** watts = ((ADC - offset) / 4095 * Vref)^2 / bridge */
inline double wattsFromAin12(quint16 ain12, const PaBridgeCal& cal)
{
    int code = static_cast<int>(ain12) - cal.adcOffset;
    if (code < 0)
        code = 0;
    const double v = (static_cast<double>(code) / 4095.0) * cal.vref;
    if (cal.bridge <= 0.0)
        return 0.0;
    return (v * v) / cal.bridge;
}

inline double alexBridgeWattsFromVolts(double volts, double bridge = 0.09)
{
    if (bridge <= 0.0)
        return 0.0;
    return volts * volts / bridge;
}

/**
 * SWR from Alex/Hermes fwd & rev watts.
 * Below \a kMinFwdWattsForSwr the PA ADCs are noise (P1 RX often ~0.02 W) and
 * rho clamps to 0.999 → SWR ≈ 1999. Require TX + real forward power.
 */
constexpr double kMinFwdWattsForSwr = 0.1;
constexpr double kMaxReportedSwr = 25.0;

inline double swrFromFwdRevWatts(double fwdWatts, double revWatts, bool transmitting)
{
    if (!transmitting || !(fwdWatts >= kMinFwdWattsForSwr))
        return 1.0;
    if (revWatts < 0.0)
        revWatts = 0.0;
    double rho = std::sqrt(revWatts / fwdWatts);
    if (rho > 0.999)
        rho = 0.999;
    const double swr = (1.0 + rho) / (1.0 - rho);
    return std::min(swr, kMaxReportedSwr);
}

/** Alex TX LPF auto-select thresholds (Hz) — Protocol 1, C4 byte */
constexpr long kAlexLpf6mMinHz     = 35600000L; // > this → 6m LPF (0x10)
constexpr long kAlexLpf12_10mMinHz = 24000000L; // > this → 12/10m LPF (0x20)
constexpr long kAlexLpf17_15mMinHz = 16500000L; // > this → 17/15m LPF (0x40)
constexpr long kAlexLpf30_20mMinHz =  8000000L; // > this → 30/20m LPF (0x01)
constexpr long kAlexLpf60_40mMinHz =  5000000L; // > this → 60/40m LPF (0x02)
constexpr long kAlexLpf80mMinHz    =  2500000L; // > this → 80m LPF (0x04)
                                                // else      → 160m LPF (0x08)

/**
 * Canonical alexConfig bitfield (Settings / Protocol 2 / Protocol 1):
 *   0x0001 manual HPF/LPF select
 *   0x0002 bypass all HPFs
 *   0x0004 6m LNA
 *   0x0008–0x0080 HPF lines (1.5 / 6.5 / 9.5 / 13 / 20 MHz)
 *   0x0100–0x4000 LPF lines (160 … 6m)
 *
 * Protocol 1 C3 HPF (bits valid only when C2 bit 6 = manual):
 *   bit0=13MHz, bit1=20MHz, bit2=9.5, bit3=6.5, bit4=1.5, bit5=bypass, bit6=6m LNA
 * Protocol 1 C4 LPF:
 *   bit0=30/20, bit1=60/40, bit2=80, bit3=160, bit4=6m, bit5=12/10, bit6=17/15
 */
inline bool protocol1AlexManualFilterSelect(quint16 alexConfig)
{
    return (alexConfig & 0x0001u) != 0;
}

/** Pack alexConfig HPF bits into Protocol 1 C3 (VNA bit 7 not included). */
inline quint8 protocol1AlexManualHpfByte(quint16 alexConfig)
{
    quint8 c3 = 0;
    c3 |= static_cast<quint8>((alexConfig & 0x0040u) >> 6); // 13 MHz → bit 0
    c3 |= static_cast<quint8>((alexConfig & 0x0080u) >> 6); // 20 MHz → bit 1
    c3 |= static_cast<quint8>((alexConfig & 0x0020u) >> 3); // 9.5 MHz → bit 2
    c3 |= static_cast<quint8>((alexConfig & 0x0010u) >> 1); // 6.5 MHz → bit 3
    c3 |= static_cast<quint8>((alexConfig & 0x0008u) << 1); // 1.5 MHz → bit 4
    c3 |= static_cast<quint8>((alexConfig & 0x0002u) << 4); // bypass → bit 5
    c3 |= static_cast<quint8>((alexConfig & 0x0004u) << 4); // 6m LNA → bit 6
    return c3;
}

/** Pack alexConfig LPF bits into Protocol 1 C4. */
inline quint8 protocol1AlexManualLpfByte(quint16 alexConfig)
{
    quint8 c4 = 0;
    c4 |= static_cast<quint8>((alexConfig & 0x0800u) >> 11); // 30/20m → bit 0
    c4 |= static_cast<quint8>((alexConfig & 0x0400u) >> 9);  // 60/40m → bit 1
    c4 |= static_cast<quint8>((alexConfig & 0x0200u) >> 7);  // 80m → bit 2
    c4 |= static_cast<quint8>((alexConfig & 0x0100u) >> 5);  // 160m → bit 3
    c4 |= static_cast<quint8>((alexConfig & 0x4000u) >> 10); // 6m → bit 4
    c4 |= static_cast<quint8>((alexConfig & 0x2000u) >> 8);  // 12/10m → bit 5
    c4 |= static_cast<quint8>((alexConfig & 0x1000u) >> 6);  // 17/15m → bit 6
    return c4;
}

/** Frequency-based Alex LPF C4 byte (auto filter select / TX safety fallback). */
inline quint8 protocol1AlexAutoLpfByte(long txFrequencyHz)
{
    if (txFrequencyHz > kAlexLpf6mMinHz)
        return 0x10;
    if (txFrequencyHz > kAlexLpf12_10mMinHz)
        return 0x20;
    if (txFrequencyHz > kAlexLpf17_15mMinHz)
        return 0x40;
    if (txFrequencyHz > kAlexLpf30_20mMinHz)
        return 0x01;
    if (txFrequencyHz > kAlexLpf60_40mMinHz)
        return 0x02;
    if (txFrequencyHz > kAlexLpf80mMinHz)
        return 0x04;
    return 0x08; // 160m
}

/**
 * Protocol 1 case-3 C4: manual LPF from alexConfig; if manual but no LPF bit is
 * set while transmitting, fall back to frequency auto-select so the PA is not
 * left without a filter. Auto mode uses frequency only while TX is active.
 */
inline quint8 protocol1AlexC4LpfByte(quint16 alexConfig, long txFrequencyHz, bool transmitting)
{
    if (protocol1AlexManualFilterSelect(alexConfig)) {
        const quint8 manual = protocol1AlexManualLpfByte(alexConfig);
        if (manual != 0)
            return manual;
        if (transmitting)
            return protocol1AlexAutoLpfByte(txFrequencyHz);
        return 0;
    }
    if (transmitting)
        return protocol1AlexAutoLpfByte(txFrequencyHz);
    return 0;
}

/** alexStates bits [1:0] → Protocol 1 C3 RX antenna field (0–3). */
inline quint8 protocol1AlexRxAntennaBits(int alexState)
{
    return static_cast<quint8>(alexState & 0x03);
}

/**
 * Protocol 1 C4 antenna relay (same policy as Protocol 2 Alex0):
 * use RX Ant bits [1:0] while receiving, TX Ant bits [6:5] while transmitting.
 * UI 0=none→Tx1, 1=Ant1, 2=Ant2, 3=Ant3 → wire 0=Tx1, 1=Tx2, 2=Tx3.
 * Masks so attenuator bits [8:7] never leak into the relay field.
 */
inline quint8 protocol1AlexAntennaRelayBits(int alexState, bool transmitting)
{
    const int ant = transmitting
        ? ((alexState >> 5) & 0x03)
        : (alexState & 0x03);
    return static_cast<quint8>((ant != 0) ? (ant - 1) : ant);
}

/** TX-only packing (MOX/PTT path). Prefer protocol1AlexAntennaRelayBits. */
inline quint8 protocol1AlexTxRelayBits(int alexState)
{
    return protocol1AlexAntennaRelayBits(alexState, true);
}

}  // namespace ProtocolBoundaryUtils

#endif  // PROTOCOL_BOUNDARY_UTILS_H
