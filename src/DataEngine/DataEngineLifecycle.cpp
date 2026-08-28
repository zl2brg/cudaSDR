/**
* @file  DataEngineLifecycle.cpp
* @brief Protocol/engine bring-up and teardown helpers for DataEngine
*/

#include "DataEngineLifecycle.h"
#include "cusdr_dataEngine.h"
#include "CProtocol1.h"
#include "CProtocol2.h"
#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include <QCoreApplication>
#include <QDeadlineTimer>
#include <QEventLoop>

// Helpers are not QObjects; force QObject::connect for CHECKED_CONNECT.
#ifdef CHECKED_CONNECT
#undef CHECKED_CONNECT
#endif
#define CHECKED_CONNECT(source, signal, receiver, slot) \
	if (!QObject::connect(source, signal, receiver, slot)) \
		qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);

DataEngineLifecycle::DataEngineLifecycle(DataEngine *engine)
	: m_engine(engine)
{
}

namespace {
bool hasUsableHpsdrDevice(const TNetworkDevicecard &card)
{
	if (card.ip_address.isNull() || card.protocol <= 0)
		return false;
	const QString mac = QString::fromLatin1(card.mac_address);
	return !mac.isEmpty() && mac != QLatin1String("00:00:00:00:00:00");
}
} // namespace

bool DataEngineLifecycle::startDataEngineWithoutConnection() {

	DATA_ENGINE_DEBUG << "no HPSDR-HW interface";

	if (m_engine->m_dataIO->inputBuffer.length() > 0) {

        m_engine->initReceivers(1);
		if (!m_engine->m_dataIO)	m_engine->createDataIO();
		if (!m_engine->m_dataIOThread) m_engine->createDataIO();
		if (!m_engine->m_dataProcessor)	m_engine->createDataProcessor();



		// data receiver thread
		if (!m_engine->startDataIO(QThread::HighPriority)) {

			m_engine->setSystemState(QSDR::DataReceiverThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
			return false;
		}

				// IQ data processing thread
		if (!m_engine->startDataProcessor(QThread::HighPriority)) {

			m_engine->setSystemState(QSDR::DataProcessThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
			return false;
		}
		m_engine->setSystemState(QSDR::NoError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineUp);
        m_engine->set->setRadioState(RadioState::RX);
		return true;
	}
	else {

		DATA_ENGINE_DEBUG << "no data available - data file loaded?";
		return false;
	}
}


bool DataEngineLifecycle::findHPSDRDevices() {

	if (!m_engine->m_discoverer) m_engine->createDiscoverer();

	// HPSDR network IO thread
	if (!m_engine->startDiscoverer(QThread::NormalPriority)) {

		m_engine->m_dataIO->networkIOMutex.lock();
		DATA_ENGINE_DEBUG << "HPSDR device discovery thread could not be started.";
		m_engine->m_dataIO->networkIOMutex.unlock();
		return false;
	}

    // Invoke discovery on the discoverer thread (moved to thread via moveToThread,
    // so a queued invocation is required — a direct call would run on the wrong thread).
    QMetaObject::invokeMethod(m_engine->m_discoverer, "initHPSDRDevice", Qt::QueuedConnection);

	// Never QWaitCondition::wait() on the GUI thread: that stops the Qt event
	// loop. Probe already populated the device list asynchronously; if Start
	// must discover, keep the loop spinning until the worker finishes.
	DATA_ENGINE_DEBUG << "HPSDR network device detection...please wait.";
	m_engine->set->setSystemMessage("HPSDR network device detection...please wait", 0);

	QDeadlineTimer deadline(3000);
	while (!deadline.hasExpired()) {
		m_engine->m_hpsdrDevices = m_engine->set->getHpsdrNetworkDevices();
		if (m_engine->m_hpsdrDevices > 0 && !m_engine->set->getMetisCardsList().isEmpty())
			break;
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 50);
	}
	m_engine->m_hpsdrDevices = m_engine->set->getHpsdrNetworkDevices();

	if (m_engine->m_hpsdrDevices == 0) {
		m_engine->stopDiscoverer();
		DATA_ENGINE_DEBUG << "no device found. HPSDR hardware powered? Network connection established?";
		m_engine->set->setSystemMessage("no device found. HPSDR hardware powered? Network connection established?", 10000);

		m_engine->setSystemState(QSDR::HwIOError,	m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
		return false;
	}

	emit m_engine->clearSystemMessageEvent();

	QList<TNetworkDevicecard> metisList = m_engine->set->getMetisCardsList();
	DATA_ENGINE_DEBUG << "found " << metisList.count() << " network device(s)";

	for (int i = 0; i < metisList.count(); i++) {

		DATA_ENGINE_DEBUG 	<< "Device "
							<< i << " @ "
							<< qPrintable(metisList.at(i).ip_address.toString())
							<< " [" << metisList.at(i).mac_address << "]"
							<< " protocol=" << metisList.at(i).protocol;
	}

	if (!metisList.isEmpty()) {
		const TNetworkDevicecard current = m_engine->set->getCurrentMetisCard();
		const QString currentMac = QString::fromLatin1(current.mac_address);
		TNetworkDevicecard selected = metisList.first();
		for (const TNetworkDevicecard &card : metisList) {
			if (!currentMac.isEmpty()
				&& QString::fromLatin1(card.mac_address) == currentMac
				&& card.protocol == current.protocol) {
				selected = card;
				break;
			}
		}
		m_engine->set->setCurrentHPSDRDevice(selected);
		DATA_ENGINE_DEBUG << "using device "
						  << qPrintable(selected.ip_address.toString())
						  << " [" << selected.mac_address << "]"
						  << " protocol=" << selected.protocol
						  << " board=" << selected.boardName
						  << " adcs=" << selected.adcs;
	}

	m_engine->m_dataIO->hpsdrDeviceIPAddress = m_engine->set->getCurrentMetisCard().ip_address;
	m_engine->hpsdrDeviceName = m_engine->set->getCurrentMetisCard().boardName;
	DATA_ENGINE_DEBUG << "using HPSDR network device at " << qPrintable(m_engine->m_dataIO->hpsdrDeviceIPAddress.toString());

	m_engine->stopDiscoverer();

	if (m_engine->getFirmwareVersions()) return true;
	return false;
}


bool DataEngineLifecycle::start() {

	m_engine->m_fwCount = 0;
	m_engine->m_sendState = 0;

	// Only (re)create the protocol object if it does not exist yet.
	// When getFirmwareVersions() precedes start(), it already created the correct
	// protocol and set it on DataIO. Recreating here races with the DataIO thread.
	if (!m_engine->m_protocol && (m_engine->m_hwInterface == QSDR::Metis || m_engine->m_hwInterface == QSDR::Hermes)) {
		if (m_engine->set->getCurrentMetisCard().protocol == 2)
			m_engine->m_protocol = std::make_unique<CProtocol2>();
		else
			m_engine->m_protocol = std::make_unique<CProtocol1>();
	}
	if (m_engine->m_dataIO)
		m_engine->m_dataIO->setProtocol(m_engine->m_protocol.get());

	int rcvrs = m_engine->set->getNumberOfReceivers();
    if (!m_engine->m_audioInput) {
        m_engine->createAudioInputProcessor();
    }
    m_engine->m_audioInput->Setup();

	if (!m_engine->m_dataIO) m_engine->createDataIO();
	if (!m_engine->m_dataIOThread) m_engine->createDataIO();

	if (!m_engine->m_dataProcessor) m_engine->createDataProcessor();


	if (m_engine->m_serverMode == QSDR::SDRMode && !m_engine->m_wbDataProcessor)
		m_engine->createWideBandDataProcessor();


	switch (m_engine->m_serverMode) {

		//case QSDR::ExternalDSP:

			/*
			//CHECKED_CONNECT(
			//	set,
			//	SIGNAL(frequencyChanged(bool, int, long)),
			//	this,
			//	SLOT(setFrequency(bool, int, long)));

        //if (!m_audioProcessorRunning) {

        //	//if (!m_audioProcessor)	createAudioProcessor();
        //	if (!m_audioReceiver)	createAudioReceiver();

        //	m_audioInProcThread->start();
        //	if (m_audioInProcThread->isRunning()) {
        //
        //		m_audioInProcThreadRunning = true;
        //		DATA_ENGINE_DEBUG << "Audio processor process started.";
        //	}
        //	else {

        //		m_audioInProcThreadRunning = false;
        //		setSystemState(
        //						QSDR::AudioThreadError,
        //						m_hwInterface,
        //						m_serverMode,
        //						QSDR::DataEngineDown);
        //		return false;
        //	}
        //
        //	audio_rx = 0;
        //	clientList.append(0);

			//	m_audioProcessorRunning = true;
			//	setSystemState(
			//			QSDR::NoError,
			//			m_hwInterface,
			//			m_serverMode,
			//			QSDR::DataEngineUp);
			//}
			 */
			//return false;

        case QSDR::SDRMode:

            m_engine->setTimeStamp(false);
            break;

		default:

			DATA_ENGINE_DEBUG << "no valid server mode";

			m_engine->setSystemState(QSDR::ServerModeError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
			return false;
	}	// end switch (m_serverMode)

	if (m_engine->RX.count() != rcvrs || m_engine->m_dspThreadList.count() != rcvrs) {
		if (!m_engine->m_dspThreadList.isEmpty()) {
			foreach (QThread* thread, m_engine->m_dspThreadList) {
				thread->quit();
				thread->wait();
			}
			qDeleteAll(m_engine->m_dspThreadList.begin(), m_engine->m_dspThreadList.end());
			m_engine->m_dspThreadList.clear();
		}

		if (!m_engine->RX.isEmpty()) {
			qDeleteAll(m_engine->RX.begin(), m_engine->RX.end());
			m_engine->RX.clear();
		}

		if (!m_engine->initReceivers(rcvrs)) {
			DATA_ENGINE_DEBUG << "failed to initialize receivers for count" << rcvrs;
			return false;
		}
	}

	m_engine->connectDSPSlots();
	const QList<qint64> ctrFrequencies = m_engine->set->getCtrFrequencies();
	for (int i = 0; i < rcvrs ; i++) {

		m_engine->RX.at(i)->setConnectedStatus(true);
		if (i < ctrFrequencies.count()) {
			m_engine->setFrequency(true, i, ctrFrequencies.at(i));
		}


        //CHECKED_CONNECT(
		//		RX.at(i),
		//		SIGNAL(outputBufferSignal(int, const CPX &)),
		//		this, //m_dataProcessor,
		//		SLOT(setOutputBuffer(int, const CPX &)));

		CHECKED_CONNECT(
				m_engine->RX.at(i),
				&SliceProcessor::outputBufferSignal,
				m_engine->m_dataProcessor,
				&DataProcessor::setOutputBuffer);

        CHECKED_CONNECT(
				m_engine->RX.at(i),
				&SliceProcessor::audioBufferSignal,
				m_engine->m_dataProcessor,
				&DataProcessor::send_hpsdr_data);

		m_engine->m_dspThreadList.at(i)->start(QThread::HighPriority);

		if (m_engine->m_dspThreadList.at(i)->isRunning()) {

			//m_dataProcThreadRunning = true;
			m_engine->m_dataIO->networkIOMutex.lock();
			DATA_ENGINE_DEBUG << "receiver processor thread started for Rx " << i;
			m_engine->m_dataIO->networkIOMutex.unlock();
		}
		else {

			//m_dataProcThreadRunning = false;
			//setSystemState(QSDR::DataProcessThreadError, m_hwInterface, m_serverMode, QSDR::DataEngineDown);
			return false;
	}
		m_engine->m_dataIO->set_wbBuffers(m_engine->set->getWidebandBuffers());
	}

/*
    if (!startAudioInputProcessor(QThread::NormalPriority))
    {
        DATA_ENGINE_DEBUG << "Audio Input data processor thread could not be started.";
        return false;
    }
*/

	if (!m_engine->startWideBandDataProcessor(QThread::NormalPriority)) {

		DATA_ENGINE_DEBUG << "wide band data processor thread could not be started.";
		return false;
	}

	// data IO thread
//	if (!startDataIO(QThread::HighPriority)) {//  ::NormalPriority)) {
//
//		DATA_ENGINE_DEBUG << "data receiver thread could not be started.";
//		return false;
//	}

	// IQ data processing thread
	if (!m_engine->startDataProcessor(QThread::HighPriority)) {

		DATA_ENGINE_DEBUG << "data processor thread could not be started.";
		return false;
	}

	// Never write the DataIO QUdpSocket from the UI thread. P1 used to
	// sendInitFrames/networkDeviceStartStop here after msleep(100); the
	// same-host Hermes replies on that socket immediately, so UI
	// writeDatagram raced DataIO readyRead and froze the GUI.
	// Arm P1 start BEFORE startDataIO so initDataReceiverSocket() (runs on
	// QThread::started, before exec) sends from the socket it just bound.
	DataIO* dataIO = m_engine->m_dataIO;
	const int rxCount = m_engine->receivers();
	const bool isProtocol2 = m_engine->set->getCurrentMetisCard().protocol == 2;
	if (dataIO && !isProtocol2) {
		const char startByte = (m_engine->m_serverMode == QSDR::SDRMode && m_engine->set->getWidebandData())
			? 0x03
			: 0x01;
		DATA_ENGINE_DEBUG << "[START] arming Protocol 1 networkDeviceStartStop(" << int(startByte) << ")";
		dataIO->armProtocol1Start(rxCount, startByte);
	}

	// data IO thread
	if (!m_engine->startDataIO(QThread::HighPriority)) {//  ::NormalPriority::HighPriority)) {

		DATA_ENGINE_DEBUG << "data IO thread could not be started.";
		return false;
	}

	if (dataIO && isProtocol2) {
		QMetaObject::invokeMethod(dataIO, [dataIO, rxCount]() {
			for (int i = 0; i < rxCount; ++i)
				dataIO->sendInitFramesToNetworkDevice(i);
		}, Qt::QueuedConnection);
	}

	// Protocol 2: do NOT use formatStartStop() as the first live HP packet.
	// That datagram zeros DDC frequencies, and hpsdrsim gates IQ until
	// enable + rate + freq are all non-zero (wideband only needs Run).
	// Push the full General→DDC→TX→HP setup, then start the control timer.
	if (m_engine->set->getCurrentMetisCard().protocol == 2 && m_engine->m_dataProcessor) {
		m_engine->rcveIQ_toggle = false;
		QMetaObject::invokeMethod(
			m_engine->m_dataProcessor,
			&DataProcessor::requestProtocol2ReceiverSetup,
			Qt::QueuedConnection);
		QMetaObject::invokeMethod(
			m_engine->m_dataProcessor,
			&DataProcessor::startControlTimer,
			Qt::QueuedConnection);
		DATA_ENGINE_DEBUG << "[P2-START] queued full receiver setup + control timer";
	}

	m_engine->m_networkDeviceRunning = true;
	m_engine->setSystemState(QSDR::NoError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineUp);
	m_engine->set->setSystemMessage("System running", 4000);

		DATA_ENGINE_DEBUG << "Data Engine thread: " << m_engine->thread();

	return true;
}


void DataEngineLifecycle::stop() {

	if (m_engine->m_dataEngineState == QSDR::DataEngineUp) {
		
		switch (m_engine->m_hwInterface) {

			case QSDR::Metis:
			case QSDR::Hermes:
				
				// turn time stamping off
				m_engine->setTimeStamp(false);

				// For Protocol 2, stop periodic control traffic before issuing the
				// final stop command so the simulator does not see interleaved HP
				// packets with sequence jumps during shutdown.
				if (m_engine->set->getCurrentMetisCard().protocol == 2 &&
					m_engine->m_dataProcessor && m_engine->m_dataProcThread && m_engine->m_dataProcThread->isRunning()) {
					QMetaObject::invokeMethod(
						m_engine->m_dataProcessor,
						&DataProcessor::stopControlTimer,
						Qt::BlockingQueuedConnection);
				}

				// stop the device on the DataIO thread (same socket affinity as start)
				if (m_engine->m_dataIO && m_engine->m_dataIOThread
					&& m_engine->m_dataIOThread->isRunning()) {
					DataIO* dataIO = m_engine->m_dataIO;
					QMetaObject::invokeMethod(dataIO, [dataIO]() {
						dataIO->networkDeviceStartStop(0);
					}, Qt::BlockingQueuedConnection);
				} else if (m_engine->m_dataIO) {
					m_engine->m_dataIO->networkDeviceStartStop(0);
				}
				m_engine->m_networkDeviceRunning = false;
				DATA_ENGINE_DEBUG << "HPSDR device stopped";

				// stop the threads
				//QThread::msleep(100);
				m_engine->stopDataIO();
				QThread::msleep(100);
				m_engine->stopDataProcessor();
				if (m_engine->m_wbDataProcessor)
					m_engine->stopWideBandDataProcessor();

                m_engine->m_protocol.reset();
                if (m_engine->m_dataIO) m_engine->m_dataIO->setProtocol(nullptr);
				
				// clear device list
				QThread::msleep(100);
				m_engine->set->clearMetisCardList();
				DATA_ENGINE_DEBUG << "device cards list cleared.";
				break;

			case QSDR::NoInterfaceMode:

				m_engine->stopDataIO();
				
				DATA_ENGINE_DEBUG << "data queue count: " << m_engine->m_dataIO->soapy_iq_queue.count();
				m_engine->stopDataProcessor();
                break;

#ifdef HAVE_SOAPYSDR
            case QSDR::SoapySDR:

                m_engine->stopDataIO();
                m_engine->stopDataProcessor();
                break;
#endif
        }

		while (!m_engine->m_dataIO->au_queue.isEmpty())
			m_engine->m_dataIO->au_queue.dequeue();

		// Mark slices stopped so DSP workers drop out of writeAudio ASAP.
		for (const auto &rx : m_engine->RX) {
			if (rx)
				rx->stop();
		}

		// Stop all WDSP channels BEFORE killing DSP threads.
		// SetChannelState(wait=1) deadlocks if called after the DSP thread is
		// dead (nobody calls fexchange0 to release the internal semaphore).
		// Clearing run=0 here while threads are still alive lets any in-flight
		// fexchange0 call observe the flag and exit, leaving the channel in a
		// clean state for CloseChannel in the destructor.
		for (const auto &rx : m_engine->RX) {
			if (rx->qtwdsp) {
				DATA_ENGINE_DEBUG << "[RX-STOP] stopping WDSP channel for rx" << rx->getReceiverNo();
				rx->qtwdsp->stopChannel();
			}
		}
		QThread::msleep(5); // let any in-flight fexchange0 observe run=0

		// clear receiver thread list
		foreach (QThread* thread, m_engine->m_dspThreadList) {

			thread->quit();
			thread->wait();
		}
		qDeleteAll(m_engine->m_dspThreadList.begin(), m_engine->m_dspThreadList.end());
		m_engine->m_dspThreadList.clear();

		// Now safe: no DSP thread is writing into QAudioSink anymore.
		for (const auto &rx : m_engine->RX) {
			if (rx)
				rx->stopAudio();
		}

		// clear receiver list
        for (const auto &rx : m_engine->RX) {

			rx->setConnectedStatus(false);
			m_engine->disconnectDSPSlots();

			if (m_engine->m_radioModel && m_engine->m_radioModel->telemetry()) {
				QObject::disconnect(rx, nullptr, m_engine->m_radioModel->telemetry(), nullptr);
			}

			/*disconnect(
				rx,
				SIGNAL(outputBufferSignal(int, const CPX &)),
				this,
				SLOT(setOutputBuffer(int, const CPX &)));*/

			/*disconnect(
				rx,
				SIGNAL(outputBufferSignal(int, const CPX &)),
				m_dataProcessor,
				SLOT(setOutputBuffer(int, const CPX &)));*/

			//rx->deleteDSPInterface();
			//DATA_ENGINE_DEBUG << "DSP core deleted.";
		}
		qDeleteAll(m_engine->RX.begin(), m_engine->RX.end());
		m_engine->RX.clear();
		m_engine->set->setRxList(m_engine->RX);
		DATA_ENGINE_DEBUG << "receiver threads deleted, receivers deleted, receiver & thread list cleared.";
		m_engine->set->setSystemMessage("Data engine shut down.", 4000);

		m_engine->setSystemState(QSDR::NoError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
	}

	m_engine->m_rxSamples = 0;
	m_engine->m_restart = true;
	m_engine->m_found = 0;
	m_engine->m_hpsdrDevices = 0;

	m_engine->set->setMercuryVersion(0);
	m_engine->set->setPenelopeVersion(0);
	m_engine->set->setMetisVersion(0);
	m_engine->set->setHermesVersion(0);

	//set->setPeakHold(false);
	//set->resetWidebandSpectrumBuffer();

	/*disconnect(
		set, 
		SIGNAL(ctrFrequencyChanged(int, int, long)), 
		this, 
		SLOT(setFrequency(int, int, long)));*/

	DATA_ENGINE_DEBUG << "shut down done.";
}


bool DataEngineLifecycle::initDataEngine() {

#ifdef TESTING
	qDebug() << "************************** TESTING MODUS ***********************************";
	return m_engine->start();
#endif

	if (m_engine->m_hwInterface == QSDR::NoInterfaceMode) {
		
		return m_engine->startDataEngineWithoutConnection();
	}
#ifdef HAVE_SOAPYSDR
    else if (m_engine->m_hwInterface == QSDR::SoapySDR) {
        return m_engine->startSoapyEngine();
    }
#endif
	else {
		const TNetworkDevicecard already = m_engine->set->getCurrentMetisCard();
		if (hasUsableHpsdrDevice(already) && m_engine->m_dataIO) {
			// Probe / Network panel already selected the radio. Do not block
			// the GUI event loop in QWaitCondition::wait() on Start.
			m_engine->set->setCurrentHPSDRDevice(already);
			m_engine->m_dataIO->hpsdrDeviceIPAddress = already.ip_address;
			m_engine->hpsdrDeviceName = already.boardName;
			DATA_ENGINE_DEBUG << "Start using selected device "
							  << qPrintable(already.ip_address.toString())
							  << " [" << already.mac_address << "]"
							  << " protocol=" << already.protocol
							  << " board=" << already.boardName;
			if (m_engine->getFirmwareVersions())
				return m_engine->start();
			return false;
		}

		if (m_engine->findHPSDRDevices()) {
			DATA_ENGINE_DEBUG << "got firmware versions:";
			DATA_ENGINE_DEBUG << "	Metis firmware:  " << m_engine->metisFW;
			DATA_ENGINE_DEBUG << "	Mercury firmware:  " << m_engine->mercuryFW;
			DATA_ENGINE_DEBUG << "	Penelope firmware:  " << m_engine->penelopeFW;
			DATA_ENGINE_DEBUG << "	Pennylane firmware:  " << m_engine->pennylaneFW;
			DATA_ENGINE_DEBUG << "	Hermes firmware: " << m_engine->hermesFW;

			return m_engine->start();
		}
	}
	return false;
}


void DataEngineLifecycle::searchHpsdrNetworkDevices() {

	if (!m_engine->m_discoverer) m_engine->createDiscoverer();

	// HPSDR network IO thread
	if (!m_engine->startDiscoverer(QThread::NormalPriority)) {

		DATA_ENGINE_DEBUG << "HPSDR network discovery thread could not be started.";
		return;
	}

    QMetaObject::invokeMethod(m_engine->m_discoverer, "initHPSDRDevice", Qt::QueuedConnection);

	QDeadlineTimer deadline(3000);
	while (!deadline.hasExpired()) {
		if (m_engine->set->getHpsdrNetworkDevices() > 0)
			break;
		QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 50);
	}

	//m_discoverer->findHPSDRDevices();

	// stop the discovery thread
	m_engine->stopDiscoverer();
}

