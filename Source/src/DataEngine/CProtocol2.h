#ifndef CPROTOCOL2_H
#define CPROTOCOL2_H

#include "IHPSDRProtocol.h"
#include <QtEndian>

class CProtocol2 : public IHPSDRProtocol {
public:
    CProtocol2();
    ~CProtocol2() override;

    bool isPacketValid(const unsigned char* data, int len) override;
    uint32_t getSequence(const unsigned char* data) override;
    int getPacketType(const unsigned char* data) override;

    void processInputBuffer(const QByteArray& buffer, DataEngine* de, quint16 sourcePort) override;
    void decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) override;
    void encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState, quint16& port) override;

    QByteArray formatStartStop(char value, quint16& port) override;
    QByteArray formatInitFrame(int rx, THPSDRParameter* io, quint16& port) override;
    QByteArray formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) override;

    int getPayloadSize() override { return 1428; } // 1444-byte DDC packet minus 16-byte header
    // Protocol 2 DDC data packet header (per spec v4.3 p.51):
    //   Bytes  0- 3: Sequence number       (4 bytes)
    //   Bytes  4-11: VITA-49 Time Stamp     (8 bytes)
    //   Bytes 12-13: Bits per sample        (2 bytes)
    //   Bytes 14-15: I&Q Samples per frame  (2 bytes)
    //   Byte  16+  : I&Q sample data
    // Total header = 16 bytes; IQ data = 238 × 6 = 1428 bytes.
    int getHeaderSize() override { return 16; }
    // Returns the ports to bind: control ports 1024-1029 plus one DDC port
    // per active receiver (1035 for DDC0, 1036 for DDC1, ...).
    QList<quint16> getRequiredPorts() override;

private:
    uint32_t m_lastSequence;
    // Per-DDC sample fill counters (one per possible DDC channel, indexed by
    // DDC index = port - 1035).  Separate counters are needed because DDC0 and
    // DDC1 packets arrive independently and must each fill their RX inBuf
    // without interfering with each other's fill state.
    int m_rxSamplesPerDDC[MAX_RECEIVERS];
    QMap<quint16, uint32_t> m_sequences;
    // Stored by isPacketValid() and read by getPacketType() to discriminate
    // between DDC-data packets (large) and High-Priority-Status packets (small).
    mutable int m_lastPacketLen;
};

#endif // CPROTOCOL2_H
