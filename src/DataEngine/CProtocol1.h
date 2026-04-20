#ifndef CPROTOCOL1_H
#define CPROTOCOL1_H

#include "IHPSDRProtocol.h"
#include "cusdr_settings.h"
#include <QtEndian>

// Alex TX LPF auto-select thresholds (Hz) — Protocol 1, C4 byte
static constexpr long ALEX_LPF_6M_MIN_HZ    = 35600000L; // > this → 6m LPF (0x10)
static constexpr long ALEX_LPF_12_10M_MIN_HZ = 24000000L; // > this → 12/10m LPF (0x20)
static constexpr long ALEX_LPF_17_15M_MIN_HZ = 16500000L; // > this → 17/15m LPF (0x40)
static constexpr long ALEX_LPF_30_20M_MIN_HZ =  8000000L; // > this → 30/20m LPF (0x01)
static constexpr long ALEX_LPF_60_40M_MIN_HZ =  5000000L; // > this → 60/40m LPF (0x02)
static constexpr long ALEX_LPF_80M_MIN_HZ    =  2500000L; // > this → 80m LPF (0x04)
                                                           // else      → 160m LPF (0x08)

class CProtocol1 : public IHPSDRProtocol {
public:
    CProtocol1();
    ~CProtocol1() override;

    bool isPacketValid(const unsigned char* data, int len) override;
    uint32_t getSequence(const unsigned char* data) override;
    int getPacketType(const unsigned char* data) override;

    void processInputBuffer(const QByteArray& buffer, DataEngine* de, quint16 sourcePort) override;
    void decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) override;
    void encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState, quint16& port) override;

    QByteArray formatStartStop(char value, quint16& port) override;
    QByteArray formatInitFrame(int rx, THPSDRParameter* io, quint16& port) override;
    QByteArray formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) override;

    int getPayloadSize() override { return BUFFER_SIZE; }
    int getHeaderSize() override { return METIS_HEADER_SIZE; }
    QList<quint16> getRequiredPorts() override;

private:
    QByteArray m_metisGetDataSignature;
    QByteArray m_deviceSendDataSignature;

    quint8  m_adc_rx1_4, m_adc_rx5_8, m_adc_rx9_16;
    quint8  m_new_adc_rx1_4, m_new_adc_rx5_8, m_new_adc_rx9_16;
    int     m_firstTimeRxInit;
    int     m_rxSamples;
    int     m_fwCount;

    double  m_lsample;
    double  m_rsample;
    int     m_leftSample;
    int     m_rightSample;
    int     m_micSample;
    float   m_micSample_float;
};

#endif // CPROTOCOL1_H
