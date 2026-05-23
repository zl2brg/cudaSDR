#include "Models/SliceModel.h"
/**
* @file cusdr_receiver.cpp
* @brief cuSDR receiver class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-11-12
*/

/* Copyright (C)
*
* 2010 - Hermann von Hasseln, DL3HVH
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.tw
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/
#define LOG_RECEIVER

// use: RECEIVER_DEBUG

#include "cusdr_receiver.h"

namespace {
constexpr int HIGH_RATE_TRANSITION_DROP_BUFFERS = 12;
}

Receiver::Receiver(SliceModel *model, QObject *parent)
	: QObject(parent)
        , m_sliceModel(model)
	, set(Settings::instance())
	, m_stopped(false)
	, m_receiver(model ? model->id() : 0)
	, m_samplerate(set->getSampleRate())
	, m_audioMode(1)
	, m_rateTransitionDropBuffers(0)
	//, m_calOffset(63.0)
	//, m_calOffset(33.0)
{
	InitCPX(inBuf, BUFFER_SIZE, 0.0f);
	InitCPX(outBuf, BUFFER_SIZE, 0.0f);
    InitCPX(audioOutputBuf, BUFFER_SIZE, 0.0f);
    setAudioBufferSize();
    newSpectrum.resize(BUFFER_SIZE*4);
	highResTimer = std::make_unique<HResTimer>();
#ifdef USE_INTERNAL_AUDIO
    m_audioOutput = new ReceiverAudioOutput(this);
    m_audioOutput->start();
#endif
#ifdef HAVE_CODEC2
	m_freeDVMode = set->getFreeDVMode(m_receiver);
	m_freeDVProcessor = new FreeDVProcessor(m_freeDVMode);
#endif
	setupConnections();
    m_displayTime = (int)(1000000.0/set->getFramesPerSecond(m_receiver));
	m_smeterTime.start();
}

Receiver::~Receiver() {
    qDebug() << "Destroy Receiver " << m_receiver;
    inBuf.clear();
    outBuf.clear();
	if (m_audioOutput) {
#ifdef USE_INTERNAL_AUDIO
		m_audioOutput->stop();
#endif
		delete m_audioOutput;
		m_audioOutput = nullptr;
	}
    m_stopped = false;
    delete qtwdsp;
#ifdef HAVE_CODEC2
	delete m_freeDVProcessor;
	m_freeDVProcessor = nullptr;
#endif
}

void Receiver::setAudioBufferSize() {
    int scale=m_samplerate/48000;
    m_audiobuffersize = 1024/scale;
    RECEIVER_DEBUG << "set Audio buffer size to: " << m_audiobuffersize;
    }

void Receiver::setupConnections() {
    connect(set, &Settings::systemStateChanged,
            this, &Receiver::setSystemState);

    connect(set, &Settings::sampleRateChanged,
            this, &Receiver::setSampleRate);

    connect(set, &Settings::framesPerSecondChanged,
            this, &Receiver::setFramesPerSecond);

#ifdef HAVE_CODEC2
	connect(set, &Settings::freeDVModeChanged,
			this, &Receiver::setFreeDVMode);
#endif
}

bool Receiver::initDSPInterface() {

	if (set->getReceiverDspCore(m_receiver) == QSDR::QtDSP) {

        if (!initQtWDSPInterface()) return false;

	}
	return true;
}



bool Receiver::initQtWDSPInterface() {

    RECEIVER_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "BUFFER_SIZE=" << BUFFER_SIZE;
//    qtwdsp = std::make_unique<QWDSPEngine>(this, m_receiver, BUFFER_SIZE);
    qtwdsp = new QWDSPEngine(m_sliceModel, this, BUFFER_SIZE);

    if (!qtwdsp || !qtwdsp->isValid()) {  // Add validity check
        RECEIVER_DEBUG << "[RX-ADD] ERROR: could not start QWtDSP for receiver: " << m_receiver;
        return false;
    }
    RECEIVER_DEBUG << "[RX-ADD] QWDSPEngine constructed for rx=" << m_receiver << "(isValid=true)";

    qtwdsp->setQtDSPStatus(true);
    const float volume = m_sliceModel ? m_sliceModel->volume() : static_cast<float>(set->getMainVolume(m_receiver));
    qtwdsp->setVolume(volume);

    const DSPMode mode = m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver);
    RECEIVER_DEBUG << "[RX-ADD] rx=" << m_receiver << "set DSP mode to:" << set->getDSPModeString(mode);

    qtwdsp->setDSPMode(mode);

    if (m_sliceModel) {
        qtwdsp->setFilter(m_sliceModel->filterLow(), m_sliceModel->filterHigh());
        RECEIVER_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "complete (filter lo=" << m_sliceModel->filterLow() << "hi=" << m_sliceModel->filterHigh() << ")";
    } else {
        const long ctrHz = set->getCtrFrequency(m_receiver);
        auto filter = getFilterFromDSPMode(set->getDefaultFilterList(),
                                           resolveWDSPMode(mode, ctrHz));
        qtwdsp->setFilter(filter.filterLo, filter.filterHi);
        RECEIVER_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "complete (filter lo=" << filter.filterLo << "hi=" << filter.filterHi << ")";
    }
    return true;
}

void Receiver::enqueueRawData() {
    QVector<int32_t> rawBlock;
    rawBlock.reserve(BUFFER_SIZE * 2);
    for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
        rawBlock.append(m_rawIQ[i]);
    }

    if (m_iqQueue.isFull()) {
        RECEIVER_DEBUG << "iqQueue full! dropping oldest packet";
        m_iqQueue.dequeue();
    }
    m_iqQueue.enqueue(rawBlock);
}

void Receiver::enqueueData() {
    // Legacy support or internal use
    if (m_iqQueue.isFull()) {
        RECEIVER_DEBUG << "iqQueue full! dropping oldest packet";
        m_iqQueue.dequeue();
    }
    // Convert CPX to raw int for now if this is ever called, or just do nothing
}

void Receiver::stop() {

	m_mutex.lock();
	m_stopped = true;
	m_mutex.unlock();
}

void Receiver::dspProcessing() {
	static quint64 dspEntryCount = 0;
	static quint64 dspEmptyQueueCount = 0;

	++dspEntryCount;
	if ((dspEntryCount % 100) == 1) {
		RECEIVER_DEBUG << "dspProcessing entry rx=" << m_receiver
					   << " count=" << dspEntryCount
					   << " iqQueue=" << m_iqQueue.count();
	}

	if (m_iqQueue.isEmpty()) {
		++dspEmptyQueueCount;
		if ((dspEmptyQueueCount % 100) == 1) {
			RECEIVER_DEBUG << "dspProcessing empty iqQueue rx=" << m_receiver
						   << " emptyCount=" << dspEmptyQueueCount;
		}
		return;
	}

	{
		QMutexLocker locker(&m_mutex);
		if (m_rateTransitionDropBuffers > 0) {
			m_iqQueue.dequeue();
			--m_rateTransitionDropBuffers;
			return;
		}
	}
    
    QVector<int32_t> rawIQ = m_iqQueue.dequeue();
    
    // Perform 24-bit integer to double conversion in this thread
    // This offloads work from the bottleneck DataProcessor thread.
    const double scale = 1.0 / 8388607.0;
    cpx* inPtr = inBuf.data(); // Trigger detach once
    const int32_t* rawPtr = rawIQ.constData();
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        inPtr[i].re = (double)rawPtr[2*i] * scale;
        inPtr[i].im = (double)rawPtr[2*i+1] * scale;
    }

    int spectrumDataReady;
    
    m_dspMutex.lock();
    m_dspCallTimer.start();
    qtwdsp->processDSP(inBuf, audioOutputBuf);
    double dspUs = m_dspCallTimer.nsecsElapsed() / 1000.0;
    m_dspMutex.unlock();

    ++m_dspCallCount;
    if (dspUs < m_dspTimeMin) m_dspTimeMin = dspUs;
    if (dspUs > m_dspTimeMax) m_dspTimeMax = dspUs;
    m_dspTimeAccum += dspUs;

    static constexpr quint64 DSP_REPORT_INTERVAL = 500;
    if ((m_dspCallCount % DSP_REPORT_INTERVAL) == 0) {
        double mean = m_dspTimeAccum / DSP_REPORT_INTERVAL;
        RECEIVER_DEBUG << "DSP perf rx=" << m_receiver
                       << " calls=" << m_dspCallCount
                       << " mean=" << QString::number(mean, 'f', 1) << "Âµs"
                       << " min="  << QString::number(m_dspTimeMin, 'f', 1) << "Âµs"
                       << " max="  << QString::number(m_dspTimeMax, 'f', 1) << "Âµs"
                       << " budget=" << QString::number(getDisplayDelay(), 'f', 0) << "Âµs"
                       << " iqQ=" << m_iqQueue.count();
        m_dspTimeAccum = 0.0;
        m_dspTimeMin = 1e9;
        m_dspTimeMax = 0.0;
    }

      if (highResTimer->getElapsedTimeInMicroSec() >= getDisplayDelay()) {

        
		if (m_state == RadioState::RX) {
			GetPixels(m_receiver, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
		} else {
			GetPixels(TX_ID, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);

			// TX spectrum can be temporarily unavailable depending on DSP timing.
			// Fall back to receiver spectrum so panadapter updates don't stall in TX.
			if (!spectrumDataReady) {
				qDebug() << "Tx spectrum fetch fail; falling back to RX spectrum";
				GetPixels(m_receiver, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
			}
		}

        if (spectrumDataReady) {
            newSpectrum = qtwdsp->spectrumBuffer;  // Direct assignment
            emit spectrumBufferChanged(m_receiver, newSpectrum);
        }
        
        highResTimer->start();
    }

    if (m_receiver == set->getCurrentReceiver()) {
        // Consider if this needs mutex protection too
        if (m_smeterTime.elapsed() > 200) {
            m_sMeterValue = qtwdsp->getSMeterInstValue();
            emit sMeterValueChanged(m_receiver, m_sMeterValue);
            m_smeterTime.restart();
        }
#ifdef USE_INTERNAL_AUDIO
		const DSPMode dspMode = m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver);
		if (dspMode != DSPMode::FDV) {
			// Normal analogue modes: pass WDSP audio output straight to soundcard.
			m_audioOutput->writeAudio(interleaveFromCPX(audioOutputBuf, m_audiobuffersize));
		}
		else {
			QVector<float> mono(m_audiobuffersize);
			const cpx* src = audioOutputBuf.constData();
			for (int i = 0; i < m_audiobuffersize; ++i)
				mono[i] = static_cast<float>(src[i].re);

			bool wroteAudio = false;

#ifdef HAVE_CODEC2
			if (m_freeDVProcessor) {
				QVector<float> speech = m_freeDVProcessor->processSamples(mono.constData(), m_audiobuffersize);
				// processSamples always returns n*2 floats (silence-padded when no
				// frame is ready), so write unconditionally — this prevents the
				// passthrough from adding a second burst of audio.
				m_audioOutput->writeAudio(speech);
				wroteAudio = true;
				if (m_freeDVProcessor->isSync())
					m_freeDVRxFrames += 1;

				if ((m_dspCallCount % 50) == 1) {
					set->setFreeDVStatus(
						m_receiver,
						m_freeDVProcessor->isSync(),
						m_freeDVProcessor->getSNR(),
						m_freeDVRxFrames);
				}
			}
#endif

			if (!wroteAudio) {
				QVector<float> passthrough;
				passthrough.reserve(m_audiobuffersize * 2);
				for (int i = 0; i < m_audiobuffersize; ++i) {
					const float s = mono.at(i);
					passthrough.append(s);
					passthrough.append(s);
				}
				m_audioOutput->writeAudio(passthrough);
			}
		}
#endif // USE_INTERNAL_AUDIO
        emit audioBufferSignal(m_receiver, audioOutputBuf, m_audiobuffersize);
    }
}

void Receiver::setFreeDVMode(int rx, int mode) {
	#ifdef HAVE_CODEC2
	if (rx != m_receiver) return;
	if (m_freeDVMode == mode) return;

	m_freeDVMode = mode;
	m_freeDVRxFrames = 0;

	if (m_freeDVProcessor) {
		delete m_freeDVProcessor;
		m_freeDVProcessor = nullptr;
	}

	m_freeDVProcessor = new FreeDVProcessor(m_freeDVMode);
	set->setFreeDVStatus(m_receiver, false, 0.0f, 0);
#else
	Q_UNUSED(rx)
	Q_UNUSED(mode)
#endif
}

QVector<float> Receiver::interleaveFromCPX(const CPX& in, int size) {
    int limit = (size < 0 || size > in.size()) ? in.size() : size;
    QVector<float> out(limit * 2); 
    float* outData = out.data();
    const cpx* inData = in.constData();

    for (int i = 0; i < limit; i++) {
        *outData++ = (float)inData[i].re;
        *outData++ = (float)inData[i].im;
    }
    return out;
}

void Receiver::setSampleRate(int value) {
	if (m_samplerate == value) return;
    const int previousRate = m_samplerate;

	switch (value) {
		case 48000:
		case 96000:
		case 192000:
		case 384000:
		case 768000:
		case 1536000:
			m_samplerate = value;
			break;
		default:
			RECEIVER_DEBUG << "invalid sample rate (possible values are: 48, 96, 192, 384, 768, or 1536 kHz)!\n";
			break;
	}

	if (qtwdsp) {
        m_mutex.lock();

		// Queue flush and drop-counter are now handled unconditionally below.
		const bool highRateTransition = (previousRate >= 768000 || m_samplerate >= 768000);
		(void)highRateTransition;

		setAudioBufferSize();

		// Flush the queue and drop a few buffers after any rate transition so
		// fexchange0 is not called on the channel while it is being rebuilt.
		while (!m_iqQueue.isEmpty())
			m_iqQueue.dequeue();
		m_rateTransitionDropBuffers = HIGH_RATE_TRANSITION_DROP_BUFFERS;

        qtwdsp->setSampleRate(m_samplerate);
        m_mutex.unlock();

    }
	else
		RECEIVER_DEBUG << "qtdsp down: cannot set sample rate!\n";
}

void Receiver::setServerMode(QSDR::_ServerMode mode) {

	m_serverMode = mode;
}

QSDR::_ServerMode Receiver::getServerMode()	const {

	return m_serverMode;
}

//void Receiver::setSocketState(SocketState state) {
//
//	m_socketState = state;
//}

//Receiver::SocketState Receiver::socketState() const {
//
//	return m_socketState;
//}

void Receiver::setSystemState(
	QSDR::_Error err,
	QSDR::_HWInterfaceMode hwmode,
	QSDR::_ServerMode mode,
	QSDR::_DataEngineState state)
{
	Q_UNUSED (err)

	if (m_hwInterface != hwmode)
		m_hwInterface = hwmode;

	if (m_serverMode != mode)
		m_serverMode = mode;

	if (m_dataEngineState != state)
		m_dataEngineState = state;
}

void Receiver::setAudioMode(int mode) {

	if (m_audioMode == mode) return;

	m_audioMode = mode;
}

//void Receiver::setID(int value) {
//
//	m_receiverID = value;
//	RECEIVER_DEBUG << "This is receiver " << m_receiverID;
//}

void Receiver::setReceiver(int value) {

	m_receiver = value;
}

void Receiver::setFramesPerSecond(int rx, int value) {

	if (m_receiver == rx)
		m_displayTime = (int)(1000000.0/value);
}

void Receiver::setPeerAddress(QHostAddress addr) {

	m_peerAddress = addr;
}

void Receiver::setSocketDescriptor(int value) {

	m_socketDescriptor = value;
}

void Receiver::setClient(int value) {

	m_client = value;
}

void Receiver::setIQPort(int value) {

	m_iqPort = value;
}

void Receiver::setBSPort(int value) {

	m_bsPort = value;
}

void Receiver::setConnectedStatus(bool value) {

	m_connected = value;
}
