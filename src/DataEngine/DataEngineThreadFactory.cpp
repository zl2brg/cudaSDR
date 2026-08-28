/**
* @file  DataEngineThreadFactory.cpp
* @brief Worker thread create/start/stop helpers for DataEngine
*/

#include "DataEngineThreadFactory.h"
#include "cusdr_dataEngine.h"
#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include "Util/cusdr_tciserver.h"
#ifdef HAVE_SOAPYSDR
#include "SoapySDRDataSource.h"
#endif

// Helpers are not QObjects; force QObject::connect for CHECKED_CONNECT.
#ifdef CHECKED_CONNECT
#undef CHECKED_CONNECT
#endif
#define CHECKED_CONNECT(source, signal, receiver, slot) \
	if (!QObject::connect(source, signal, receiver, slot)) \
		qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);

DataEngineThreadFactory::DataEngineThreadFactory(DataEngine *engine)
	: m_engine(engine)
{
}

void DataEngineThreadFactory::createDiscoverer() {

	m_engine->m_discoverer = new Discoverer(m_engine->m_dataIO);

	m_engine->m_discoveryThread = new QThreadEx();
	m_engine->m_discoverer->moveToThread(m_engine->m_discoveryThread);

#ifdef HAVE_SOAPYSDR
    QObject::connect(m_engine->m_discoverer, &Discoverer::soapyDeviceListFound, m_engine->set, &Settings::setSoapyDeviceList);
#endif
}

bool DataEngineThreadFactory::startDiscoverer(QThread::Priority prio) {

	m_engine->m_discoveryThread->start(prio);

	if (m_engine->m_discoveryThread->isRunning()) {
					
		m_engine->m_discoveryThreadRunning = true;
		m_engine->m_dataIO->networkIOMutex.lock();
        DATA_ENGINE_DEBUG << "HPSDR device discovery thread started.";
		m_engine->m_dataIO->networkIOMutex.unlock();

		return true;
	}
	else {

		m_engine->m_discoveryThreadRunning = false;
		return false;
	}
}

void DataEngineThreadFactory::stopDiscoverer() {

	if (m_engine->m_discoveryThread->isRunning()) {
		
		m_engine->m_discoveryThread->quit();
		m_engine->m_discoveryThread->wait(1000);
		delete m_engine->m_discoveryThread;
        m_engine->m_discoveryThread = nullptr;
		delete m_engine->m_discoverer;
		m_engine->m_discoverer = nullptr;

		m_engine->m_discoveryThreadRunning = false;

		DATA_ENGINE_DEBUG << "HPSDR discovery thread stopped and deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "HPSDR discovery thread wasn't started.";
}


void DataEngineThreadFactory::createDataIO() {
	if (!m_engine->m_dataIO) {
		m_engine->m_dataIO = new DataIO();
	}
	m_engine->m_dataIO->setDataEngine(m_engine);
	m_engine->m_dataIO->setRadioModel(m_engine->m_radioModel);
	m_engine->m_dataIO->setProtocol(m_engine->m_protocol.get());
	m_engine->m_dataIO->setSampleRate(m_engine->samplerate);

	// Eager ctor creates DataIO for discovery mutex/queues; thread wiring still
	// happens here. Skip if the IO thread is already set up.
	if (m_engine->m_dataIOThread)
		return;

#ifdef HAVE_SOAPYSDR
    if (m_engine->m_hwInterface == QSDR::SoapySDR) {
        m_engine->m_soapySDRSource = new SoapySDRDataSource(m_engine);
        m_engine->m_dataIOThread = new QThreadEx();
        m_engine->m_soapySDRSource->moveToThread(m_engine->m_dataIOThread);

        m_engine->m_soapySDRSource->connect(
                    m_engine->m_dataIOThread,
                    &QThread::started,
                    m_engine->m_soapySDRSource,
                    &SoapySDRDataSource::init);

        m_engine->m_soapySDRSource->connect(
                    m_engine->m_dataIOThread,
                    &QThread::started,
                    m_engine->m_soapySDRSource,
                    &SoapySDRDataSource::runStream);

        if (RadioTelemetry* tel = m_engine->m_radioModel ? m_engine->m_radioModel->telemetry() : nullptr) {
            QObject::connect(m_engine->m_soapySDRSource, &SoapySDRDataSource::widebandSpectrumReady,
                    tel, &RadioTelemetry::setWidebandSpectrumBuffer);
            QObject::connect(m_engine->m_soapySDRSource, &SoapySDRDataSource::widebandSpectrumReset,
                    tel, &RadioTelemetry::resetWidebandSpectrumBuffer);
            QObject::connect(m_engine->m_soapySDRSource, &SoapySDRDataSource::widebandFrequencyRangeReady,
                    tel, &RadioTelemetry::setWidebandFrequencyRange);
        }
        return;
    }
#endif

	switch (m_engine->m_serverMode) {
		
		//case QSDR::ExternalDSP:
		//	break;

		//case QSDR::InternalDSP:
		case QSDR::SDRMode:

			m_engine->m_dataIO->networkIOMutex.lock();
			DATA_ENGINE_DEBUG 	<< "configured for "
								<< qPrintable(QString::number(m_engine->set->getNumberOfReceivers()))
								<< " receiver(s) at "
								<< qPrintable(QString::number(m_engine->set->getSampleRate()/1000))
								<< " kHz sample rate";
			m_engine->m_dataIO->networkIOMutex.unlock();
//			sendMessage(
//				m_message.arg(
//					QString::number(set->getNumberOfReceivers()),
//					QString::number(set->getSampleRate()/1000)));
			break;


        case QSDR::NoServerMode:
            break;
    }

	m_engine->m_dataIOThread = new QThreadEx();
	m_engine->m_dataIO->moveToThread(m_engine->m_dataIOThread);

	switch (m_engine->m_hwInterface) {

		case QSDR::NoInterfaceMode:
/*
			m_dataIO->connect(
						m_dataIOThread,
						SIGNAL(started()), 
						SLOT(readData()));
						*/
			break;
			
		case QSDR::Metis:
		case QSDR::Hermes:

			m_engine->m_dataIO->connect(
						m_engine->m_dataIOThread,
						&QThread::started, 
						m_engine->m_dataIO,
						&DataIO::initDataReceiverSocket);
			break;
#ifdef HAVE_SOAPYSDR
        case QSDR::SoapySDR:
            break;
#endif
    }
}

bool DataEngineThreadFactory::startDataIO(QThread::Priority prio) {
	if (!m_engine->m_dataIOThread) {
		DATA_ENGINE_DEBUG << "data IO thread not created; calling createDataIO()";
		m_engine->createDataIO();
	}
	if (!m_engine->m_dataIOThread) {
		DATA_ENGINE_DEBUG << "data IO thread could not be created.";
		m_engine->setSystemState(QSDR::DataProcessThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
		return false;
	}

		m_engine->m_dataIOThread->start(prio);

	if (m_engine->m_dataIOThread->isRunning()) {
					
		m_engine->m_dataIOThreadRunning = true;
		DATA_ENGINE_DEBUG << "data IO thread started.";

		return true;
	}
	else {

		m_engine->m_dataIOThreadRunning = false;
		m_engine->setSystemState(QSDR::DataProcessThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
		return false;
	}
}

void DataEngineThreadFactory::stopDataIO() {

	if (m_engine->m_dataIOThread && m_engine->m_dataIOThread->isRunning()) {
					
#ifdef HAVE_SOAPYSDR
        if (m_engine->m_soapySDRSource) {
            m_engine->m_soapySDRSource->stop();
        }
#endif
        if (m_engine->m_dataIO) {
            m_engine->m_dataIO->stop();
        }

		m_engine->m_dataIOThread->quit();

		// Wait for a clean exit; never delete a still-running QThread (segfault on quit).
		if (!m_engine->m_dataIOThread->wait(5000)) {
			DATA_ENGINE_DEBUG << "data IO thread did not finish within 5s; terminating.";
			m_engine->m_dataIOThread->terminate();
			m_engine->m_dataIOThread->wait(1000);
		}
		m_engine->m_dataIOThreadRunning = false;
		
		delete m_engine->m_dataIOThread;
        m_engine->m_dataIOThread = nullptr;

		// Keep DataIO alive for queues/mutex across stop/start and discovery.
		if (m_engine->m_dataIO) {
			m_engine->m_dataIO->moveToThread(m_engine->thread());
			m_engine->m_dataIO->setProtocol(nullptr);
		}

#ifdef HAVE_SOAPYSDR
        delete m_engine->m_soapySDRSource;
        m_engine->m_soapySDRSource = nullptr;
#endif

		DATA_ENGINE_DEBUG << "data IO thread deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "data IO thread wasn't started.";
}


void DataEngineThreadFactory::createDataProcessor() {

	m_engine->m_dataProcessor = new DataProcessor(m_engine, m_engine->m_serverMode, m_engine->m_hwInterface);

    // Connect audio input to the newly created processor
    if (m_engine->m_audioInput) {
        CHECKED_CONNECT(
                m_engine->m_audioInput,
                SIGNAL(tx_mic_data_ready()),
                m_engine->m_dataProcessor,
                SLOT(processSoapyMicData()));
    }

	m_engine->sendSocket = new QUdpSocket();
    m_engine->m_controlSocket = new QUdpSocket();
    if (!m_engine->m_controlSocket->bind(QHostAddress::AnyIPv4, 0)) {
        DATA_ENGINE_DEBUG << "Warning: Could not bind m_controlSocket.";
    }
    QObject::connect(
			m_engine->sendSocket,
        &QAbstractSocket::errorOccurred,
			m_engine->m_dataProcessor,
        &DataProcessor::displayDataProcessorSocketError
        );



	switch (m_engine->m_serverMode) {
		
		// The signal iqDataReady is generated by the function
		// processInputBuffer when a block of input data are
		// decoded.

		case QSDR::SDRMode:
			/*QObject::connect(
				this,
				SIGNAL(iqDataReady(int)),
				SLOT(dttSPDspProcessing(int)),
				Qt::DirectConnection);*/

			break;
			
		case QSDR::NoServerMode:
        break;
    }

	m_engine->m_dataProcThread = new QThreadEx();
	m_engine->m_dataProcessor->moveToThread(m_engine->m_dataProcThread);
	m_engine->sendSocket->moveToThread(m_engine->m_dataProcThread);
    if (m_engine->m_controlSocket) {
        m_engine->m_controlSocket->moveToThread(m_engine->m_dataProcThread);
    }

	switch (m_engine->m_hwInterface) {

		case QSDR::NoInterfaceMode:
            /*
			m_dataProcessor->connect(
						m_dataProcThread, 
						&QThread::started, 
						m_dataProcessor, 
						&DataProcessor::processData);
            */
			break;
			
		case QSDR::Metis:
		case QSDR::Hermes:

		    /*
			m_dataProcessor->connect(
						m_dataProcThread, 
						SIGNAL(started()), 
						SLOT(processDeviceData()));
*/

			if (m_engine->m_dataIO) {
				CHECKED_CONNECT(
						m_engine->m_dataIO,
						&DataIO::readydata,
						m_engine->m_dataProcessor,
						&DataProcessor::processReadData);
            } else {
				DATA_ENGINE_DEBUG << "createDataProcessor: no data source found, skipping readydata connection.";
			}

            break;

#ifdef HAVE_SOAPYSDR
        case QSDR::SoapySDR:
            if (m_engine->m_soapySDRSource) {
                qDebug() << "DataEngine: Connecting m_soapySDRSource::readydata to DataProcessor";
                CHECKED_CONNECT(
                        m_engine->m_soapySDRSource,
                        &SoapySDRDataSource::readydata,
                        m_engine->m_dataProcessor,
                        &DataProcessor::processReadData);
                // Also trigger one immediate check in case data is already waiting
                QMetaObject::invokeMethod(m_engine->m_dataProcessor, "processReadData", Qt::QueuedConnection);
            }
            break;
#endif
    }
}


bool DataEngineThreadFactory::startDataProcessor(QThread::Priority prio) {

	m_engine->m_dataProcThread->start(prio);
				
	if (m_engine->m_dataProcThread->isRunning()) {
					
		m_engine->m_dataProcThreadRunning = true;
		m_engine->m_dataIO->networkIOMutex.lock();
		DATA_ENGINE_DEBUG << "data processor thread started.";
		m_engine->m_dataIO->networkIOMutex.unlock();

		return true;
	}
	else {

		m_engine->m_dataProcThreadRunning = false;
		m_engine->setSystemState(QSDR::DataProcessThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
		return false;
	}
}

void DataEngineThreadFactory::stopDataProcessor() {

	if (m_engine->m_dataProcThread->isRunning()) {
		if (m_engine->m_dataProcessor) {
			QMetaObject::invokeMethod(m_engine->m_dataProcessor,
								  &DataProcessor::stopControlTimer,
								  Qt::BlockingQueuedConnection);
			QMetaObject::invokeMethod(m_engine->m_dataProcessor,
								  &DataProcessor::stop,
								  Qt::BlockingQueuedConnection);
		}
		
		if (m_engine->m_serverMode == QSDR::SDRMode ) {
			
			if (m_engine->m_dataIO->iq_queue.isEmpty()) {
				m_engine->m_dataIO->iq_queue.enqueue(TIQPacket(QByteArray(BUFFER_SIZE, 0x0), 0));
			}
		}

		m_engine->m_dataProcThread->quit();
		m_engine->m_dataProcThread->wait();
		delete m_engine->m_dataProcThread;
        m_engine->m_dataProcThread = nullptr;
		delete m_engine->m_dataProcessor;
		m_engine->m_dataProcessor = nullptr;

		if (m_engine->m_serverMode == QSDR::SDRMode ) {

			while (!m_engine->m_dataIO->iq_queue.isEmpty())
				m_engine->m_dataIO->iq_queue.dequeue();

			DATA_ENGINE_DEBUG << "iq_queue empty.";
		}

		m_engine->m_dataProcThreadRunning = false;

		DATA_ENGINE_DEBUG << "data processor thread deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "data processor thread wasn't started.";
}


void DataEngineThreadFactory::createAudioOutProcessor() {

	m_engine->m_audioOutProcessor = new AudioOutProcessor(m_engine, m_engine->m_serverMode);
	m_engine->m_audioOutProcThread = new QThreadEx();
	m_engine->m_audioOutProcessor->moveToThread(m_engine->m_audioOutProcThread);
}

void DataEngineThreadFactory::startAudioOutProcessor(QThread::Priority prio) {

	Q_UNUSED (prio)
}

void DataEngineThreadFactory::stopAudioOutProcessor() {
}


void DataEngineThreadFactory::createWideBandDataProcessor() {

	int size;

	if (m_engine->mercuryFW > 32 || m_engine->hermesFW > 11)
		size = BIGWIDEBANDSIZE;
	else
		size = SMALLWIDEBANDSIZE;
	
	m_engine->m_wbDataProcessor = new WideBandDataProcessor(m_engine, m_engine->m_serverMode, size);

	QObject::connect(m_engine->set, &Settings::spectrumAveragingCntChanged,
			m_engine, &DataEngine::setWbSpectrumAveraging);

	if (RadioTelemetry* tel = m_engine->m_radioModel ? m_engine->m_radioModel->telemetry() : nullptr) {
		QObject::connect(m_engine->m_wbDataProcessor, &WideBandDataProcessor::wbSpectrumBufferChanged,
		        tel, &RadioTelemetry::setWidebandSpectrumBuffer);
	}


	m_engine->m_wbDataProcThread = new QThreadEx();
	m_engine->m_wbDataProcessor->moveToThread(m_engine->m_wbDataProcThread);
	m_engine->m_wbDataProcessor->connect(
							m_engine->m_wbDataProcThread, 
							&QThread::started, 
							m_engine->m_wbDataProcessor, 
							&WideBandDataProcessor::processWideBandData);
}

bool DataEngineThreadFactory::startWideBandDataProcessor(QThread::Priority prio) {
	m_engine->m_wbDataProcThread->start(prio);//(QThread::TimeCriticalPriority);//(QThread::HighPriority);//(QThread::LowPriority);

	if (m_engine->m_wbDataProcThread->isRunning()) {
					
		m_engine->m_wbDataRcvrThreadRunning = true;
		m_engine->m_dataIO->networkIOMutex.lock();
		DATA_ENGINE_DEBUG << "wide band data processor thread started.";
		m_engine->m_dataIO->networkIOMutex.unlock();

		return true;
	}
	else {

		m_engine->m_wbDataRcvrThreadRunning = false;
		m_engine->setSystemState(QSDR::WideBandDataProcessThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
		return false;
	}
}

void DataEngineThreadFactory::stopWideBandDataProcessor() {

	if (m_engine->m_wbDataProcThread->isRunning()) {
					
		m_engine->m_wbDataProcessor->stop();
		if (m_engine->m_dataIO->wb_queue.isEmpty())
			m_engine->m_dataIO->wb_queue.enqueue(m_engine->m_datagram);

		m_engine->m_wbDataProcThread->quit();
		m_engine->m_wbDataProcThread->wait();
		delete m_engine->m_wbDataProcThread;
        m_engine->m_wbDataProcThread = nullptr;
		delete m_engine->m_wbDataProcessor;
		m_engine->m_wbDataProcessor = nullptr;

		m_engine->m_wbDataRcvrThreadRunning = false;
		
		DATA_ENGINE_DEBUG << "wide band data processor thread deleted.";
	}
	else
		DATA_ENGINE_DEBUG << "wide band data processor thread wasn't started.";
}


void DataEngineThreadFactory::createAudioReceiver() {

	m_engine->m_audioReceiver = new AudioReceiver(m_engine);

	QObject::connect(m_engine->m_audioReceiver, &AudioReceiver::rcveIQEvent,
			m_engine, &DataEngine::setRcveIQSignal);

	QObject::connect(m_engine->m_audioReceiver, &AudioReceiver::clientConnectedEvent,
			m_engine, qOverload<bool>(&DataEngine::setClientConnected));

	
	m_engine->m_AudioRcvrThread = new QThreadEx();
	m_engine->m_audioReceiver->moveToThread(m_engine->m_AudioRcvrThread);

	m_engine->m_audioReceiver->connect(
						m_engine->m_AudioRcvrThread, 
						&QThread::started, 
						m_engine->m_audioReceiver, 
						&AudioReceiver::initClient);
}


void DataEngineThreadFactory::createAudioInputProcessor() {

    m_engine->m_audioInput = new TransmitAudioInput();

    // Give the TCI server the network-mic queue so remote client TX audio can
    // be injected into the transmit path (network audio takes over the mic
    // input when frames arrive; see DataProcessor::fetch_MicData).
    if (m_engine->set && m_engine->set->tciServer())
        m_engine->set->tciServer()->setTransmitAudioQueue(&m_engine->m_audioInput->m_netAudioInQueue);

    m_engine->m_cwIO = new iambic(m_engine);

    QObject::connect(m_engine->m_dataProcessor, &DataProcessor::keyer_event,
            m_engine->m_cwIO, &iambic::keyer_event, Qt::DirectConnection);
    QObject::connect(m_engine->m_cwIO, &iambic::key_down,
            m_engine->m_dataProcessor, &DataProcessor::key_down, Qt::DirectConnection);

    // Connect to any SliceProcessors already in RX list (e.g. SoapySDR path
    // where initReceivers runs before createAudioInputProcessor).
    for (SliceProcessor* rx : qAsConst(m_engine->RX)) {
        if (rx) {
            QObject::connect(m_engine->m_cwIO, &iambic::key_down,
                    rx, &SliceProcessor::cwKeyDown, Qt::DirectConnection);
        }
    }

    m_engine->m_cwIO->Start();
}

