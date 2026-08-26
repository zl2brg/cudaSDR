/**
* @file  DataEngineSoapy.cpp
* @brief SoapySDR discovery/start helpers for DataEngine
*/

#include "DataEngineSoapy.h"
#include "cusdr_dataEngine.h"

DataEngineSoapy::DataEngineSoapy(DataEngine *engine)
	: m_engine(engine)
{
}

#ifdef HAVE_SOAPYSDR
void DataEngineSoapy::searchSoapyDevices() {
    if (!m_engine->m_discoverer) m_engine->createDiscoverer();
    if (!m_engine->m_discoveryThread->isRunning()) {
        m_engine->m_discoveryThread->start();
    }
    QMetaObject::invokeMethod(m_engine->m_discoverer, "discoverSoapyDevices", Qt::QueuedConnection);
}


bool DataEngineSoapy::startSoapyEngine() {
        if (!m_engine->m_soapySDRSource) m_engine->createDataIO();
        m_engine->initReceivers(1);

        if (!m_engine->m_audioInput)
            m_engine->createAudioInputProcessor();
        m_engine->m_audioInput->Setup();

        // Start DSP threads for all receivers (skipped by the normal HPSDR path)
        for (int i = 0; i < m_engine->m_dspThreadList.size(); ++i) {
            m_engine->m_dspThreadList.at(i)->start(QThread::HighPriority);
            DATA_ENGINE_DEBUG << "SoapySDR: started DSP thread for rx" << i;
        }

        if (!m_engine->m_dataProcessor) m_engine->createDataProcessor();

        if (!m_engine->startDataIO(QThread::HighPriority)) {
            m_engine->setSystemState(QSDR::DataReceiverThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
            return false;
        }

        if (!m_engine->startDataProcessor(QThread::HighPriority)) {
            m_engine->setSystemState(QSDR::DataProcessThreadError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineDown);
            return false;
        }

        m_engine->setSystemState(QSDR::NoError, m_engine->m_hwInterface, m_engine->m_serverMode, QSDR::DataEngineUp);
        m_engine->set->setRadioState(RadioState::RX);
        return true;
}

#endif
