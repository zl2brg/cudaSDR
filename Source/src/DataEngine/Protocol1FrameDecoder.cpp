/**
 * @file  Protocol1FrameDecoder.cpp
 * @brief HPSDR Protocol 1 IQ frame decoder / C&C encoder.
 *
 * This file contains code originally part of DataProcessor inside
 * cusdr_dataEngine.cpp.  It was extracted to implement the IFrameDecoder
 * interface and isolate all Protocol 1 wire-format specifics.
 */

#include "Protocol1FrameDecoder.h"
#include "cusdr_dataEngine.h"   // DataEngine, RX, io, BUFFER_SIZE, etc.

Protocol1FrameDecoder::Protocol1FrameDecoder(DataEngine* de,
                                             QSDR::_HWInterfaceMode hwMode,
                                             Settings* s)
    : IFrameDecoder(nullptr)
    , de(de)
    , m_hwInterface(hwMode)
    , set(s)
    , m_rxSamples(0)
    , m_maxSamples(0)
    , m_leftSample(0)
    , m_rightSample(0)
    , m_micSample(0)
    , m_lsample(0.0)
    , m_rsample(0.0)
    , m_micSample_float(0.0f)
    , m_fwCount(0)
    , m_sendState(0)
    , m_firstTimeRxInit(0)
    , m_adc_rx1_4(0)
    , m_adc_rx5_8(0)
    , m_adc_rx9_16(0)
    , m_new_adc_rx1_4(0)
    , m_new_adc_rx5_8(0)
    , m_new_adc_rx9_16(0)
{
    m_SyncChangedTime.start();
    m_ADCChangedTime.start();
}

// ---------------------------------------------------------------------------
// IFrameDecoder::decodeInputFrame
// ---------------------------------------------------------------------------
// Moved from DataProcessor::processInputBuffer (cusdr_dataEngine.cpp).
// Parses one 512-byte Protocol 1 half-frame: 3 SYNC bytes, 5 C&C bytes,
// then receiver IQ triplets interleaved with a 16-bit mic sample.
// Triggers DSP processing on each receiver when BUFFER_SIZE samples
// have been accumulated.
// ---------------------------------------------------------------------------
void Protocol1FrameDecoder::decodeInputFrame(const QByteArray& buffer)
{
    int s = 0;

    if (buffer.at(s++) == SYNC && buffer.at(s++) == SYNC && buffer.at(s++) == SYNC)
    {
        // Extract C&C bytes
        decodeCCBytes(buffer.mid(3, 5));
        s += 5;

        switch (de->io.receivers)
        {
            case 1:  m_maxSamples = 512 - 0;   break;
            case 2:  m_maxSamples = 512 - 0;   break;
            case 3:  m_maxSamples = 512 - 4;   break;
            case 4:  m_maxSamples = 512 - 10;  break;
            case 5:  m_maxSamples = 512 - 24;  break;
            case 6:  m_maxSamples = 512 - 10;  break;
            case 7:  m_maxSamples = 512 - 20;  break;
            case 8:  m_maxSamples = 512 - 4;   break;
            case 9:  m_maxSamples = 512 - 0;   break;
            case 10: m_maxSamples = 512 - 8;   break;
            case 11: m_maxSamples = 512 - 28;  break;
            case 12: m_maxSamples = 512 - 60;  break;
            case 13: m_maxSamples = 512 - 24;  break;
            case 14: m_maxSamples = 512 - 74;  break;
            case 15: m_maxSamples = 512 - 44;  break;
            case 16: m_maxSamples = 512 - 14;  break;
            case 17: m_maxSamples = 512 - 88;  break;
            case 18: m_maxSamples = 512 - 64;  break;
            case 19: m_maxSamples = 512 - 40;  break;
            case 20: m_maxSamples = 512 - 16;  break;
        }

        while (s < m_maxSamples)
        {
            for (int r = 0; r < de->io.receivers; r++)
            {
                m_leftSample   = (int)((  signed char) buffer.at(s++)) << 16;
                m_leftSample  += (int)((unsigned char) buffer.at(s++)) << 8;
                m_leftSample  += (int)((unsigned char) buffer.at(s++));
                m_rightSample  = (int)((  signed char) buffer.at(s++)) << 16;
                m_rightSample += (int)((unsigned char) buffer.at(s++)) << 8;
                m_rightSample += (int)((unsigned char) buffer.at(s++));

                m_lsample = (double)(m_leftSample  / 8388607.0f);
                m_rsample = (double)(m_rightSample / 8388607.0f);

                if (de->RX.at(r)->qtwdsp) {
                    de->RX[r]->inBuf[m_rxSamples].re = m_lsample;
                    de->RX[r]->inBuf[m_rxSamples].im = m_rsample;
                }
            }

            m_micSample  = (int)((signed char) buffer.at(s++)) << 8;
            m_micSample += (int)((unsigned char) buffer.at(s++));
            m_micSample_float = (float) m_micSample / 32767.0f * de->io.mic_gain;

            m_rxSamples++;

            if (m_rxSamples == BUFFER_SIZE)
            {
                for (int r = 0; r < de->io.receivers; r++) {
                    if (de->RX.at(r)->qtwdsp) {
                        QMetaObject::invokeMethod(de->RX.at(r),
                                                  "dspProcessing",
                                                  Qt::DirectConnection);
                    }
                }
                m_rxSamples = 0;
            }
        }
    }
    else
    {
        if (m_SyncChangedTime.elapsed() > 10) {
            set->setProtocolSync(2);
            m_SyncChangedTime.restart();
        }
    }
}

// ---------------------------------------------------------------------------
// decodeCCBytes  (private)
// ---------------------------------------------------------------------------
// Moved from DataProcessor::decodeCCBytes (cusdr_dataEngine.cpp).
// Note: emits IFrameDecoder::keyerEvent instead of DataProcessor::keyer_event.
// ---------------------------------------------------------------------------
void Protocol1FrameDecoder::decodeCCBytes(const QByteArray& buffer)
{
    de->io.ccRx.previous_dash = de->io.ccRx.dash;
    de->io.ccRx.previous_dot  = de->io.ccRx.dot;
    de->io.ccRx.ptt    = (bool)((buffer.at(0) & 0x01) == 0x01);
    de->io.ccRx.dash   = (bool)((buffer.at(0) & 0x02) == 0x02);
    de->io.ccRx.dot    = (bool)((buffer.at(0) & 0x04) == 0x04);
    de->io.ccRx.lt2208 = (bool)((buffer.at(1) & 0x01) == 0x01);

    if (de->io.ccRx.dash != de->io.ccRx.previous_dash)
        emit keyerEvent(0, de->io.ccRx.dash);
    if (de->io.ccRx.dot != de->io.ccRx.previous_dot)
        emit keyerEvent(1, de->io.ccRx.dot);

    de->io.ccRx.roundRobin = (uchar)(buffer.at(0) >> 3);

    switch (de->io.ccRx.roundRobin)
    {
        case 0:
            if (de->io.ccRx.lt2208) {
                if (m_ADCChangedTime.elapsed() > 50) {
                    set->setADCOverflow(2);
                    m_ADCChangedTime.restart();
                }
            }

            if (m_hwInterface == QSDR::Hermes) {
                de->io.ccRx.hermesI01 = (bool)((buffer.at(1) & 0x02) == 0x02);
                de->io.ccRx.hermesI02 = (bool)((buffer.at(1) & 0x04) == 0x04);
                de->io.ccRx.hermesI03 = (bool)((buffer.at(1) & 0x08) == 0x08);
                de->io.ccRx.hermesI04 = (bool)((buffer.at(1) & 0x10) == 0x10);
            }

            if (m_fwCount < 100) {
                if (m_hwInterface == QSDR::Metis) {
                    if (de->io.ccRx.devices.mercuryFWVersion != (uchar)buffer.at(2)) {
                        de->io.ccRx.devices.mercuryFWVersion = buffer.at(2);
                        set->setMercuryVersion(de->io.ccRx.devices.mercuryFWVersion);
                    }
                    if (de->io.ccRx.devices.penelopeFWVersion != (uchar)buffer.at(3)) {
                        de->io.ccRx.devices.penelopeFWVersion = buffer.at(3);
                        de->io.ccRx.devices.pennylaneFWVersion = buffer.at(3);
                        set->setPenelopeVersion(de->io.ccRx.devices.penelopeFWVersion);
                        set->setPennyLaneVersion(de->io.ccRx.devices.penelopeFWVersion);
                    }
                    if (de->io.ccRx.devices.metisFWVersion != (uchar)buffer.at(4)) {
                        de->io.ccRx.devices.metisFWVersion = buffer.at(4);
                        set->setMetisVersion(de->io.ccRx.devices.metisFWVersion);
                    }
                } else if (m_hwInterface == QSDR::Hermes) {
                    if (de->io.ccRx.devices.hermesFWVersion != (uchar)buffer.at(4)) {
                        de->io.ccRx.devices.hermesFWVersion = buffer.at(4);
                        set->setHermesVersion(de->io.ccRx.devices.hermesFWVersion);
                    }
                }
                m_fwCount++;
            }
            break;

        case 1:
            if (set->getPenelopePresence() || (m_hwInterface == QSDR::Hermes)) {
                de->io.ccRx.ain5 = (quint16)((quint16)(buffer.at(1) << 8) + (quint16)buffer.at(2));
                de->io.penelopeForwardVolts = (qreal)(3.3 * (qreal)de->io.ccRx.ain5 / 4095.0);
                de->io.penelopeForwardPower = (qreal)(de->io.penelopeForwardVolts * de->io.penelopeForwardVolts / 0.09);
            }
            if (set->getAlexPresence()) {
                de->io.ccRx.ain1 = (quint16)((quint16)(buffer.at(3) << 8) + (quint16)buffer.at(4));
                de->io.alexForwardVolts = (qreal)(3.3 * (qreal)de->io.ccRx.ain1 / 4095.0);
                de->io.alexForwardPower = (qreal)(de->io.alexForwardVolts * de->io.alexForwardVolts / 0.09);
            }
            break;

        case 2:
            if (set->getAlexPresence()) {
                de->io.ccRx.ain2 = (quint16)((quint16)(buffer.at(1) << 8) + (quint16)buffer.at(2));
                de->io.alexReverseVolts = (qreal)(3.3 * (qreal)de->io.ccRx.ain2 / 4095.0);
                de->io.alexReversePower = (qreal)(de->io.alexReverseVolts * de->io.alexReverseVolts / 0.09);
            }
            if (set->getPenelopePresence() || (m_hwInterface == QSDR::Hermes)) {
                de->io.ccRx.ain3 = (quint16)((quint16)(buffer.at(3) << 8) + (quint16)buffer.at(4));
                de->io.ain3Volts = (qreal)(3.3 * (double)de->io.ccRx.ain3 / 4095.0);
            }
            break;

        case 3:
            if (set->getPenelopePresence() || (m_hwInterface == QSDR::Hermes)) {
                de->io.ccRx.ain4 = (quint16)((quint16)(buffer.at(1) << 8) + (quint16)buffer.at(2));
                de->io.ccRx.ain6 = (quint16)((quint16)(buffer.at(3) << 8) + (quint16)buffer.at(4));
                de->io.ain4Volts = (qreal)(3.3 * (qreal)de->io.ccRx.ain4 / 4095.0);
                if (m_hwInterface == QSDR::Hermes)
                    de->io.supplyVolts = (qreal)((qreal)de->io.ccRx.ain6 / 186.0f);
            }
            break;
    }
}

// ---------------------------------------------------------------------------
// IFrameDecoder::encodeControlBytes
// ---------------------------------------------------------------------------
// Moved from DataProcessor::encodeCCBytes (cusdr_dataEngine.cpp).
// Fills de->io.output_buffer[0..2] with SYNC bytes and
// de->io.output_buffer[3..7] with de->io.control_out[0..4].
// Implements an 8-state round-robin:
//   0 - general config  1 - TX frequency
//   2 - RX frequency    3 - drive / filter board config
//   4 (Alex case)       5 - CW internal
//   6 - CW hang/tone    7 - CW keyer params
// ---------------------------------------------------------------------------
void Protocol1FrameDecoder::encodeControlBytes()
{
    de->io.output_buffer[0] = SYNC;
    de->io.output_buffer[1] = SYNC;
    de->io.output_buffer[2] = SYNC;

    de->io.mutex.lock();

    switch (m_sendState)
    {
        case 0:
        {
            uchar rxAnt, rxOut, ant;

            de->io.control_out[0] = 0x0;
            de->io.control_out[1] = 0x0;
            de->io.control_out[2] = 0x0;
            de->io.control_out[3] = 0x0;
            de->io.control_out[4] = 0x0;

            de->io.control_out[1] |= de->io.speed;
            de->io.control_out[1] &= 0x03;
            de->io.control_out[1] |= de->io.ccTx.clockByte;

            de->io.control_out[2] = de->io.rxClass;

            if (de->io.ccTx.pennyOCenabled) {
                de->io.control_out[2] &= 0x1;
                if (de->io.ccTx.currentBand != (HamBand)gen) {
                    if (de->io.ccTx.mox || de->io.ccTx.ptt)
                        de->io.control_out[2] |= (de->io.ccTx.txJ6pinList.at(de->io.ccTx.currentBand) >> 1) << 1;
                    else
                        de->io.control_out[2] |= (de->io.ccTx.rxJ6pinList.at(de->io.ccTx.currentBand) >> 1) << 1;
                }
            }

            rxAnt = 0x07 & (de->io.ccTx.alexStates.at(de->io.ccTx.currentBand) >> 2);
            rxOut = (rxAnt > 0) ? 1 : 0;

            de->io.control_out[3]  = (de->io.ccTx.alexStates.at(de->io.ccTx.currentBand) >> 7);
            de->io.control_out[3] &= 0xFB;
            de->io.control_out[3] |= (de->io.ccTx.mercuryAttenuator << 2);
            de->io.control_out[3] &= 0xF7;
            de->io.control_out[3] |= (de->io.ccTx.dither << 3);
            de->io.control_out[3] &= 0xEF;
            de->io.control_out[3] |= (de->io.ccTx.random << 4);
            de->io.control_out[3] &= 0x9F;
            de->io.control_out[3] |= rxAnt << 5;
            de->io.control_out[3] &= 0x7F;
            de->io.control_out[3] |= rxOut << 7;

            if (de->io.ccTx.mox || de->io.ccTx.ptt)
                ant = (de->io.ccTx.alexStates.at(de->io.ccTx.currentBand) >> 5);
            else
                ant = de->io.ccTx.alexStates.at(de->io.ccTx.currentBand);

            de->io.control_out[4] |= (ant != 0) ? ant - 1 : ant;
            de->io.control_out[4] &= 0xFB;
            de->io.control_out[4] |= de->io.ccTx.duplex << 2;
            de->io.control_out[4] &= 0x07;
            de->io.control_out[4] |= (de->io.receivers - 1) << 3;

            m_sendState = 1;
            break;
        }

        case 1:
        {
            de->io.control_out[0] = 0x2;
            long txfrequency = de->io.ccTx.txFrequency;
            de->io.control_out[1] = (txfrequency >> 24);
            de->io.control_out[2] = (txfrequency >> 16);
            de->io.control_out[3] = (txfrequency >> 8);
            de->io.control_out[4] =  txfrequency;
            de->io.tx_freq_change  = -1;
            m_sendState = 2;
            break;
        }

        case 2:
        {
            // RX frequency round-robin counter – forces all RX freqs on startup
            if (m_firstTimeRxInit) {
                m_firstTimeRxInit -= 1;
                de->io.rx_freq_change = m_firstTimeRxInit;
            }

            if (de->io.rx_freq_change >= 0) {
                de->io.control_out[0] = (de->io.rx_freq_change + 2) << 1;
                de->io.control_out[1] = de->RX.at(de->io.rx_freq_change)->getCtrFrequency() >> 24;
                de->io.control_out[2] = de->RX.at(de->io.rx_freq_change)->getCtrFrequency() >> 16;
                de->io.control_out[3] = de->RX.at(de->io.rx_freq_change)->getCtrFrequency() >> 8;
                de->io.control_out[4] = de->RX.at(de->io.rx_freq_change)->getCtrFrequency();
                de->io.rx_freq_change = -1;
            }
            m_sendState = 3;
            break;
        }

        case 3:
        {
            de->io.control_out[0] = 0x12;
            de->io.control_out[1] = (uchar) de->io.ccTx.drivelevel;
            de->io.control_out[2] = 0x10;
            de->io.control_out[3] = 0x0;
            de->io.control_out[4] = 0x0;
            de->io.control_out[2] &= 0xBF;

            // Alex LPF selection while transmitting
            if (de->io.ccTx.mox || de->io.ccTx.ptt) {
                double txFrequency = de->io.ccTx.txFrequency;
                if      (txFrequency > 35600000L) { de->io.control_out[4] = 0x10; }
                else if (txFrequency > 24000000L) { de->io.control_out[4] = 0x20; }
                else if (txFrequency > 16500000L) { de->io.control_out[4] = 0x40; }
                else if (txFrequency >  8000000L) { de->io.control_out[4] = 0x01; }
                else if (txFrequency >  5000000L) { de->io.control_out[4] = 0x02; }
                else if (txFrequency >  2500000L) { de->io.control_out[4] = 0x04; }
                else                              { de->io.control_out[4] = 0x08; }
            } else {
                de->io.control_out[4] = 0;
            }

            // Check if ADC routing has changed
            m_new_adc_rx1_4 = m_new_adc_rx5_8 = m_new_adc_rx9_16 = 0;
            for (int i = 0; i < set->getNumberOfReceivers(); i++) {
                if      (i < 4)  m_new_adc_rx1_4  |= de->RX.at(i)->getADCMode() << (i * 2);
                else if (i < 8)  m_new_adc_rx5_8  |= de->RX.at(i)->getADCMode() << ((i - 4) * 2);
                else if (i < 16) m_new_adc_rx9_16 |= de->RX.at(i)->getADCMode() << (i - 8);
            }
            if ((m_new_adc_rx1_4  != m_adc_rx1_4)  ||
                (m_new_adc_rx5_8  != m_adc_rx5_8)  ||
                (m_new_adc_rx9_16 != m_adc_rx9_16))
                m_sendState = 4;
            else
                m_sendState = 5;
            break;
        }

        case 4:
        {
            m_adc_rx1_4  = m_new_adc_rx1_4;
            m_adc_rx5_8  = m_new_adc_rx5_8;
            m_adc_rx9_16 = m_new_adc_rx9_16;

            de->io.control_out[0] = 0x1C;
            de->io.control_out[1] = m_adc_rx1_4;
            de->io.control_out[2] = m_adc_rx5_8;
            de->io.control_out[3] = 0x0;
            de->io.control_out[4] = m_adc_rx9_16;

            m_sendState = 5;
            break;
        }

        case 5:
        {
            de->io.control_out[0] = 0x1e;
            de->io.control_out[1] = 0x00;

            if ((de->io.ccTx.mode == DSPMode::CWU || de->io.ccTx.mode == DSPMode::CWL) &&
                de->m_internal_cw &&
                !(de->m_radioState == RadioState::MOX))
            {
                de->io.control_out[1] |= 0x01;
            }
            de->io.control_out[2] = (de->m_cw_sidetone_volume & 0xff);
            de->io.control_out[3] = (de->m_cw_ptt_delay & 0xff);
            de->io.control_out[4] = 0x0;
            m_sendState = 6;
            break;
        }

        case 6:
        {
            de->io.control_out[0] = 0x20;
            de->io.control_out[1] = (de->m_cw_hang_time >> 2) & 0xff;
            de->io.control_out[2] = (de->m_cw_hang_time & 0x03);
            de->io.control_out[3] = (de->m_cw_sidetone_freq >> 4) & 0x3f;
            de->io.control_out[4] = (de->m_cw_sidetone_freq & 0x0f);
            m_sendState = 7;
            break;
        }

        case 7:
        {
            de->io.control_out[0] = 0x16;
            de->io.control_out[1] = 0;
            de->io.control_out[2] = (de->m_cw_key_reversed) << 6;
            de->io.control_out[3] = (de->m_cw_keyer_speed & 0x3f);
            de->io.control_out[3] |= ((de->m_cw_keyer_mode & 0x03) << 6);
            de->io.control_out[4] = (de->m_cw_keyer_weight & 0x7f);
            m_sendState = 0;
            break;
        }
    }

    // Apply MOX / CW mode bit to C0
    if (de->io.ccTx.mode == DSPMode::CWU || de->io.ccTx.mode == DSPMode::CWL)
        de->io.control_out[0] &= ~0x01;
    else if (de->io.ccTx.mox || de->io.ccTx.ptt)
        de->io.control_out[0] |= 0x01;
    else
        de->io.control_out[0] &= ~0x01;

    // Copy control bytes into output buffer (positions 3..7)
    for (int i = 0; i < 5; i++)
        de->io.output_buffer[i + 3] = de->io.control_out[i];

    de->io.mutex.unlock();
}
