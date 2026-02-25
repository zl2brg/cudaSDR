/**
 * @file  Protocol1FrameDecoder.h
 * @brief HPSDR Protocol 1 IQ frame decoder / C&C encoder.
 *
 * Extracted from DataProcessor to isolate all wire-format knowledge.
 * Implements IFrameDecoder so DataProcessor can swap in future protocol
 * decoders (Protocol2FrameDecoder, SoapyFrameDecoder) without changes.
 */

#ifndef PROTOCOL1FRAMEDECODER_H
#define PROTOCOL1FRAMEDECODER_H

#include "IFrameDecoder.h"
#include "cusdr_settings.h"
#include <QElapsedTimer>

class DataEngine;

class Protocol1FrameDecoder : public IFrameDecoder {

    Q_OBJECT

public:
    /**
     * @param de      DataEngine owning the RX list and io struct.
     * @param hwMode  Metis vs Hermes (affects C&C byte decode paths).
     * @param s       Settings singleton.
     */
    explicit Protocol1FrameDecoder(DataEngine* de,
                                   QSDR::_HWInterfaceMode hwMode,
                                   Settings* s);
    ~Protocol1FrameDecoder() override = default;

    // IFrameDecoder interface
    void decodeInputFrame(const QByteArray& buffer) override;
    void encodeControlBytes() override;

    /**
     * @brief Initialise the RX frequency round-robin counter.
     *        Call once after all receivers have been created, with
     *        the receiver count as the argument.
     */
    void setFirstTimeRxInit(int n) { m_firstTimeRxInit = n; }

private:
    // Back-pointer context
    DataEngine*               de;
    QSDR::_HWInterfaceMode    m_hwInterface;
    Settings*                 set;

    // ---------- RX decode state ----------
    int  m_rxSamples;
    int  m_maxSamples;
    int  m_leftSample;
    int  m_rightSample;
    int  m_micSample;
    double m_lsample;
    double m_rsample;
    float  m_micSample_float;

    QElapsedTimer m_SyncChangedTime; ///< Rate-limiter for SYNC error reporting

    // ---------- CC decode state ----------
    int            m_fwCount;          ///< How many C&C frames processed (firmware version latch)
    QElapsedTimer  m_ADCChangedTime;   ///< Rate-limiter for ADC overflow reporting

    // ---------- CC encode state ----------
    int     m_sendState;              ///< Round-robin state machine index (0-7)
    int     m_firstTimeRxInit;        ///< Counts down to force all RX freq frames on startup
    quint8  m_adc_rx1_4;              ///< Cached ADC mode bits for receivers 1-4
    quint8  m_adc_rx5_8;              ///< Cached ADC mode bits for receivers 5-8
    quint8  m_adc_rx9_16;             ///< Cached ADC mode bits for receivers 9-16
    quint8  m_new_adc_rx1_4;          ///< Freshly sampled ADC mode bits for receivers 1-4
    quint8  m_new_adc_rx5_8;          ///< Freshly sampled ADC mode bits for receivers 5-8
    quint8  m_new_adc_rx9_16;         ///< Freshly sampled ADC mode bits for receivers 9-16

    /**
     * @brief Decode 5 C&C bytes from one incoming half-frame.
     *        Updates de->io.ccRx and emits keyerEvent() on state changes.
     */
    void decodeCCBytes(const QByteArray& buffer);
};

#endif // PROTOCOL1FRAMEDECODER_H
