/**
* @file  DataEngineFirmware.cpp
* @brief Firmware version probe/check helpers for DataEngine
*/

#include "DataEngineFirmware.h"
#include "cusdr_dataEngine.h"
#include "CProtocol1.h"
#include "CProtocol2.h"

DataEngineFirmware::DataEngineFirmware(DataEngine *engine)
	: m_engine(engine)
{
}

bool DataEngineFirmware::getFirmwareVersions() {

	m_engine->m_fwCount = 0;

	// Create the protocol object now, before DataIO starts, so that
	// initDataReceiverSocket() binds the correct ports and readDeviceData()
	// doesn't drop packets with a null protocol check.
	if (m_engine->m_hwInterface == QSDR::Metis || m_engine->m_hwInterface == QSDR::Hermes) {
		if (m_engine->set->getCurrentMetisCard().protocol == 2)
			m_engine->m_protocol = std::make_unique<CProtocol2>();
		else
			m_engine->m_protocol = std::make_unique<CProtocol1>();
	}
	if (m_engine->m_dataIO) m_engine->m_dataIO->setProtocol(m_engine->m_protocol.get());

	// Discovery already supplied board firmware for both protocols. Do not start
	// DataIO here: that runs on the UI thread during Start, and P1 then writes
	// the DataIO QUdpSocket from the GUI while the IO thread is in readyRead
	// (same-host Hermes replies immediately) — Qt deadlocks and the UI freezes.
	// P2 already deferred IO to start(); P1 must do the same.
	m_engine->metisFW = m_engine->set->getMetisVersion();
	m_engine->mercuryFW = m_engine->set->getMercuryVersion();
	m_engine->penelopeFW = m_engine->set->getPenelopeVersion();
	m_engine->pennylaneFW = m_engine->set->getPennyLaneVersion();
	m_engine->hermesFW = m_engine->set->getHermesVersion();
	m_engine->txParams().drivelevel = m_engine->set->get_tx_drivelevel();

	const TNetworkDevicecard card = m_engine->set->getCurrentMetisCard();
	if (m_engine->metisFW == 0 && m_engine->hermesFW == 0 && card.sw_version > 0) {
		if (card.protocol == 1 && card.boardID == 0)
			m_engine->metisFW = card.sw_version;
		else
			m_engine->hermesFW = card.sw_version;
	}

	DATA_ENGINE_DEBUG << "getFirmwareVersions: using discovery firmware (hermesFW=" << m_engine->hermesFW
	                  << " metisFW=" << m_engine->metisFW << " mercuryFW=" << m_engine->mercuryFW
	                  << " board=" << card.boardName << " protocol=" << card.protocol
	                  << "), deferring IO start to start()";

	if (m_engine->set->getFirmwareVersionCheck())
		return m_engine->checkFirmwareVersions();
	return true;
}

// credits go to George Byrkit, K9TRV: the older FW checkings are shamelessly taken from the KISS Konsole!
// TODO(P2-FIRMWARE): All firmware checks below compare hpsdrDeviceName against
// the strings "Metis" and "Hermes".  Protocol 2 hardware reports board type as a
// numeric board ID in the discovery reply (byte 11), not a name string.  The name
// assigned to hpsdrDeviceName at line 544 comes from MetisCard::boardName which
// is populated during discovery and may not equal "Metis" or "Hermes" for newer P2
// boards ("Orion", "Orion2", "Angelia", etc.).  Until the board-name mapping for
// P2 boards is verified, these checks are likely silently skipped for P2 hardware.
bool DataEngineFirmware::checkFirmwareVersions() {
	const bool isProtocol2 = (m_engine->set->getCurrentMetisCard().protocol == 2);
	const bool isMetisModular = (m_engine->hpsdrDeviceName == "Metis" && !isProtocol2);

	// Protocol 2 devices and integrated SDR boards (Hermes, Angelia, Orion, Orion2, Saturn, HL2, STEMlab)
	// do not require modular Atlas bus cards (Penelope / Mercury / PennyLane).
	if (!isMetisModular) {
		if (m_engine->m_hwInterface == QSDR::Metis && m_engine->hpsdrDeviceName != "Metis") {
			DATA_ENGINE_DEBUG << "HW interface is Metis but board is" << m_engine->hpsdrDeviceName << "continuing";
		}
		if (m_engine->hermesFW < 18 && m_engine->set->getNumberOfReceivers() > 2 && m_engine->hpsdrDeviceName == "Hermes" && !isProtocol2) {
			m_engine->stop();
			QString msg = "Hermes FW < V1.8 has only 2 receivers!";
			m_engine->set->showWarningDialog(msg);
			return false;
		}
		m_engine->setWideBandBufferCount();
		return true;
	}

		QString msg;
		switch (m_engine->metisFW) {

			case 13:
				if (((m_engine->set->getPenelopePresence() || m_engine->set->getPennyLanePresence()) &&
					(m_engine->penelopeFW == 13 || m_engine->pennylaneFW == 13)) ||
					m_engine->mercuryFW != 29)
				{
					m_engine->stop();

					msg = "Penny[Lane] FW Version V1.3 and Mercury FW V2.7 requires Metis FW V1.3!";
					m_engine->set->showWarningDialog(msg);
					return false;
				}
				break;

			case 14:
				if (((m_engine->set->getPenelopePresence() || m_engine->set->getPennyLanePresence()) &&
					(m_engine->penelopeFW == 14 || m_engine->pennylaneFW == 14)) ||
					m_engine->mercuryFW != 29)
				{
					m_engine->stop();

					msg = "Penny[Lane] FW Version V1.4 and Mercury FW V2.7 requires Metis FW V1.4!";
					m_engine->set->showWarningDialog(msg);
					return false;
				}
				break;

			case 15:

				if (((m_engine->set->getPenelopePresence() || m_engine->set->getPennyLanePresence()) &&
					(m_engine->penelopeFW == 15 || m_engine->pennylaneFW == 15)) ||
					m_engine->mercuryFW != 30)
				{
					m_engine->stop();

					msg = "Penny[Lane] FW Version V1.5 and Mercury FW V3.0 requires Metis FW V1.5!";
					m_engine->set->showWarningDialog(msg);
					return false;
				}
				break;

			case 16:

				if (((m_engine->set->getPenelopePresence() || m_engine->set->getPennyLanePresence()) &&
					(m_engine->penelopeFW == 16 || m_engine->pennylaneFW == 16)) ||
					m_engine->mercuryFW != 31)
				{
					m_engine->stop();

					msg = "Penny[Lane] FW Version V1.6 and Mercury FW V3.1 requires Metis FW V1.6!";
					m_engine->set->showWarningDialog(msg);
					return false;
				}
				break;

			case 17:
			case 18:

				if (((m_engine->set->getPenelopePresence() || m_engine->set->getPennyLanePresence()) &&
					(m_engine->penelopeFW == 17 || m_engine->pennylaneFW == 17)) ||
					m_engine->mercuryFW != 32)
				{
					m_engine->stop();

					msg = "Penny[Lane] FW Version V1.7 and Mercury FW V3.2 requires Metis FW V1.7 or V1.8!";
					m_engine->set->showWarningDialog(msg);
					return false;
				}
				break;

			case 19:
			case 20:

				m_engine->stop();

				msg = "Metis FW V1.9 or V2.0 have some problems - please upgrade to Metis V2.1!";
				m_engine->set->showWarningDialog(msg);
				return false;
				break;

			case 21:

				if ((m_engine->set->getPenelopePresence() && m_engine->penelopeFW != 17)	||
					(m_engine->set->getPennyLanePresence() && m_engine->pennylaneFW != 17)||
					m_engine->mercuryFW != 33)
				{
					m_engine->stop();

					msg = "Penny[Lane] FW Version V1.7 and Mercury FW V3.3 required for Metis FW V2.1!";
					m_engine->set->showWarningDialog(msg);
					return false;
				}
				break;

//			case 22:
//
//				if ((set->getPenelopePresence() && m_penelopeFW != 17)	||
//					(set->getPennyLanePresence() && m_pennylaneFW != 17)||
//					m_mercuryFW != 33)
//				{
//					stop();
//
//					msg = "Penny[Lane] FW Version V1.7 and Mercury FW V3.3 required for Metis FW >= V2.1!";
//					set->showWarningDialog(msg);
//					return false;
//				}
//				break;

            case 26:

                if ((m_engine->set->getPenelopePresence() && m_engine->penelopeFW != 18)	||
                    (m_engine->set->getPennyLanePresence() && m_engine->pennylaneFW != 18)||
                    m_engine->mercuryFW != 34)
                {
                    m_engine->stop();

                    msg = "Penny[Lane] FW Version V1.8 and Mercury FW V3.4 required for Metis FW V2.6!";
                    m_engine->set->showWarningDialog(msg);
                    return false;
                }
                break;

            default:

				//stop();

				msg = "Not a standard Metis FW version !";
				m_engine->set->showWarningDialog(msg);
				//return false;
				return true;
		}

	if (m_engine->mercuryFW < 33 && m_engine->set->getNumberOfReceivers() > 4 && m_engine->hpsdrDeviceName == "Metis") {

		m_engine->stop();

		QString msg = "Mercury FW < V3.3 has only 4 receivers!";
		m_engine->set->showWarningDialog(msg);
		return false;
	}

	if (m_engine->hermesFW < 18 && m_engine->set->getNumberOfReceivers() > 2 && m_engine->hpsdrDeviceName == "Hermes") {

		m_engine->stop();

		QString msg = "Hermes FW < V1.8 has only 2 receivers!";
		m_engine->set->showWarningDialog(msg);
		return false;
	}
	m_engine->setWideBandBufferCount();

	return true;
}

