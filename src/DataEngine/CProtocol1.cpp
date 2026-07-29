#include "CProtocol1.h"
#include "cusdr_dataEngine.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "Models/RadioTelemetry.h"
#include "protocol_boundary_utils.h"
#include <QtEndian>
#include <cmath>

CProtocol1::CProtocol1()
    : m_adc_rx1_4(0)
    , m_adc_rx5_8(0)
    , m_adc_rx9_16(0)
    , m_new_adc_rx1_4(0)
    , m_new_adc_rx5_8(0)
    , m_new_adc_rx9_16(0)
    , m_firstTimeRxInit(0)
    , m_rxSamples(0)
    , m_fwCount(0)
{
    m_metisGetDataSignature.resize(3);
    m_metisGetDataSignature[0] = (char)0xEF;
    m_metisGetDataSignature[1] = (char)0xFE;
    m_metisGetDataSignature[2] = (char)0x01;

    m_deviceSendDataSignature.resize(4);
    m_deviceSendDataSignature[0] = (char)0xEF;
    m_deviceSendDataSignature[1] = (char)0xFE;
    m_deviceSendDataSignature[2] = (char)0x01;
    m_deviceSendDataSignature[3] = (char)0x02;
}

CProtocol1::~CProtocol1() {}

bool CProtocol1::isPacketValid(const unsigned char* data, int len) {
    return ProtocolBoundaryUtils::isProtocol1MetisPacketValid(data, len);
}

uint32_t CProtocol1::getSequence(const unsigned char* data) {
    return ProtocolBoundaryUtils::protocol1Sequence(data);
}

int CProtocol1::getPacketType(const unsigned char* data) {
    return (int)data[3];
}

QList<quint16> CProtocol1::getRequiredPorts() {
    return { (quint16)Settings::instance()->getMetisPort() };
}

void CProtocol1::processInputBuffer(const QByteArray& buffer, DataEngine* de, quint16 sourcePort) {
    Q_UNUSED(sourcePort)
    int s = 0;
    int maxSamples;

    if (buffer.at(s++) == ProtocolBoundaryUtils::kSyncByte && buffer.at(s++) == ProtocolBoundaryUtils::kSyncByte && buffer.at(s++) == ProtocolBoundaryUtils::kSyncByte)
    {
        // extract C&C bytes
        decodeCCBytes(buffer.mid(3, 5), de);
        s += 5;

        switch (de->receivers)
        {
            case 1: maxSamples = 512-0;  break;
            case 2: maxSamples = 512-0;  break;
            case 3: maxSamples = 512-4;  break;
            case 4: maxSamples = 512-10; break;
            case 5: maxSamples = 512-24; break;
            case 6: maxSamples = 512-10; break;
            case 7: maxSamples = 512-20; break;
            case 8: maxSamples = 512-4;  break;
            case 9: maxSamples = 512-0;  break;
            case 10: maxSamples = 512-8;  break;
            case 11: maxSamples = 512-28;  break;
            case 12: maxSamples = 512-60;  break;
            case 13: maxSamples = 512-24;  break;
            case 14: maxSamples = 512-74;  break;
            case 15: maxSamples = 512-44;  break;
            case 16: maxSamples = 512-14;  break;
            case 17: maxSamples = 512-88;  break;
            case 18: maxSamples = 512-64;  break;
            case 19: maxSamples = 512-40;  break;
            case 20: maxSamples = 512-16;  break;
            default: maxSamples = 512; break;
        }

        const int availableReceivers = de->RX.size();
        const int activeReceivers = qMin(de->receivers, availableReceivers);
        if (activeReceivers <= 0) {
            return;
        }

        // extract the samples
        while (s < maxSamples)
        {
            const unsigned char* p = reinterpret_cast<const unsigned char*>(buffer.constData()) + s;
            // extract each of the receivers
            for (int r = 0; r < de->receivers; r++)
            {
                m_leftSample = ProtocolBoundaryUtils::decode24BitBE(p);
                p += 3;
                m_rightSample = ProtocolBoundaryUtils::decode24BitBE(p);
                p += 3;

                if (r < activeReceivers && de->RX.at(r)->qtwdsp) {
                    de->RX[r]->m_rawIQ[m_rxSamples * 2] = m_leftSample;
                    de->RX[r]->m_rawIQ[m_rxSamples * 2 + 1] = m_rightSample;
                }
            }
            s += de->receivers * 6;

            m_micSample = ProtocolBoundaryUtils::decode16BitBE(p);
            p += 2;
            s += 2;
            m_micSample_float = (float) m_micSample / 32767.0f * de->mic_gain; // 16 bit sample

            m_rxSamples++;

            // when we have enough rx samples we start the DSP processing.
            if (m_rxSamples == BUFFER_SIZE) {
                for (int r = 0; r < activeReceivers; r++) {
                    if (de->RX.at(r)->qtwdsp) {
                        de->RX[r]->enqueueRawData();
                        QMetaObject::invokeMethod(de->RX.at(r), "dspProcessing", Qt::QueuedConnection);
                    }
                }
                m_rxSamples = 0;
            }
        }
    }
}

void CProtocol1::decodeCCBytes(const QByteArray& buffer, DataEngine* de) {
    Settings* set = Settings::instance();
    de->ccRx.previous_dash = de->ccRx.dash;
    de->ccRx.previous_dot = de->ccRx.dot;
	de->ccRx.ptt    = (bool)((buffer.at(0) & 0x01) == 0x01);
	de->ccRx.dash   = (bool)((buffer.at(0) & 0x02) == 0x02);
	de->ccRx.dot    = (bool)((buffer.at(0) & 0x04) == 0x04);
	de->ccRx.lt2208 = (bool)((buffer.at(1) & 0x01) == 0x01);

	de->ccRx.roundRobin = (uchar)(buffer.at(0) >> 3);
	
    switch (de->ccRx.roundRobin) // cycle through C0
	{
		case 0:
			if (de->ccRx.lt2208) // check ADC signal
			{
                if (RadioTelemetry* tel = telemetryFromSettings()) {
                    tel->setADCOverflow(2);
                }
			}

			if (set->getHWInterface() == QSDR::Hermes)
			{
				de->ccRx.hermesI01 = (bool)((buffer.at(1) & 0x02) == 0x02);
				de->ccRx.hermesI02 = (bool)((buffer.at(1) & 0x04) == 0x04);
				de->ccRx.hermesI03 = (bool)((buffer.at(1) & 0x08) == 0x08);
				de->ccRx.hermesI04 = (bool)((buffer.at(1) & 0x10) == 0x10);
			}

			if (m_fwCount < 100)
			{
				const unsigned char verMercury = static_cast<unsigned char>(buffer.at(2));
				const unsigned char verPenny = static_cast<unsigned char>(buffer.at(3));
				const unsigned char verBoard = static_cast<unsigned char>(buffer.at(4));
				const bool hermesBoard = (de->hpsdrDeviceName == QLatin1String("Hermes"));

				if (set->getHWInterface() == QSDR::Metis)
				{
					// Re-publish into Settings even when io cache already matches —
					// getFirmwareVersions() zeroes Settings without always matching io.
					if (verMercury && (de->ccRx.devices.mercuryFWVersion != verMercury
							   || set->getMercuryVersion() == 0)) {
						de->ccRx.devices.mercuryFWVersion = verMercury;
						set->setMercuryVersion(verMercury);
					}

					if (verPenny && (de->ccRx.devices.penelopeFWVersion != verPenny
							 || set->getPenelopeVersion() == 0)) {
						de->ccRx.devices.penelopeFWVersion = verPenny;
						de->ccRx.devices.pennylaneFWVersion = verPenny;
						set->setPenelopeVersion(verPenny);
						set->setPennyLaneVersion(verPenny);
					}

					if (verBoard && (de->ccRx.devices.metisFWVersion != verBoard
							 || set->getMetisVersion() == 0)) {
						de->ccRx.devices.metisFWVersion = verBoard;
						set->setMetisVersion(verBoard);
					}
				}

				// Hermes / ANAN-10 board FW is always in C4. Publish hermesFW for the
				// IQ probe even when the UI HW interface is still set to Metis.
				if ((set->getHWInterface() == QSDR::Hermes || hermesBoard) && verBoard
					&& (de->ccRx.devices.hermesFWVersion != verBoard
					    || set->getHermesVersion() == 0)) {
					de->ccRx.devices.hermesFWVersion = verBoard;
					set->setHermesVersion(verBoard);
				}
				m_fwCount++;
			}
			break;

		case 1: {
			const bool hermes = (set->getHWInterface() == QSDR::Hermes);
			const auto cal = ProtocolBoundaryUtils::paBridgeCalForHermes(hermes);
			if (set->getPenelopePresence() || hermes) {
				de->ccRx.ain5 = ProtocolBoundaryUtils::decodeAin12BitBE(buffer, 1);
				de->penelopeForwardVolts = ProtocolBoundaryUtils::ain12ToVolts(de->ccRx.ain5, cal.vref);
				de->penelopeForwardPower = ProtocolBoundaryUtils::wattsFromAin12(de->ccRx.ain5, cal);
			}
			// Must mask to 12 bits: unmasked 0xFFFF → ~52.8 V → ~30 kW.
			de->ccRx.ain1 = ProtocolBoundaryUtils::decodeAin12BitBE(buffer, 3);
			de->alexForwardVolts = ProtocolBoundaryUtils::ain12ToVolts(de->ccRx.ain1, cal.vref);
			de->alexForwardPower = ProtocolBoundaryUtils::wattsFromAin12(de->ccRx.ain1, cal);

			if (RadioTelemetry* tel = telemetryFromSettings()) {
				// Alex AIN1 when enabled (ANAN-10 etc.). Fall back to AIN5 only if AIN1 is zero (pihpsdr).
				quint16 fwdCode = 0;
				if (set->getAlexPresence()) {
					fwdCode = de->ccRx.ain1;
					if (fwdCode == 0 && (set->getPenelopePresence() || hermes))
						fwdCode = de->ccRx.ain5;
				} else if (set->getPenelopePresence() || hermes) {
					fwdCode = de->ccRx.ain5;
				}
				tel->setForwardPower(ProtocolBoundaryUtils::wattsFromAin12(fwdCode, cal));
			}
			break;
		}

		case 2: {
			const bool hermes = (set->getHWInterface() == QSDR::Hermes);
			const auto cal = ProtocolBoundaryUtils::paBridgeCalForHermes(hermes);
			if (set->getAlexPresence()) {
				de->ccRx.ain2 = ProtocolBoundaryUtils::decodeAin12BitBE(buffer, 1);
				de->alexReverseVolts = ProtocolBoundaryUtils::ain12ToVolts(de->ccRx.ain2, cal.vref);
				de->alexReversePower = ProtocolBoundaryUtils::wattsFromAin12(de->ccRx.ain2, cal);
				if (RadioTelemetry* tel = telemetryFromSettings()) {
					tel->setReversePower(de->alexReversePower);
					const bool tx = set->getRadioState() == RadioState::MOX
						|| set->getRadioState() == RadioState::TUNE;
					tel->setSWR(ProtocolBoundaryUtils::swrFromFwdRevWatts(
						de->alexForwardPower, de->alexReversePower, tx));
				}
			}
			if (set->getPenelopePresence() || hermes) {
				de->ccRx.ain3 = ProtocolBoundaryUtils::decodeAin12BitBE(buffer, 3);
				de->ain3Volts = ProtocolBoundaryUtils::ain12ToVolts(de->ccRx.ain3, cal.vref);
			}
			break;
		}

		case 3:
			if (set->getPenelopePresence() || (set->getHWInterface() == QSDR::Hermes)) {
				de->ccRx.ain4 = ProtocolBoundaryUtils::decodeAin12BitBE(buffer, 1);
				de->ccRx.ain6 = ProtocolBoundaryUtils::decodeAin12BitBE(buffer, 3);
				de->ain4Volts = ProtocolBoundaryUtils::ain12ToVolts(de->ccRx.ain4);
				if (set->getHWInterface() == QSDR::Hermes)
					de->supplyVolts = (qreal)((qreal)de->ccRx.ain6 / 186.0f);
			}
			break;
	}
}

void CProtocol1::encodeCCBytes(unsigned char* buffer, DataEngine* de, RadioModel* radioModel, int& sendState, quint16& port) {
    port = ProtocolBoundaryUtils::Ports::DevicePort;
    Settings* set = Settings::instance();
    buffer[0] = ProtocolBoundaryUtils::kSyncByte;
    buffer[1] = ProtocolBoundaryUtils::kSyncByte;
    buffer[2] = ProtocolBoundaryUtils::kSyncByte;

    QMutexLocker locker(&de->mutex);
    switch (sendState) {
    	case 0:
    	    {
    		uchar rxAnt;
    		uchar rxOut;
    		uchar ant;
            de->control_out[0] = 0x0; // C0
    		de->control_out[1] = 0x0; // C1
    		de->control_out[2] = 0x0; // C2
    		de->control_out[3] = 0x0; // C3
    		de->control_out[4] = 0x0; // C4
			// C0
    		// 0 0 0 0 0 0 0 0
    		//               |
    		//               +------------ MOX (1 = active, 0 = inactive)

    		// set C1
    		//
    		// 0 0 0 0 0 0 0 0
    		// | | | | | | | |
    		// | | | | | | + +------------ Speed (00 = 48kHz, 01 = 96kHz, 10 = 192kHz)
    		// | | | | + +---------------- 10MHz Ref. (00 = Atlas/Excalibur, 01 = Penelope, 10 = Mercury)*
    		// | | | +-------------------- 122.88MHz source (0 = Penelope, 1 = Mercury)*
    		// | + +---------------------- Config (00 = nil, 01 = Penelope, 10 = Mercury, 11 = both)*
    		// +-------------------------- Mic source (0 = Janus, 1 = Penelope)*
    		//
   			// * Ignored by Hermes
    		de->control_out[1] |= (radioModel && radioModel->sampleRate() > 0)
    		                        ? ((radioModel->sampleRate() == 192000) ? 2
    		                            : (radioModel->sampleRate() == 96000) ? 1
    		                            : (radioModel->sampleRate() == 384000) ? 3
    		                            : 0)
    		                        : de->speed; // sample rate
    		de->control_out[1] &= 0x03; // 0 0 0 0 0 0 1 1
    		de->control_out[1] |= de->ccTx.clockByte;

			   		// set C2
    		//
    		// 0 0 0 0 0 0 0 0
    		// |           | |
    		// |           | +------------ Mode (1 = Class E, 0 = All other modes)
    		// +---------- +-------------- Open Collector Outputs on Penelope or Hermes (bit 6...bit 0)

    		de->control_out[2] = de->rxClass;
    		if (de->ccTx.pennyOCenabled) {
    			de->control_out[2] &= 0x1; // 0 0 0 0 0 0 0 1
    			if (de->ccTx.currentBand != (HamBand) gen) {
    				if (de->ccTx.mox || de->ccTx.ptt)
    					de->control_out[2] |= (de->ccTx.txJ6pinList.at(de->ccTx.currentBand) >> 1) << 1;
    				else
    					de->control_out[2] |= (de->ccTx.rxJ6pinList.at(de->ccTx.currentBand) >> 1) << 1;
    			}
    		}
			// set C3
    		//
    		// 0 0 0 0 0 0 0 0
    		// | | | | | | | |
    		// | | | | | | + +------------ Alex Attenuator (00 = 0dB, 01 = 10dB, 10 = 20dB, 11 = 30dB)
    		// | | | | | +---------------- Preamp On/Off (0 = Off, 1 = On)
    		// | | | | +------------------ LT2208 Dither (0 = Off, 1 = On)
    		// | | | + ------------------- LT2208 Random (0= Off, 1 = On)
    		// | + + --------------------- Alex Rx Antenna (00 = none, 01 = Rx1, 10 = Rx2, 11 = XV)
    		// + ------------------------- Alex Rx out (0 = off, 1 = on). Set if Alex Rx Antenna > 00.
    		// alexStates bits [1:0] = RX antenna; bits [4:2] = RX aux (see cusdr_alexAntennaWidget.cpp)
    		rxAnt = 0x03 & de->ccTx.alexStates.at(de->ccTx.currentBand);
    		rxOut = (rxAnt > 0) ? 1 : 0;
    		// Bits [1:0]: step attenuator value (0=0dB, 1=10dB, 2=20dB, 3=30dB).
    		// Source: mercuryAttenuator (main-window Attn menu), which overrides any
    		// Alex-att contribution from alexStates bits [8:7] (used when an Alex board
    		// is present).  Both default to 0 so the behaviour is unchanged when neither
    		// is set.
    		de->control_out[3] = (de->ccTx.alexStates.at(de->ccTx.currentBand) >> 7) & 0x03;
    		de->control_out[3] |= (de->ccTx.mercuryAttenuator & 0x03);  // step-att at bits [1:0]
    		// Bit 2: Preamp/LNA on when no step attenuation and not transmitting.
    		// C3 bit2=1 = preamp on (full sensitivity), bit2=0 = preamp off (-20 dB).
    		de->control_out[3] &= 0xFB; // 1 1 1 1 1 0 1 1 — clear bit 2
    		bool txActive = de->ccTx.mox || de->ccTx.ptt;
    		de->control_out[3] |= ((!txActive && de->ccTx.mercuryAttenuator == 0) ? 1 : 0) << 2;
    		de->control_out[3] &= 0xF7; // 1 1 1 1 0 1 1 1
    		de->control_out[3] |= (de->ccTx.dither << 3);
    		de->control_out[3] &= 0xEF; // 1 1 1 0 1 1 1 1
    		de->control_out[3] |= (de->ccTx.random << 4);
    		de->control_out[3] &= 0x9F; // 1 0 0 1 1 1 1 1
    		de->control_out[3] |= rxAnt << 5;
    		de->control_out[3] &= 0x7F; // 0 1 1 1 1 1 1 1
    		de->control_out[3] |= rxOut << 7;

            if (de->ccTx.mox || de->ccTx.ptt)
    			ant = (de->ccTx.alexStates.at(de->ccTx.currentBand) >> 5);
    		else
    			ant = de->ccTx.alexStates.at(de->ccTx.currentBand);

				// set C4
    		//
    		// 0 0 0 0 0 0 0 0
    		// | | | | | | | |
    		// | | | | | | + + ----------- Alex Tx relay (00 = Tx1, 01= Tx2, 10 = Tx3)
    		// | | | | | + --------------- Duplex (0 = off, 1 = on)
    		// + + + + +------------------ Number of Receivers (000 = 1, 11111 = 32)

                //RRK removed 4HL
            // | +------------------------ Time stamp - 1PPS on LSB of Mic data (0 = off, 1 = on)
    		// +-------------------------- Common Mercury Frequency (0 = independent frequencies to Mercury
    		//			                   Boards, 1 = same frequency to all Mercury boards)

    		de->control_out[4] |= (ant != 0) ? ant-1 : ant;
    		de->control_out[4] &= 0xFB; // 1 1 1 1 1 0 1 1
    		de->control_out[4] |= de->ccTx.duplex << 2;
    		de->control_out[4] &= 0x07; // 0 0 0 0 0 1 1 1
    		// slices() is preallocated (8); use active receiver count for hardware mux.
    		int rcvrCount = de->receivers > 0 ? de->receivers : 1;
    		de->control_out[4] |= (rcvrCount - 1) << 3;

    		sendState = 1;
    		break;
    	    }

    	case 1:
            {
			// C0
    		// 0 0 0 0 0 0 1 x     C1, C2, C3, C4 NCO Frequency in Hz for Transmitter, Apollo ATU
    		//                     (32 bit binary representation - MSB in C1)
                de->control_out[0] = 0x2; // C0
                long txfrequency = de->ccTx.txFrequency;
                // TX dial frequency = VFO, not panadapter center.
                if (radioModel && !radioModel->slices().isEmpty()) {
                    txfrequency = radioModel->slices().at(0)->frequency();
                }
                de->control_out[1] = (txfrequency >> 24);
                de->control_out[2] = (txfrequency >> 16);
                de->control_out[3] = (txfrequency >> 8);
                de->control_out[4] = txfrequency;
                de->tx_freq_change = -1;
            }
            sendState = 2;
    		break;

    	case 2:
            {
			// C0 = 0 0 0 0 0 1 0 x … Receiver NCO — must be panadapter CENTER frequency.
    		if (m_firstTimeRxInit) {
    			m_firstTimeRxInit -= 1;
    			de->rx_freq_change = m_firstTimeRxInit;
    		}
            if (de->rx_freq_change >= 0) {
                int rxIdx = de->rx_freq_change;
                const QList<qint64> freqs = set->getCtrFrequencies();
                qint64 freq = 7050000;
                if (radioModel && rxIdx >= 0 && rxIdx < radioModel->slices().count()) {
                    freq = radioModel->slices().at(rxIdx)->centerFrequency();
                } else if (rxIdx >= 0 && rxIdx < freqs.count()) {
                    freq = freqs.at(rxIdx);
                }
                de->control_out[0] = (rxIdx + 2) << 1;
                de->control_out[1] = (freq >> 24) & 0xFF;
                de->control_out[2] = (freq >> 16) & 0xFF;
                de->control_out[3] = (freq >> 8) & 0xFF;
                de->control_out[4] = freq & 0xFF;
                de->rx_freq_change = -1;
            }
            }
    		sendState = 3;
    		break;

    	case 3:
	// C1
    		// 0 0 0 0 0 0 0 0
    		// |             |
    		// +-------------+------------ Hermes/PennyLane Drive Level (0-255) (ignored by Penelope)


    		// C2
    		// 0 0 0 0 0 0 0 0
    		// | | | | | | | |
    		// | | | | | | | +------------ Hermes/Metis Penelope Mic boost (0 = 0dB, 1 = 20dB)
    		// | | | | | | +-------------- Metis/Penelope or PennyLane Mic/Line-in (0 = mic, 1 = Line-in)
            // | | | | | +---------------- Hermes - Enable/disable Apollo filter (0 = disable, 1 = enable)
            // | | | | +------------------ Hermes - Enable/disable Apollo tuner (0 = disable, 1 = enable)
            // | | | +-------------------- Hermes - Apollo auto tune (0 = end, 1 = start)
            // | | +---------------------- Hermes - select filter board (0 = Alex, 1 = Apollo)
    		// | +------------------------ Alex   - manual HPF/LPF filter select (0 = disable, 1 = enable)
    		// +-------------------------- VNA mode (0 = off, 1 = on)

    		// Alex configuration:
    		//
    		// manual 		  0

    		de->control_out[0] = 0x12; // 0 0 0 1 0 0 1 0
            de->control_out[1] = (uchar) de->ccTx.drivelevel; // C1
    		de->control_out[2] = 0x10; // C2
    		de->control_out[3] = 0x0; // C3
            de->control_out[4] = 0x0; // C4

    		// Protocol 1 TX LPF should follow TX frequency (master branch behavior).
    		// Keep C2 bit 6 cleared (auto filter select).
    		de->control_out[2] &= 0xBF; // 1 0 1 1 1 1 1 1


			// C3
    		// 0 0 0 0 0 0 0 0
    		//   | | | | | | |
    		//   | | | | | | +------------ Alex   -	select 13MHz  HPF (0 = disable, 1 = enable)*
    		//   | | | | | +-------------- Alex   -	select 20MHz  HPF (0 = disable, 1 = enable)*
    		//   | | | | +---------------- Alex   -	select 9.5MHz HPF (0 = disable, 1 = enable)*
    		//   | | | +------------------ Alex   -	select 6.5MHz HPF (0 = disable, 1 = enable)*
    		//   | | +-------------------- Alex   -	select 1.5MHz HPF (0 = disable, 1 = enable)*
    		//   | +---------------------- Alex   -	Bypass all HPFs   (0 = disable, 1 = enable)*
    		//   +------------------------ Alex   -	6M low noise amplifier (0 = disable, 1 = enable)*
    		//
    		// *Only valid when Alex - manual HPF/LPF filter select is enabled


    		de->control_out[3] &= 0xFE; // 1 1 1 1 1 1 1 0
    		de->control_out[3] |= (de->ccTx.alexConfig & 0x40) >> 6;
    		de->control_out[3] &= 0xFD; // 1 1 1 1 1 1 0 1
    		de->control_out[3] |= (de->ccTx.alexConfig & 0x80) >> 6;
    		de->control_out[3] &= 0xFB; // 1 1 1 1 1 0 1 1
    		de->control_out[3] |= (de->ccTx.alexConfig & 0x20) >> 3;
    		de->control_out[3] &= 0xF7; // 1 1 1 1 0 1 1 1
    		de->control_out[3] |= (de->ccTx.alexConfig & 0x10) >> 1;
    		de->control_out[3] &= 0xEF; // 1 1 1 0 1 1 1 1
    		de->control_out[3] |= (de->ccTx.alexConfig & 0x08) << 1;
    		de->control_out[3] &= 0xDF; // 1 1 0 1 1 1 1 1
    		de->control_out[3] |= (de->ccTx.alexConfig & 0x02) << 4;
    		de->control_out[3] &= 0xBF; // 1 0 1 1 1 1 1 1
    		de->control_out[3] |= (de->ccTx.alexConfig & 0x04) << 4;
    		de->control_out[3] &= 0x7F; // 0 1 1 1 1 1 1 1
    		de->control_out[3] |= ((int)de->ccTx.vnaMode) << 7;

            if (de->ccTx.mox || de->ccTx.ptt) {
                long txFrequency = de->ccTx.txFrequency;
                if      (txFrequency > ProtocolBoundaryUtils::kAlexLpf6mMinHz)    { de->control_out[4] = 0x10; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf12_10mMinHz) { de->control_out[4] = 0x20; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf17_15mMinHz) { de->control_out[4] = 0x40; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf30_20mMinHz) { de->control_out[4] = 0x01; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf60_40mMinHz) { de->control_out[4] = 0x02; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf80mMinHz)    { de->control_out[4] = 0x04; }
                else                                            { de->control_out[4] = 0x08; }
            } else de->control_out[4] = 0;

		m_new_adc_rx1_4 = m_new_adc_rx5_8 = m_new_adc_rx9_16 = 0;
		for (int i = 0; i < set->getNumberOfReceivers(); i++) {
			if (i < 4) m_new_adc_rx1_4 |= set->getADCMode(i) << (i * 2);
			else if (i < 8) m_new_adc_rx5_8 |= set->getADCMode(i) << ((i-4) * 2);
			else if (i < 16) m_new_adc_rx9_16 |= set->getADCMode(i) << (i-8);
		}

		if ((m_new_adc_rx1_4 != m_adc_rx1_4) || (m_new_adc_rx5_8 != m_adc_rx5_8) || (m_new_adc_rx9_16 != m_adc_rx9_16))
    			sendState = 4;
		else
    			sendState = 5;
    		break;

    	case 4:

			// C4
    		// 0 0 0 0 0 0 0 0
    		//   | | | | | | |
    		//   | | | | | | +------------ Alex   - 	select 30/20m LPF (0 = disable, 1 = enable)*
    		//   | | | | | +-------------- Alex   - 	select 60/40m LPF (0 = disable, 1 = enable)*
    		//   | | | | +---------------- Alex   - 	select 80m    LPF (0 = disable, 1 = enable)*
    		//   | | | +------------------ Alex   - 	select 160m   LPF (0 = disable, 1 = enable)*
    		//   | | +-------------------- Alex   - 	select 6m     LPF (0 = disable, 1 = enable)*
    		//   | +---------------------- Alex   - 	select 12/10m LPF (0 = disable, 1 = enable)*
    		//   +------------------------ Alex   - 	select 17/15m LPF (0 = disable, 1 = enable)*
    		//
    		// *Only valid when Alex - manual HPF/LPF filter select is enabled

		m_adc_rx1_4 = m_new_adc_rx1_4;
		m_adc_rx5_8 = m_new_adc_rx5_8;
		m_adc_rx9_16 = m_new_adc_rx9_16;
		de->control_out[0] = 0x1C; // 0 0 0 1 1 1 0 x
    	de->control_out[1] = m_adc_rx1_4; // C1
    	de->control_out[2] = m_adc_rx5_8; // C2
    	// C3 bits [4:0]: DDC/ADC input step attenuator (0-31 dB).
    	// During TX force 30 dB to protect the RX front-end.
    	de->control_out[3] = (de->ccTx.mox || de->ccTx.ptt)
    	    ? 30
    	    : (uchar)qBound(0, de->ccTx.mercuryAttenuator * 10, 31);
		de->control_out[4] = m_adc_rx9_16; // C4
        sendState = 5;
    		break;

        case 5:
            de->control_out[0] = 0x1e; // 0 0 0 1 1 1 1 x
            de->control_out[1] = 0x00;
            if((de->ccTx.mode==DSPMode::CWU || de->ccTx.mode==DSPMode::CWL)  && (set->isInternalCw()) &&!(set->getRadioState() == RadioState::MOX))
            {
                de->control_out[1]|=0x01;
            }
            de->control_out[2] = (set->getCwSidetoneVolume() & 0xff); //cw sidetone level
            de->control_out[3] = (set->getCwPttDelay() & 0xff); // ptt delay
            de->control_out[4] = 0x0;
            sendState = 6;
            break;

        case 6:
            de->control_out[0] = 0x20; // 0 0 0 1 1 1 1 x
            de->control_out[1] = (set->getCwHangTime() >> 2) & 0xff; // cw hang time bits 9:2
            de->control_out[2] = (set->getCwHangTime() & 0x03); //cw hang time 1:0
            de->control_out[3] = (set->getCwSidetoneFreq() >> 4) & 0x3f; // cw sidetone frequnecy 11:4y
            de->control_out[4] = (set->getCwSidetoneFreq() & 0x0f) ; // cw sidetone frequency 3:0;
            sendState = 7;
            break;

        case 7:
            de->control_out[0] = 0x16; // 0 0 0 1 1 1 1 x
            de->control_out[1] = 0;
            de->control_out[2] = (set->isCwKeyReversed()) << 6;
            de->control_out[3] = (set->getCwKeyerSpeed() & 0x3f);
            de->control_out[3]  |= ((set->getCwKeyerMode()  & 0x03) << 6);
            de->control_out[4] = (set->getCwKeyerWeight() & 0x7f);
            sendState = 0;
            break;
    }
    if ((de->ccTx.mode==DSPMode::CWU || de->ccTx.mode==DSPMode::CWL) )
    {
         de->control_out[0] &= ~0x01;
       }
    else  if (de->ccTx.mox || de->ccTx.ptt) de->control_out[0] |= 0x01;
    else de->control_out[0] &= ~0x01;

    for (int i = 0; i < 5; i++) {
        de->output_buffer[i + 3] = de->control_out[i];
    }
}

QByteArray CProtocol1::formatStartStop(char value, quint16& port) {
    port = ProtocolBoundaryUtils::Ports::DevicePort;
    QByteArray commandDatagram;
    commandDatagram.resize(64);
    commandDatagram[0] = (char)0xEF;
    commandDatagram[1] = (char)0xFE;
    commandDatagram[2] = (char)0x04;
    commandDatagram[3] = (char)value;
    for (int i = 4; i < 64; i++) commandDatagram[i] = 0x00;
    return commandDatagram;
}

QByteArray CProtocol1::formatInitFrame(int rx, DataEngine* de, RadioModel* radioModel, quint16& port) {
    port = ProtocolBoundaryUtils::Ports::DevicePort;
    QByteArray initDatagram;
	initDatagram.resize(1032);

	initDatagram[0] = (char)0xEF;
	initDatagram[1] = (char)0xFE;
	initDatagram[2] = (char)0x01;
	initDatagram[3] = (char)0x02;
	initDatagram[4] = (char)0x00;
	initDatagram[5] = (char)0x00;
	initDatagram[6] = (char)0x00;
	initDatagram[7] = (char)0x00;

	initDatagram[8] = ProtocolBoundaryUtils::kSyncByte;
    initDatagram[9] = ProtocolBoundaryUtils::kSyncByte;
    initDatagram[10] = ProtocolBoundaryUtils::kSyncByte;

	for (int i = 0; i < 5; i++) {
		initDatagram[i + 11]  = de->control_out[i];
	}

	for (int i = 16; i < 520; i++) {
		initDatagram[i]  = 0x00;
	}

	initDatagram[520] = ProtocolBoundaryUtils::kSyncByte;
    initDatagram[521] = ProtocolBoundaryUtils::kSyncByte;
    initDatagram[522] = ProtocolBoundaryUtils::kSyncByte;

	initDatagram[523] = de->control_out[0] | ((rx + 2) << 1);
    Settings* set = Settings::instance();
    qint64 freq = 7050000;
    // Init frame NCO = panadapter center frequency (not VFO).
    if (radioModel && rx >= 0 && rx < radioModel->slices().count()) {
        freq = radioModel->slices().at(rx)->centerFrequency();
    } else {
        const QList<qint64> freqs = set->getCtrFrequencies();
        if (rx >= 0 && rx < freqs.count()) {
            freq = freqs.at(rx);
        }
    }
	initDatagram[524] = (freq >> 24) & 0xFF;
	initDatagram[525] = (freq >> 16) & 0xFF;
	initDatagram[526] = (freq >> 8) & 0xFF;
	initDatagram[527] = freq & 0xFF;

	for (int i = 528; i < 1032; i++) initDatagram[i]  = 0x00;

    return initDatagram;
}

QByteArray CProtocol1::formatOutputPacket(const QByteArray& audioData, uint32_t& sequence) {
    QByteArray outDatagram;
    uint32_t outseq = qFromBigEndian(sequence);
    outDatagram.resize(0);
    outDatagram += m_deviceSendDataSignature;
    QByteArray seq(reinterpret_cast<const char*>(&outseq), sizeof(outseq));
    outDatagram += seq;
    outDatagram += audioData;
    sequence++;
    return outDatagram;
}
