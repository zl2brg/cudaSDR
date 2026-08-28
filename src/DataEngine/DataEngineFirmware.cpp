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

	// init receivers
	int rcvrs = m_engine->set->getNumberOfReceivers();
	QString str = "Initializing %1 receiver(s)...please wait";
	m_engine->set->setSystemMessage(str.arg(m_engine->set->getNumberOfReceivers()), rcvrs * 500);
	if (!m_engine->initReceivers(rcvrs)) return false;

	if (!m_engine->m_dataIO) m_engine->createDataIO();
	if (!m_engine->m_dataIOThread) m_engine->createDataIO();
		
	if (!m_engine->m_dataProcessor) m_engine->createDataProcessor();

	switch (m_engine->m_serverMode) {

		case QSDR::SDRMode:
			
			for (int i = 0; i < m_engine->set->getNumberOfReceivers(); i++) {

				m_engine->RX.at(i)->setConnectedStatus(true);
			}
			break;
		default:

			DATA_ENGINE_DEBUG << "no valid server mode";
			m_engine->setSystemState(QSDR::ServerModeError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);

			return false;
	}

	m_engine->connectDSPSlots();

//	for (int i = 0; i < set->getNumberOfReceivers(); i++)
//		RX.at(i)->setAudioVolume(i, set->getMainVolume());

	const bool isProtocol2 = m_engine->set->getCurrentMetisCard().protocol == 2;

	// Protocol 2: discovery already provided board FW. Do NOT start DataIO /
	// DataProcessor here — start() must bind the ephemeral socket and run
	// P2-RXSETUP on a clean bring-up. Starting IO early then "restarting"
	// without stop() left sockets alive but DDC IQ never latched.
	if (isProtocol2) {
		m_engine->metisFW = m_engine->set->getMetisVersion();
		m_engine->mercuryFW = m_engine->set->getMercuryVersion();
		m_engine->penelopeFW = m_engine->set->getPenelopeVersion();
		m_engine->pennylaneFW = m_engine->set->getPennyLaneVersion();
		m_engine->hermesFW = m_engine->set->getHermesVersion();
		m_engine->txParams().drivelevel = m_engine->set->get_tx_drivelevel();
		DATA_ENGINE_DEBUG << "Protocol 2: using discovery firmware, deferring IO start to start()";
		if (m_engine->set->getFirmwareVersionCheck())
			return m_engine->checkFirmwareVersions();
		return true;
	}

	// IQ data processing thread
	if (!m_engine->startDataProcessor(QThread::HighPriority)) {

		DATA_ENGINE_DEBUG << "data processor thread could not be started.";
		return false;
	}

	// data IO thread
	if (!m_engine->startDataIO(QThread::HighPriority)) {//  ::NormalPriority)) {

		DATA_ENGINE_DEBUG << "data IO thread could not be started.";
		return false;
	}

	//setSampleRate(set->getSampleRate());
	QThread::msleep(100);

	// Protocol 1: start device to collect FW version response packets.
	{
		// Discovery may have cached Metis/Hermes board FW. Those values are not
		// Mercury/Hermes C&C versions from live IQ. Clear them so a stale Metis
		// discovery version cannot drive checkFirmwareVersions() into a stop()
		// ~300ms later with a misleading "Mercury FW required" dialog and
		// "Error: No error" (stop() clears the system error).
		m_engine->set->setMercuryVersion(0);
		m_engine->set->setPenelopeVersion(0);
		m_engine->set->setPennyLaneVersion(0);
		m_engine->set->setHermesVersion(0);
		m_engine->set->setMetisVersion(0);

		// pre-conditioning
		if (m_engine->m_dataIO) {
			for (int i = 0; i < m_engine->receivers(); i++)
				m_engine->m_dataIO->sendInitFramesToNetworkDevice(i);
		}

		if (m_engine->m_serverMode == QSDR::SDRMode && m_engine->m_dataIO)
			m_engine->m_dataIO->networkDeviceStartStop(0x01);

		m_engine->m_networkDeviceRunning = true;
		m_engine->setSystemState(QSDR::NoError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineUp);

		// Poll for IQ C&C firmware bytes instead of a single blind sleep.
		// Historical wait was 300ms; allow a bit longer for slow NICs/Wi‑Fi.
		const int probeTimeoutMs = 1500;
		const int pollMs = 50;
		int waitedMs = 0;
		while (waitedMs < probeTimeoutMs) {
			QThread::msleep(pollMs);
			waitedMs += pollMs;
			// Hermes/ANAN boards often report board FW as metisFW when the Metis
			// C&C decode path is used; any non-zero IQ C&C version means IQ is live.
			if (m_engine->set->getMercuryVersion() > 0 || m_engine->set->getHermesVersion() > 0
				|| m_engine->set->getMetisVersion() > 0)
				break;
		}

		m_engine->metisFW = m_engine->set->getMetisVersion();
		m_engine->mercuryFW = m_engine->set->getMercuryVersion();
		m_engine->penelopeFW = m_engine->set->getPenelopeVersion();
		m_engine->pennylaneFW = m_engine->set->getPennyLaneVersion();
		m_engine->hermesFW = m_engine->set->getHermesVersion();

		if (m_engine->mercuryFW == 0 && m_engine->hermesFW == 0 && m_engine->metisFW == 0) {
			DATA_ENGINE_DEBUG << "no IQ firmware response after" << waitedMs
							  << "ms (metisFW=" << m_engine->metisFW
							  << " mercuryFW=" << m_engine->mercuryFW
							  << " hermesFW=" << m_engine->hermesFW << ")";
			m_engine->set->setSystemMessage(
				"No IQ/firmware response from device after start — check "
				"network, local interface, firewall, and that nothing else "
				"owns the radio",
				10000);
			// Device was started for the probe; tear it down and keep FirmwareError
			// (stop() alone would report "No error").
			m_engine->stop();
			m_engine->setSystemState(QSDR::FirmwareError, m_engine->m_hwInterface, m_engine->m_serverMode,
						   QSDR::DataEngineDown);
			return false;
		}

		DATA_ENGINE_DEBUG << "IQ firmware response after" << waitedMs << "ms";
	}

	m_engine->txParams().drivelevel = m_engine->set->get_tx_drivelevel();
	if (m_engine->set->getFirmwareVersionCheck())
		return m_engine->checkFirmwareVersions();
	else
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

