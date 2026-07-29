#include "Models/SliceModel.h"
/**
* @file cusdr_sliceProcessor.cpp
* @brief Per-slice DSP worker class
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-11-12
*/

/* Copyright (C)
*
* 2010 - Hermann von Hasseln, DL3HVH
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public License version 2 as
* published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.tw
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02110-1301, USA.
*
*/
#define LOG_SLICE_PROCESSOR

// use: SLICE_PROCESSOR_DEBUG

#include "cusdr_sliceProcessor.h"

namespace {
constexpr int HIGH_RATE_TRANSITION_DROP_BUFFERS = 12;
constexpr qint64 RETUNE_AUDIO_MUTE_MS = 120;
}

SliceProcessor::SliceProcessor(SliceModel *model, QObject *parent)
	: QObject(parent)
        , m_sliceModel(model)
	, set(Settings::instance())
	, m_stopped(false)
	, m_receiver(model ? model->id() : 0)
	, m_samplerate(set->getSampleRate())
    , m_soapyInputSampleRate(set->getSampleRate())
	, m_audioMode(1)
    , m_iqQueue(100)
    , m_soapyQueue(100)
    , m_soapyDspPending(false)
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
	// No parent: the sink must stay on the thread that created it. SliceProcessor is
	// moved to a DSP thread, which would drag QAudioSink's socket notifiers along and
	// leave them registered on the wrong event dispatcher across a device change.
    m_audioOutput = new ReceiverAudioOutput(nullptr);
    m_audioOutput->start();
    m_audioAccumulator.resize(1024 * 2); // Buffer up to 1024 stereo samples
    m_audioAccumulatorFill = 0;
#endif
#ifdef HAVE_CODEC2
	m_freeDVMode = set->getFreeDVMode(m_receiver);
	if (m_freeDVMode == 100) {
#ifdef HAVE_RADE
		m_radeProcessor = new RadeProcessor();
#endif
	} else {
		m_freeDVProcessor = new FreeDVProcessor(m_freeDVMode);
	}
#endif
	setupConnections();
    m_displayTime = (int)(1000000.0/set->getFramesPerSecond(m_receiver));
	m_smeterTime.start();
    m_retuneTimer.start();
}

SliceProcessor::~SliceProcessor() {
    qDebug() << "Destroy SliceProcessor " << m_receiver;
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
#ifdef HAVE_RADE
	delete m_radeProcessor;
	m_radeProcessor = nullptr;
#endif
}

void SliceProcessor::setAudioBufferSize() {
    int scale=m_samplerate/48000;
    m_audiobuffersize = 1024/scale;
    SLICE_PROCESSOR_DEBUG << "set Audio buffer size to: " << m_audiobuffersize;
    }

void SliceProcessor::setupConnections() {
    connect(set, &Settings::systemStateChanged,
            this, &SliceProcessor::setSystemState);

    connect(set, &Settings::sampleRateChanged,
            this, &SliceProcessor::setSampleRate);

    connect(set, &Settings::framesPerSecondChanged,
            this, &SliceProcessor::setFramesPerSecond);

#ifdef HAVE_CODEC2
	connect(set, &Settings::freeDVModeChanged,
			this, &SliceProcessor::setFreeDVMode);
#endif

#ifdef HAVE_SOAPYSDR
    connect(set, &Settings::soapyAutoCalibrateChanged,
            this, &SliceProcessor::resetSoapyDcEstimator);
#endif

#ifdef HAVE_SOAPYSDR
    if (m_sliceModel && set->getHWInterface() == QSDR::SoapySDR) {
        connect(m_sliceModel, &SliceModel::frequencyChanged,
                this, &SliceProcessor::noteRetuneActivity);
        connect(m_sliceModel, &SliceModel::centerFrequencyChanged,
                this, &SliceProcessor::noteRetuneActivity);
    }
#endif
}

#ifdef HAVE_SOAPYSDR
void SliceProcessor::resetSoapyDcEstimator()
{
    m_soapyDcAvgI = 0.0;
    m_soapyDcAvgQ = 0.0;
}
#endif

bool SliceProcessor::initDSPInterface() {

	if (set->getReceiverDspCore(m_receiver) == QSDR::QtDSP) {

        if (!initQtWDSPInterface()) return false;

	}
	return true;
}



bool SliceProcessor::initQtWDSPInterface() {

    SLICE_PROCESSOR_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "BUFFER_SIZE=" << BUFFER_SIZE;
//    qtwdsp = std::make_unique<QWDSPEngine>(this, m_receiver, BUFFER_SIZE);
    qtwdsp = new QWDSPEngine(m_sliceModel, this, BUFFER_SIZE);

    if (!qtwdsp || !qtwdsp->isValid()) {  // Add validity check
        SLICE_PROCESSOR_DEBUG << "[RX-ADD] ERROR: could not start QWtDSP for receiver: " << m_receiver;
        return false;
    }
    SLICE_PROCESSOR_DEBUG << "[RX-ADD] QWDSPEngine constructed for rx=" << m_receiver << "(isValid=true)";

    qtwdsp->setQtDSPStatus(true);
    const float volume = m_sliceModel ? m_sliceModel->volume() : static_cast<float>(set->getMainVolume(m_receiver));
    qtwdsp->setVolume(volume);

    const DSPMode mode = m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver);
    SLICE_PROCESSOR_DEBUG << "[RX-ADD] rx=" << m_receiver << "set DSP mode to:" << set->getDSPModeString(mode);

    qtwdsp->setDSPMode(mode);

    if (m_sliceModel) {
        qtwdsp->setFilter(m_sliceModel->filterLow(), m_sliceModel->filterHigh());
        SLICE_PROCESSOR_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "complete (filter lo=" << m_sliceModel->filterLow() << "hi=" << m_sliceModel->filterHigh() << ")";
    } else {
        const long ctrHz = set->getCtrFrequency(m_receiver);
        auto filter = getFilterFromDSPMode(set->getDefaultFilterList(),
                                           resolveWDSPMode(mode, ctrHz));
        qtwdsp->setFilter(filter.filterLo, filter.filterHi);
        SLICE_PROCESSOR_DEBUG << "[RX-ADD] initQtWDSPInterface: rx=" << m_receiver << "complete (filter lo=" << filter.filterLo << "hi=" << filter.filterHi << ")";
    }
    return true;
}

void SliceProcessor::enqueueRawData() {
    QVector<int32_t> rawBlock;
    rawBlock.reserve(BUFFER_SIZE * 2);
    for (int i = 0; i < BUFFER_SIZE * 2; ++i) {
        rawBlock.append(m_rawIQ[i]);
    }

    if (m_iqQueue.isFull()) {
        m_iqQueue.dequeue();
        ++m_iqQueueDropCount;
    }
    m_iqQueue.enqueue(rawBlock);

    if (!m_queueDropLogTimer.isValid())
        m_queueDropLogTimer.start();
    if (m_queueDropLogTimer.elapsed() >= 5000) {
        if (m_iqQueueDropCount > 0 || m_soapyQueueDropCount > 0) {
            SLICE_PROCESSOR_DEBUG << "[RX" << m_receiver << "] queue drops in last 5s: iq="
                                  << m_iqQueueDropCount << " soapy=" << m_soapyQueueDropCount;
        }
        m_iqQueueDropCount = 0;
        m_soapyQueueDropCount = 0;
        m_queueDropLogTimer.restart();
    }
}

void SliceProcessor::enqueueRawData(const QVector<int32_t> &rawBlock) {
    if (m_iqQueue.isFull()) {
        m_iqQueue.dequeue();
        ++m_iqQueueDropCount;
    }
    m_iqQueue.enqueue(rawBlock);

    if (!m_queueDropLogTimer.isValid())
        m_queueDropLogTimer.start();
    if (m_queueDropLogTimer.elapsed() >= 5000) {
        if (m_iqQueueDropCount > 0 || m_soapyQueueDropCount > 0) {
            SLICE_PROCESSOR_DEBUG << "[RX" << m_receiver << "] queue drops in last 5s: iq="
                                  << m_iqQueueDropCount << " soapy=" << m_soapyQueueDropCount;
        }
        m_iqQueueDropCount = 0;
        m_soapyQueueDropCount = 0;
        m_queueDropLogTimer.restart();
    }
}

void SliceProcessor::enqueueSoapyData(const QVector<float> &data) {
    if (m_soapyQueue.isFull()) {
        m_soapyQueue.dequeue();
        ++m_soapyQueueDropCount;
    }
    m_soapyQueue.enqueue(data);

    if (!m_queueDropLogTimer.isValid())
        m_queueDropLogTimer.start();
    if (m_queueDropLogTimer.elapsed() >= 5000) {
        if (m_iqQueueDropCount > 0 || m_soapyQueueDropCount > 0) {
            SLICE_PROCESSOR_DEBUG << "[RX" << m_receiver << "] queue drops in last 5s: iq="
                                  << m_iqQueueDropCount << " soapy=" << m_soapyQueueDropCount;
        }
        m_iqQueueDropCount = 0;
        m_soapyQueueDropCount = 0;
        m_queueDropLogTimer.restart();
    }
}

void SliceProcessor::setSoapyInputSampleRate(int value) {
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this,
                                  "setSoapyInputSampleRate",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, value));
        return;
    }

    if (value <= 0 || m_soapyInputSampleRate == value)
        return;

    m_soapyInputSampleRate = value;
    if (qtwdsp) {
        QMutexLocker dspLocker(&m_dspMutex);
        qtwdsp->setInputSampleRate(m_soapyInputSampleRate);
    }
}

void SliceProcessor::noteRetuneActivity(qint64)
{
#ifdef HAVE_SOAPYSDR
    if (set->getHWInterface() != QSDR::SoapySDR)
        return;

    if (!m_retuneTimer.isValid()) {
        m_retuneTimer.start();
    }

    const qint64 nowMs = m_retuneTimer.elapsed();
    const qint64 muteUntil = nowMs + RETUNE_AUDIO_MUTE_MS;
    if (muteUntil > m_audioMuteUntilMs) {
        m_audioMuteUntilMs = muteUntil;
    }
#endif
}

void SliceProcessor::enqueueData() {
    // Legacy support or internal use
    if (m_iqQueue.isFull()) {
        SLICE_PROCESSOR_DEBUG << "iqQueue full! dropping oldest packet";
        m_iqQueue.dequeue();
    }
    // Convert CPX to raw int for now if this is ever called, or just do nothing
}

void SliceProcessor::stop() {

	m_mutex.lock();
	m_stopped = true;
	m_mutex.unlock();
}

void SliceProcessor::dspProcessingSoapy() {
    // Flag is already set to true by trySetSoapyDspPending() in processReadData.
    
    while (!m_soapyQueue.isEmpty()) {
        const QVector<float> rawIQ = m_soapyQueue.dequeue();

        {
            QMutexLocker locker(&m_mutex);
            if (m_rateTransitionDropBuffers > 0) {
                --m_rateTransitionDropBuffers;
                continue;
            }
        }

        if (rawIQ.size() < BUFFER_SIZE * 2) continue;

        ++m_dspCallCount;

        cpx* inPtr = inBuf.data();
        const float* rawPtr = rawIQ.constData();
        const bool soapyDcRemove =
            (set->getHWInterface() == QSDR::SoapySDR && set->getSoapyAutoCalibrate());
        
        // cudaSDR/WDSP expects conjugated IQ for most USB SDR data paths.
        // We negate Q to flip the spectrum to the correct orientation.
        const bool negateQ = true; 

        constexpr double kDcAlpha = 0.004; // ~256-sample time constant at 48 kHz

        for (int i = 0; i < BUFFER_SIZE; ++i) {
            double I = static_cast<double>(rawPtr[2 * i]);
            double Q = static_cast<double>(rawPtr[2 * i + 1]);
            if (negateQ) Q = -Q;

            if (soapyDcRemove) {
                m_soapyDcAvgI += kDcAlpha * (I - m_soapyDcAvgI);
                m_soapyDcAvgQ += kDcAlpha * (Q - m_soapyDcAvgQ);
                I -= m_soapyDcAvgI;
                Q -= m_soapyDcAvgQ;
            }
            inPtr[i].re = I;
            inPtr[i].im = Q;
        }

        dspProcessingCore();
    }
    
    // Clear pending flag. 
    m_soapyDspPending.store(false, std::memory_order_release);
    
    // Safety check: if more data arrived between the loop and the flag clear, post again.
    if (!m_soapyQueue.isEmpty() && trySetSoapyDspPending()) {
        QMetaObject::invokeMethod(this, "dspProcessingSoapy", Qt::QueuedConnection);
    }
}

void SliceProcessor::dspProcessing() {
	if (m_iqQueue.isEmpty()) {
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
    dspProcessing(rawIQ);
}

void SliceProcessor::dspProcessing(const QVector<int32_t> &rawIQ) {
    if (rawIQ.size() < BUFFER_SIZE * 2) return;

    ++m_dspCallCount;

    // Perform 24-bit integer to double conversion
    const double scale = 1.0 / 8388607.0;
    cpx* inPtr = inBuf.data();
    const int32_t* rawPtr = rawIQ.constData();
    for (int i = 0; i < BUFFER_SIZE; ++i) {
        inPtr[i].re = (double)rawPtr[2*i] * scale;
        inPtr[i].im = (double)rawPtr[2*i+1] * scale;
    }

    dspProcessingCore();
}

void SliceProcessor::dspProcessingCore() {
    int spectrumDataReady;
    bool txPixelsRequested = false;

    // TCI raw IQ tap. inBuf holds the normalized input I/Q at the radio's RX
    // rate (m_samplerate) for both HPSDR (P1/P2) and SoapySDR. Capture it now,
    // before WDSP consumes inBuf, but emit it at the END of this function so
    // the RX audio frame (emitted mid-function) is queued onto the shared TCI
    // socket first — audio has priority over the droppable panadapter IQ.
    // Built only when a TCI client is subscribed, so the DSP hot path pays
    // nothing when nobody is listening.
    QVector<float> tciIqFrame;
    if (set->tciIqActive())
        tciIqFrame = interleaveFromCPX(inBuf);

    m_dspMutex.lock();
    m_dspCallTimer.start();
    qtwdsp->processDSP(inBuf, audioOutputBuf);
    double dspUs = m_dspCallTimer.nsecsElapsed() / 1000.0;
    m_dspMutex.unlock();

    if (dspUs < m_dspTimeMin) m_dspTimeMin = dspUs;
    if (dspUs > m_dspTimeMax) m_dspTimeMax = dspUs;
    m_dspTimeAccum += dspUs;

    static constexpr quint64 DSP_REPORT_INTERVAL = 500;
    if ((m_dspCallCount % DSP_REPORT_INTERVAL) == 0) {
        // Keep statistics updated for optional diagnostics, but do not log in hot path.
        m_dspTimeAccum = 0.0;
        m_dspTimeMin = 1e9;
        m_dspTimeMax = 0.0;
    }

      if (highResTimer->getElapsedTimeInMicroSec() >= getDisplayDelay()) {

#ifdef HAVE_SOAPYSDR
		if (set->getHWInterface() == QSDR::SoapySDR && set->is_transmitting()) {
			if (!set->getTxFullDuplex()) {
				// Half duplex: TX panadapter updated from get_tx_iqData() (RX DSP idle).
				spectrumDataReady = 0;
			} else {
                txPixelsRequested = true;
				GetPixels(TX_ID, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
				if (spectrumDataReady)
					applyTxPanadapterDisplayOffset(qtwdsp->spectrumBuffer);
				else
					GetPixels(m_receiver, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
			}
		} else
#endif
		if (m_state == RadioState::RX) {
			GetPixels(m_receiver, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
		} else {
            txPixelsRequested = true;
			GetPixels(TX_ID, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
			if (spectrumDataReady)
				applyTxPanadapterDisplayOffset(qtwdsp->spectrumBuffer);
			else
				GetPixels(m_receiver, 0, qtwdsp->spectrumBuffer.data(), &spectrumDataReady);
		}

        if (spectrumDataReady) {
            newSpectrum = qtwdsp->spectrumBuffer;  // Direct assignment
            emit spectrumBufferChanged(m_receiver, newSpectrum);
        }

        static const bool txPanDiagEnabled = (qEnvironmentVariableIntValue("CUSDR_TX_DIAG") != 0);
        if (txPanDiagEnabled && txPixelsRequested && (m_dspCallCount % 100) == 1) {
            qDebug().nospace() << "[TX-PAN-DIAG] rx=" << m_receiver
                               << " mode=" << (m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver))
                               << " state=" << m_state
                               << " txPixels=" << (spectrumDataReady ? "yes" : "no");
        }
        highResTimer->start();
    }

    if (m_receiver == set->getCurrentReceiver()) {
        int audioSamplesThisCall = m_audiobuffersize;
#ifdef HAVE_SOAPYSDR
        if (set->getHWInterface() == QSDR::SoapySDR && m_soapyInputSampleRate > 0) {
            // WDSP channel output is fixed at 48 kHz. With high Soapy input rates,
            // each fexchange0 call produces proportionally fewer output samples.
            audioSamplesThisCall = std::max(1,
                static_cast<int>((static_cast<long long>(BUFFER_SIZE) * 48000LL) / m_soapyInputSampleRate));
        }
#endif

        if (m_smeterTime.elapsed() > 200) {
            m_sMeterValue = qtwdsp->getSMeterInstValue();
            emit sMeterValueChanged(m_receiver, m_sMeterValue);
            m_smeterTime.restart();
        }
#ifdef USE_INTERNAL_AUDIO
        const DSPMode dspMode = m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver);
        bool retuneMuteAudio = false;
#ifdef HAVE_SOAPYSDR
        if (set->getHWInterface() == QSDR::SoapySDR) {
            retuneMuteAudio = m_retuneTimer.isValid() && (m_retuneTimer.elapsed() < m_audioMuteUntilMs);
        }
#endif
        auto deliverInternalAudio = [this, retuneMuteAudio](const QVector<float> &soundcardStereo,
                                                              const QVector<float> &tciStereo) {
            if (retuneMuteAudio)
                return;
            if (m_audioOutput)
                m_audioOutput->writeAudio(soundcardStereo);
            emit rxAudioSamples(m_receiver, tciStereo, 48000);
        };
        if (dspMode != DSPMode::FDV) {
			// Normal analogue modes: pass WDSP audio output straight to soundcard.
            const int n = audioSamplesThisCall;
            deliverInternalAudio(interleaveFromCPX(audioOutputBuf, n),
                                 monoStereoFromCPX(audioOutputBuf, n));
		}
		else {
			QVector<float> mono(audioSamplesThisCall);
			const cpx* src = audioOutputBuf.constData();
			for (int i = 0; i < audioSamplesThisCall; ++i)
				mono[i] = static_cast<float>(src[i].re);

			bool wroteAudio = false;

#ifdef HAVE_CODEC2
#ifdef HAVE_RADE
            if (m_freeDVMode == 100 && m_radeProcessor) {
                QVector<float> speech = m_radeProcessor->processSamples(mono.constData(), audioSamplesThisCall);
                deliverInternalAudio(speech, speech);
                wroteAudio = true;
                if (m_radeProcessor->isSync())
                    m_freeDVRxFrames += 1;

                if ((m_dspCallCount % 50) == 1) {
                    set->setFreeDVStatus(
                        m_receiver,
                        m_radeProcessor->isSync(),
                        m_radeProcessor->getSNR(),
                        m_freeDVRxFrames);
                }
            } else
#endif
            if (m_freeDVProcessor) {
                QVector<float> speech = m_freeDVProcessor->processSamples(mono.constData(), audioSamplesThisCall);
				// processSamples always returns n*2 floats (silence-padded when no
				// frame is ready), so write unconditionally — this prevents the
				// passthrough from adding a second burst of audio.
                deliverInternalAudio(speech, speech);
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
                passthrough.reserve(audioSamplesThisCall * 2);
                for (int i = 0; i < audioSamplesThisCall; ++i) {
					const float s = mono.at(i);
					passthrough.append(s);
					passthrough.append(s);
				}
                deliverInternalAudio(passthrough, passthrough);
			}
		}
#endif // USE_INTERNAL_AUDIO
        // HPSDR: audioBufferSignal feeds the network TX/RX interleave path.
        // Soapy: TX IQ is paced by DataProcessor::m_soapyTxIqTimer (half- and full-duplex).
        if (set->getHWInterface() != QSDR::SoapySDR) {
            emit audioBufferSignal(m_receiver, audioOutputBuf, audioSamplesThisCall);
        }
    }

    // Emit the captured raw IQ last: RX audio has already been queued to the
    // TCI socket above, so audio wins the shared link and panadapter IQ only
    // uses spare capacity (and is dropped under backpressure by the server).
    if (!tciIqFrame.isEmpty())
        emit rxIqSamples(m_receiver, tciIqFrame, m_samplerate);
}

void SliceProcessor::setFreeDVMode(int rx, int mode) {
	#ifdef HAVE_CODEC2
	if (rx != m_receiver) return;
	if (m_freeDVMode == mode) return;

	m_freeDVMode = mode;
	m_freeDVRxFrames = 0;

	if (m_freeDVProcessor) {
		delete m_freeDVProcessor;
		m_freeDVProcessor = nullptr;
	}
#ifdef HAVE_RADE
	if (m_radeProcessor) {
		delete m_radeProcessor;
		m_radeProcessor = nullptr;
	}
#endif

	if (m_freeDVMode == 100) {
#ifdef HAVE_RADE
		m_radeProcessor = new RadeProcessor();
#endif
	} else {
		m_freeDVProcessor = new FreeDVProcessor(m_freeDVMode);
	}
	set->setFreeDVStatus(m_receiver, false, 0.0f, 0);
#else
	Q_UNUSED(rx)
	Q_UNUSED(mode)
#endif
}

QVector<float> SliceProcessor::interleaveFromCPX(const CPX& in, int size) {
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

QVector<float> SliceProcessor::monoStereoFromCPX(const CPX& in, int size) {
    int limit = (size < 0 || size > in.size()) ? in.size() : size;
    QVector<float> out(limit * 2);
    float *outData = out.data();
    const cpx *inData = in.constData();

    for (int i = 0; i < limit; ++i) {
        const float sample = static_cast<float>(inData[i].re);
        *outData++ = sample;
        *outData++ = sample;
    }
    return out;
}

void SliceProcessor::setSampleRate(int value) {
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
			SLICE_PROCESSOR_DEBUG << "invalid sample rate (possible values are: 48, 96, 192, 384, 768, or 1536 kHz)!\n";
			break;
	}

	if (qtwdsp) {
        QMutexLocker dspLocker(&m_dspMutex);

		// Queue flush and drop-counter are now handled unconditionally below.
		const bool highRateTransition = (previousRate >= 768000 || m_samplerate >= 768000);
		(void)highRateTransition;

		setAudioBufferSize();

		// Flush the queue and drop a few buffers after any rate transition so
		// fexchange0 is not called on the channel while it is being rebuilt.
		while (!m_iqQueue.isEmpty())
			m_iqQueue.dequeue();
		while (!m_soapyQueue.isEmpty())
			m_soapyQueue.dequeue();
		m_rateTransitionDropBuffers = HIGH_RATE_TRANSITION_DROP_BUFFERS;

        // Dual-Rate Strategy:
        // We always force 48 kHz DSP for the demodulator/audio path for maximum
        // stability and correct audio pitch. The user-selected high rate is used
        // only for the hardware input and waterfall analyzer.
        const int dspMathRate = 48000;
        qtwdsp->setSampleRate(m_samplerate, dspMathRate);

    }
	else
		SLICE_PROCESSOR_DEBUG << "qtdsp down: cannot set sample rate!\n";
}

void SliceProcessor::setServerMode(QSDR::_ServerMode mode) {

	m_serverMode = mode;
}

QSDR::_ServerMode SliceProcessor::getServerMode()	const {

	return m_serverMode;
}

//void SliceProcessor::setSocketState(SocketState state) {
//
//	m_socketState = state;
//}

//SliceProcessor::SocketState SliceProcessor::socketState() const {
//
//	return m_socketState;
//}

void SliceProcessor::setSystemState(
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

void SliceProcessor::setAudioMode(int mode) {

	if (m_audioMode == mode) return;

	m_audioMode = mode;
}

//void SliceProcessor::setID(int value) {
//
//	m_receiverID = value;
//	SLICE_PROCESSOR_DEBUG << "This is receiver " << m_receiverID;
//}

void SliceProcessor::setReceiver(int value) {

	m_receiver = value;
}

void SliceProcessor::setFramesPerSecond(int rx, int value) {

	if (m_receiver == rx)
		m_displayTime = (int)(1000000.0/value);
}

void SliceProcessor::setPeerAddress(QHostAddress addr) {

	m_peerAddress = addr;
}

void SliceProcessor::setSocketDescriptor(int value) {

	m_socketDescriptor = value;
}

void SliceProcessor::setClient(int value) {

	m_client = value;
}

void SliceProcessor::setIQPort(int value) {

	m_iqPort = value;
}

void SliceProcessor::setBSPort(int value) {

	m_bsPort = value;
}

void SliceProcessor::setConnectedStatus(bool value) {

	m_connected = value;
}
