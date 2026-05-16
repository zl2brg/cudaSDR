#include "CProtocol1.h"
#include "cusdr_dataEngine.h"
#include "protocol_boundary_utils.h"
#include <QtEndian>

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
        decodeCCBytes(buffer.mid(3, 5), &de->io);
        s += 5;

        switch (de->io.receivers)
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

        // extract the samples
        while (s < maxSamples)
        {
            const unsigned char* p = reinterpret_cast<const unsigned char*>(buffer.constData()) + s;
            // extract each of the receivers
            for (int r = 0; r < de->io.receivers; r++)
            {
                m_leftSample = ProtocolBoundaryUtils::decode24BitBE(p);
                p += 3;
                m_rightSample = ProtocolBoundaryUtils::decode24BitBE(p);
                p += 3;

                if (de->RX.at(r)->qtwdsp) {
                    de->RX[r]->m_rawIQ[m_rxSamples * 2] = m_leftSample;
                    de->RX[r]->m_rawIQ[m_rxSamples * 2 + 1] = m_rightSample;
                }
            }
            s += de->io.receivers * 6;

            m_micSample = ProtocolBoundaryUtils::decode16BitBE(p);
            p += 2;
            s += 2;
            m_micSample_float = (float) m_micSample / 32767.0f * de->io.mic_gain; // 16 bit sample

            m_rxSamples++;

            // when we have enough rx samples we start the DSP processing.
            if (m_rxSamples == BUFFER_SIZE) {
                for (int r = 0; r < de->io.receivers; r++) {
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

void CProtocol1::decodeCCBytes(const QByteArray& buffer, THPSDRParameter* io) {
    Settings* set = Settings::instance();
    io->ccRx.previous_dash = io->ccRx.dash;
    io->ccRx.previous_dot = io->ccRx.dot;
	io->ccRx.ptt    = (bool)((buffer.at(0) & 0x01) == 0x01);
	io->ccRx.dash   = (bool)((buffer.at(0) & 0x02) == 0x02);
	io->ccRx.dot    = (bool)((buffer.at(0) & 0x04) == 0x04);
	io->ccRx.lt2208 = (bool)((buffer.at(1) & 0x01) == 0x01);

	io->ccRx.roundRobin = (uchar)(buffer.at(0) >> 3);
	
    switch (io->ccRx.roundRobin) // cycle through C0
	{
		case 0:
			if (io->ccRx.lt2208) // check ADC signal
			{
                set->setADCOverflow(2);
			}

			if (set->getHWInterface() == QSDR::Hermes)
			{
				io->ccRx.hermesI01 = (bool)((buffer.at(1) & 0x02) == 0x02);
				io->ccRx.hermesI02 = (bool)((buffer.at(1) & 0x04) == 0x04);
				io->ccRx.hermesI03 = (bool)((buffer.at(1) & 0x08) == 0x08);
				io->ccRx.hermesI04 = (bool)((buffer.at(1) & 0x10) == 0x10);
			}

			if (m_fwCount < 100)
			{
				if (set->getHWInterface() == QSDR::Metis)
				{
					if (io->ccRx.devices.mercuryFWVersion != (unsigned char)buffer.at(2))
					{
                        io->ccRx.devices.mercuryFWVersion = (unsigned char)buffer.at(2);
						set->setMercuryVersion(io->ccRx.devices.mercuryFWVersion);
					}

					if (io->ccRx.devices.penelopeFWVersion != (unsigned char)buffer.at(3))
					{
						io->ccRx.devices.penelopeFWVersion = (unsigned char)buffer.at(3);
						io->ccRx.devices.pennylaneFWVersion = (unsigned char)buffer.at(3);
						set->setPenelopeVersion(io->ccRx.devices.penelopeFWVersion);
						set->setPennyLaneVersion(io->ccRx.devices.penelopeFWVersion);
					}

					if (io->ccRx.devices.metisFWVersion != (unsigned char)buffer.at(4))
					{
						io->ccRx.devices.metisFWVersion = (unsigned char)buffer.at(4);
						set->setMetisVersion(io->ccRx.devices.metisFWVersion);
					}
				}
				else if (set->getHWInterface() == QSDR::Hermes) {
					if (io->ccRx.devices.hermesFWVersion != (unsigned char)buffer.at(4)) {
						io->ccRx.devices.hermesFWVersion = (unsigned char)buffer.at(4);
						set->setHermesVersion(io->ccRx.devices.hermesFWVersion);
					}
				}
				m_fwCount++;
			}
			break;

		case 1:
			if (set->getPenelopePresence() || (set->getHWInterface() == QSDR::Hermes)) {
				io->ccRx.ain5 = (quint16)((quint16)(buffer.at(1) << 8) + (quint16)buffer.at(2));
				io->penelopeForwardVolts = (qreal)(3.3 * (qreal)io->ccRx.ain5 / 4095.0);
				io->penelopeForwardPower = (qreal)(io->penelopeForwardVolts * io->penelopeForwardVolts / 0.09);
			}
			if (set->getAlexPresence()) {
				io->ccRx.ain1 = (quint16)((quint16)(buffer.at(3) << 8) + (quint16)buffer.at(4));
				io->alexForwardVolts = (qreal)(3.3 * (qreal)io->ccRx.ain1 / 4095.0);
				io->alexForwardPower = (qreal)(io->alexForwardVolts * io->alexForwardVolts / 0.09);
				set->setForwardPower(io->alexForwardPower);
			} else if (set->getPenelopePresence() || (set->getHWInterface() == QSDR::Hermes)) {
				// No Alex: report Penelope/Hermes PA forward power from AIN5
				set->setForwardPower(io->penelopeForwardPower);
			}
            break;

		case 2:
			if (set->getAlexPresence()) {
				io->ccRx.ain2 = (quint16)((quint16)(buffer.at(1) << 8) + (quint16)buffer.at(2));
				io->alexReverseVolts = (qreal)(3.3 * (qreal)io->ccRx.ain2 / 4095.0);
				io->alexReversePower = (qreal)(io->alexReverseVolts * io->alexReverseVolts / 0.09);
				set->setReversePower(io->alexReversePower);
			}
			if (set->getPenelopePresence() || (set->getHWInterface() == QSDR::Hermes)) {
				io->ccRx.ain3 = (quint16)((quint16)(buffer.at(3) << 8) + (quint16)buffer.at(4));
				io->ain3Volts = (qreal)(3.3 * (double)io->ccRx.ain3 / 4095.0);
			}
			break;

		case 3:
			if (set->getPenelopePresence() || (set->getHWInterface() == QSDR::Hermes)) {
				io->ccRx.ain4 = (quint16)((quint16)(buffer.at(1) << 8) + (quint16)buffer.at(2));
				io->ccRx.ain6 = (quint16)((quint16)(buffer.at(3) << 8) + (quint16)buffer.at(4));
				io->ain4Volts = (qreal)(3.3 * (qreal)io->ccRx.ain4 / 4095.0);
				if (set->getHWInterface() == QSDR::Hermes)
					io->supplyVolts = (qreal)((qreal)io->ccRx.ain6 / 186.0f);
			}
			break;
	}
}

void CProtocol1::encodeCCBytes(unsigned char* buffer, THPSDRParameter* io, int& sendState, quint16& port) {
    port = ProtocolBoundaryUtils::Ports::DevicePort;
    Settings* set = Settings::instance();
    buffer[0] = ProtocolBoundaryUtils::kSyncByte;
    buffer[1] = ProtocolBoundaryUtils::kSyncByte;
    buffer[2] = ProtocolBoundaryUtils::kSyncByte;

    QMutexLocker locker(&io->mutex);
    switch (sendState) {
    	case 0:
    	    {
    		uchar rxAnt;
    		uchar rxOut;
    		uchar ant;
            io->control_out[0] = 0x0; // C0
    		io->control_out[1] = 0x0; // C1
    		io->control_out[2] = 0x0; // C2
    		io->control_out[3] = 0x0; // C3
    		io->control_out[4] = 0x0; // C4
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
    		io->control_out[1] |= io->speed; // sample rate
    		io->control_out[1] &= 0x03; // 0 0 0 0 0 0 1 1
    		io->control_out[1] |= io->ccTx.clockByte;

			   		// set C2
    		//
    		// 0 0 0 0 0 0 0 0
    		// |           | |
    		// |           | +------------ Mode (1 = Class E, 0 = All other modes)
    		// +---------- +-------------- Open Collector Outputs on Penelope or Hermes (bit 6...bit 0)

    		io->control_out[2] = io->rxClass;
    		if (io->ccTx.pennyOCenabled) {
    			io->control_out[2] &= 0x1; // 0 0 0 0 0 0 0 1
    			if (io->ccTx.currentBand != (HamBand) gen) {
    				if (io->ccTx.mox || io->ccTx.ptt)
    					io->control_out[2] |= (io->ccTx.txJ6pinList.at(io->ccTx.currentBand) >> 1) << 1;
    				else
    					io->control_out[2] |= (io->ccTx.rxJ6pinList.at(io->ccTx.currentBand) >> 1) << 1;
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
    		rxAnt = 0x07 & (io->ccTx.alexStates.at(io->ccTx.currentBand) >> 2);
    		rxOut = (rxAnt > 0) ? 1 : 0;
    		io->control_out[3] = (io->ccTx.alexStates.at(io->ccTx.currentBand) >> 7);
    		io->control_out[3] &= 0xFB; // 1 1 1 1 1 0 1 1
    		// During TX force preamp off (-20 dB) to protect the RX front-end.
    		// C3 bit2=1 = preamp on (0 dB), bit2=0 = preamp off (-20 dB).
    		bool txActive = io->ccTx.mox || io->ccTx.ptt;
    		io->control_out[3] |= ((!txActive && io->ccTx.mercuryAttenuator == 0) ? 1 : 0) << 2;
    		io->control_out[3] &= 0xF7; // 1 1 1 1 0 1 1 1
    		io->control_out[3] |= (io->ccTx.dither << 3);
    		io->control_out[3] &= 0xEF; // 1 1 1 0 1 1 1 1
    		io->control_out[3] |= (io->ccTx.random << 4);
    		io->control_out[3] &= 0x9F; // 1 0 0 1 1 1 1 1
    		io->control_out[3] |= rxAnt << 5;
    		io->control_out[3] &= 0x7F; // 0 1 1 1 1 1 1 1
    		io->control_out[3] |= rxOut << 7;

            if (io->ccTx.mox || io->ccTx.ptt)
    			ant = (io->ccTx.alexStates.at(io->ccTx.currentBand) >> 5);
    		else
    			ant = io->ccTx.alexStates.at(io->ccTx.currentBand);

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

    		io->control_out[4] |= (ant != 0) ? ant-1 : ant;
    		io->control_out[4] &= 0xFB; // 1 1 1 1 1 0 1 1
    		io->control_out[4] |= io->ccTx.duplex << 2;
    		io->control_out[4] &= 0x07; // 0 0 0 0 0 1 1 1
    		io->control_out[4] |= (io->receivers - 1) << 3;

    		sendState = 1;
    		break;
    	    }

    	case 1:
            {
			// C0
    		// 0 0 0 0 0 0 1 x     C1, C2, C3, C4 NCO Frequency in Hz for Transmitter, Apollo ATU
    		//                     (32 bit binary representation - MSB in C1)
                io->control_out[0] = 0x2; // C0
                long txfrequency = io->ccTx.txFrequency;
                io->control_out[1] = (txfrequency >> 24);
                io->control_out[2] = (txfrequency >> 16);
                io->control_out[3] = (txfrequency >> 8);
                io->control_out[4] = txfrequency;
                io->tx_freq_change = -1;
            }
            sendState = 2;
    		break;

    	case 2:

			// C0 = 0 0 0 0 0 1 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver_1
    		// C0 = 0 0 0 0 0 1 1 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _2
    		// C0 = 0 0 0 0 1 0 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _3
    		// C0 = 0 0 0 0 1 0 1 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _4
    		// C0 = 0 0 0 0 1 1 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _5
    		// C0 = 0 0 0 0 1 1 1 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _6
    		// C0 = 0 0 0 1 0 0 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _7
    		// C0 = 0 0 1 0 0 1 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _8 // Was 0 0 0 1 0 0 1 x
    		// C0 = 0 0 1 1 0 1 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _16
    		// C0 = 0 1 0 1 0 1 0 x     C1, C2, C3, C4   NCO Frequency in Hz for Receiver _32

    		if (m_firstTimeRxInit) {
    			m_firstTimeRxInit -= 1;
    			io->rx_freq_change = m_firstTimeRxInit;
    		}
            if (io->rx_freq_change >= 0) {
                io->control_out[0] = (io->rx_freq_change + 2) << 1;
                io->control_out[1] = set->getCtrFrequencies().at(io->rx_freq_change) >> 24;
                io->control_out[2] = set->getCtrFrequencies().at(io->rx_freq_change) >> 16;
                io->control_out[3] = set->getCtrFrequencies().at(io->rx_freq_change) >> 8;
                io->control_out[4] = set->getCtrFrequencies().at(io->rx_freq_change);
                io->rx_freq_change = -1;
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

    		io->control_out[0] = 0x12; // 0 0 0 1 0 0 1 0
            io->control_out[1] = (uchar) io->ccTx.drivelevel; // C1
    		io->control_out[2] = 0x10; // C2
    		io->control_out[3] = 0x0; // C3
            io->control_out[4] = 0x0; // C4

    		io->control_out[2] &= 0xBF; // 1 0 1 1 1 1 1 1


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


    		io->control_out[3] &= 0xFE; // 1 1 1 1 1 1 1 0
    		io->control_out[3] |= (io->ccTx.alexConfig & 0x40) >> 6;
    		io->control_out[3] &= 0xFD; // 1 1 1 1 1 1 0 1
    		io->control_out[3] |= (io->ccTx.alexConfig & 0x80) >> 6;
    		io->control_out[3] &= 0xFB; // 1 1 1 1 1 0 1 1
    		io->control_out[3] |= (io->ccTx.alexConfig & 0x20) >> 3;
    		io->control_out[3] &= 0xF7; // 1 1 1 1 0 1 1 1
    		io->control_out[3] |= (io->ccTx.alexConfig & 0x10) >> 1;
    		io->control_out[3] &= 0xEF; // 1 1 1 0 1 1 1 1
    		io->control_out[3] |= (io->ccTx.alexConfig & 0x08) << 1;
    		io->control_out[3] &= 0xDF; // 1 1 0 1 1 1 1 1
    		io->control_out[3] |= (io->ccTx.alexConfig & 0x02) << 4;
    		io->control_out[3] &= 0xBF; // 1 0 1 1 1 1 1 1
    		io->control_out[3] |= (io->ccTx.alexConfig & 0x04) << 4;
    		io->control_out[3] &= 0x7F; // 0 1 1 1 1 1 1 1
    		io->control_out[3] |= ((int)io->ccTx.vnaMode) << 7;

            if (io->ccTx.mox || io->ccTx.ptt) {
                long txFrequency = io->ccTx.txFrequency;
                if      (txFrequency > ProtocolBoundaryUtils::kAlexLpf6mMinHz)    { io->control_out[4] = 0x10; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf12_10mMinHz) { io->control_out[4] = 0x20; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf17_15mMinHz) { io->control_out[4] = 0x40; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf30_20mMinHz) { io->control_out[4] = 0x01; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf60_40mMinHz) { io->control_out[4] = 0x02; }
                else if (txFrequency > ProtocolBoundaryUtils::kAlexLpf80mMinHz)    { io->control_out[4] = 0x04; }
                else                                            { io->control_out[4] = 0x08; }
            } else io->control_out[4] = 0;

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
		io->control_out[0] = 0x1C; // 0 0 0 1 1 1 0 x
    	io->control_out[1] = m_adc_rx1_4; // C1
    	io->control_out[2] = m_adc_rx5_8; // C2
    	// C3 bits [4:0]: DDC/ADC input step attenuator (0-31 dB).
    	// During TX force 30 dB to protect the RX front-end.
    	io->control_out[3] = (io->ccTx.mox || io->ccTx.ptt)
    	    ? 30
    	    : (uchar)qBound(0, io->ccTx.mercuryAttenuator * 10, 31);
		io->control_out[4] = m_adc_rx9_16; // C4
        sendState = 5;
    		break;

        case 5:
            io->control_out[0] = 0x1e; // 0 0 0 1 1 1 1 x
            io->control_out[1] = 0x00;
            if((io->ccTx.mode==DSPMode::CWU || io->ccTx.mode==DSPMode::CWL)  && (set->isInternalCw()) &&!(set->getRadioState() == RadioState::MOX))
            {
                io->control_out[1]|=0x01;
            }
            io->control_out[2] = (set->getCwSidetoneVolume() & 0xff); //cw sidetone level
            io->control_out[3] = (set->getCwPttDelay() & 0xff); // ptt delay
            io->control_out[4] = 0x0;
            sendState = 6;
            break;

        case 6:
            io->control_out[0] = 0x20; // 0 0 0 1 1 1 1 x
            io->control_out[1] = (set->getCwHangTime() >> 2) & 0xff; // cw hang time bits 9:2
            io->control_out[2] = (set->getCwHangTime() & 0x03); //cw hang time 1:0
            io->control_out[3] = (set->getCwSidetoneFreq() >> 4) & 0x3f; // cw sidetone frequnecy 11:4y
            io->control_out[4] = (set->getCwSidetoneFreq() & 0x0f) ; // cw sidetone frequency 3:0;
            sendState = 7;
            break;

        case 7:
            io->control_out[0] = 0x16; // 0 0 0 1 1 1 1 x
            io->control_out[1] = 0;
            io->control_out[2] = (set->isCwKeyReversed()) << 6;
            io->control_out[3] = (set->getCwKeyerSpeed() & 0x3f);
            io->control_out[3]  |= ((set->getCwKeyerMode()  & 0x03) << 6);
            io->control_out[4] = (set->getCwKeyerWeight() & 0x7f);
            sendState = 0;
            break;
    }
    if ((io->ccTx.mode==DSPMode::CWU || io->ccTx.mode==DSPMode::CWL) )
    {
         io->control_out[0] &= ~0x01;
       }
    else  if (io->ccTx.mox || io->ccTx.ptt) io->control_out[0] |= 0x01;
    else io->control_out[0] &= ~0x01;

    for (int i = 0; i < 5; i++) {
        io->output_buffer[i + 3] = io->control_out[i];
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

QByteArray CProtocol1::formatInitFrame(int rx, THPSDRParameter* io, quint16& port) {
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
		initDatagram[i + 11]  = io->control_out[i];
	}

	for (int i = 16; i < 520; i++) {
		initDatagram[i]  = 0x00;
	}

	initDatagram[520] = ProtocolBoundaryUtils::kSyncByte;
    initDatagram[521] = ProtocolBoundaryUtils::kSyncByte;
    initDatagram[522] = ProtocolBoundaryUtils::kSyncByte;

	initDatagram[523] = io->control_out[0] | ((rx + 2) << 1);
    Settings* set = Settings::instance();
	initDatagram[524] = set->getCtrFrequencies().at(rx) >> 24;
	initDatagram[525] = set->getCtrFrequencies().at(rx) >> 16;
	initDatagram[526] = set->getCtrFrequencies().at(rx) >> 8;
	initDatagram[527] = set->getCtrFrequencies().at(rx) ;

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
