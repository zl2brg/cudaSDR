#ifndef PROTOCOL_BOUNDARY_UTILS_H
#define PROTOCOL_BOUNDARY_UTILS_H

#include <QByteArray>
#include <QtEndian>
#include <cstring>
#include <cstdint>

namespace ProtocolBoundaryUtils {

constexpr int kProtocol1MetisDataSize = 1032;
constexpr unsigned char kProtocol1Sig0 = 0xEF;
constexpr unsigned char kProtocol1Sig1 = 0xFE;
constexpr unsigned char kProtocol1Sig2 = 0x01;

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

    quint16 ddcAudioPort = qToBigEndian(static_cast<quint16>(1028));
    memcpy(pkt.data() + 13, &ddcAudioPort, 2);

    quint16 ducIqPort = qToBigEndian(static_cast<quint16>(1029));
    memcpy(pkt.data() + 15, &ducIqPort, 2);

    quint16 ddc0Port = qToBigEndian(static_cast<quint16>(1035));
    memcpy(pkt.data() + 17, &ddc0Port, 2);

    pkt[23] = 1;

    out.datagram = pkt;
    return out;
}

}  // namespace ProtocolBoundaryUtils

#endif  // PROTOCOL_BOUNDARY_UTILS_H
