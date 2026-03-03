#ifndef CPROTOCOL1_H
#define CPROTOCOL1_H

#include "IHPSDRProtocol.h"
#include "cusdr_settings.h"
#include <QtEndian>

class CProtocol1 : public IHPSDRProtocol {
public:
    CProtocol1();
    ~CProtocol1() override;

    bool isPacketValid(const unsigned char* data, int len) override;
    uint32_t getSequence(const unsigned char* data) override;
    int getPacketType(const unsigned char* data) override;

    void processInputBuffer(const QByteArray& buffer, DataEngine* de) override;
    void decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) override;
    void encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState) override;

    QByteArray formatStartStop(char value) override;
    QByteArray formatInitFrame(int rx, THPSDRParameter* io) override;
    QByteArray formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) override;

    int getPayloadSize() override { return BUFFER_SIZE; }
    int getHeaderSize() override { return METIS_HEADER_SIZE; }

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
