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
#include "DataEngine/ISdrDevice.h"
#include "QtWDSP/WdspTxChannel.h"
#include "Util/cusdr_tciserver.h"
#include <cmath>

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
    , m_dspPending(false)
    , m_rateTransitionDropBuffers(0)
	//, m_calOffset(63.0)
	//, m_calOffset(33.0)
{
	InitCPX(inBuf, BUFFER_SIZE, 0.0f);
	InitCPX(outBuf, BUFFER_SIZE, 0.0f);
    InitCPX(audioOutputBuf, BUFFER_SIZE, 0.0f);
    m_soundcardScratch.resize(BUFFER_SIZE * 2, 0.0f);
    m_tciAudioScratch.resize(BUFFER_SIZE * 2, 0.0f);
    m_monoScratch.resize(BUFFER_SIZE, 0.0f);
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
		m_dvDemodulator = std::make_unique<RadeProcessor>();
#endif
	} else {
		m_dvDemodulator = std::make_unique<FreeDVProcessor>(m_freeDVMode);
	}
#endif
	m_cwDecoder = new CwDecoder(m_receiver, this);
	m_cwDecoder->setPitch(set->getCwSidetoneFreq());
	if (m_sliceModel) {
		connect(m_cwDecoder, &CwDecoder::textUpdated,
				m_sliceModel, [this](int rx, const QString &text) {
			if (m_sliceModel && m_sliceModel->id() == rx)
				m_sliceModel->setCwDecodedText(text);
		});
		connect(m_cwDecoder, &CwDecoder::wpmChanged,
				m_sliceModel, [this](int rx, int wpm) {
			if (m_sliceModel && m_sliceModel->id() == rx)
				m_sliceModel->setCwWpm(wpm);
		});
		connect(m_cwDecoder, &CwDecoder::toneStatusChanged,
				m_sliceModel, [this](int rx, bool active, float snrDb) {
			Q_UNUSED(snrDb)
			if (m_sliceModel && m_sliceModel->id() == rx)
				m_sliceModel->setCwToneActive(active);
		});
		connect(m_cwDecoder, &CwDecoder::trackedPitchChanged,
				m_sliceModel, [this](int rx, int pitch) {
			if (m_sliceModel && m_sliceModel->id() == rx)
				m_sliceModel->setCwTrackedPitch(pitch);
		});
		connect(m_sliceModel, &SliceModel::cwDecodedTextChanged,
				this, [this](const QString &text) {
			if (text.isEmpty() && m_cwDecoder)
				m_cwDecoder->clearText();
		});
		connect(m_sliceModel, &SliceModel::cwDecodeEnabledChanged,
				this, [this](bool enabled) {
			if (m_cwDecoder) {
				m_cwDecoder->setEnabled(enabled);
				if (!enabled)
					m_cwDecoder->clearText();
			}
		});
	}
	connect(set, &Settings::CwSidetoneFreqChanged, m_cwDecoder, &CwDecoder::setPitch);

	setupConnections();
    m_displayTime = (int)(1000000.0/set->getFramesPerSecond(m_receiver));
	m_smeterTime.start();
    m_retuneTimer.start();
}

SliceProcessor::~SliceProcessor() {
    qDebug() << "Destroy SliceProcessor " << m_receiver;
    if (set && set->tciServer()) {
        set->tciServer()->setRxAudioRing(m_receiver, nullptr);
    }
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

    connect(set, &Settings::soapyAutoCalibrateChanged,
            this, &SliceProcessor::resetSoapyDcEstimator);

    if (m_sliceModel && set->getHWInterface() == QSDR::SoapySDR) {
        connect(m_sliceModel, &SliceModel::frequencyChanged,
                this, &SliceProcessor::noteRetuneActivity);
        connect(m_sliceModel, &SliceModel::centerFrequencyChanged,
                this, &SliceProcessor::noteRetuneActivity);
    }
}

void SliceProcessor::resetSoapyDcEstimator()
{
    m_soapyDcAvgI = 0.0;
    m_soapyDcAvgQ = 0.0;
}

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

    // Channel already opened dual-rate (pan → 48 kHz DSP). No-op if rates match.
    const DSPMode mode = m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver);
    qtwdsp->setSampleRate(m_samplerate, QWDSPEngine::preferredDspRate(mode, m_samplerate));
    setAudioBufferSize();

    qtwdsp->setQtDSPStatus(true);
    const float volume = m_sliceModel ? m_sliceModel->volume() : static_cast<float>(set->getMainVolume(m_receiver));
    qtwdsp->setVolume(volume);

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

void SliceProcessor::enqueueRxIq(const float* interleavedIq, int numComplexSamples) {
    if (!interleavedIq || numComplexSamples <= 0) return;

    m_rxRing.writeDropOldest(interleavedIq, static_cast<size_t>(numComplexSamples * 2));

    if (!m_queueDropLogTimer.isValid())
        m_queueDropLogTimer.start();
    if (m_queueDropLogTimer.elapsed() >= 5000) {
        uint64_t drops = m_rxRing.dropCount();
        if (drops > 0) {
            SLICE_PROCESSOR_DEBUG << "[RX" << m_receiver << "] ring drops in last 5s: " << drops;
            m_rxRing.resetDropCount();
        }
        m_queueDropLogTimer.restart();
    }

    if (trySetDspPending()) {
        QMetaObject::invokeMethod(this, "dspProcessing", Qt::QueuedConnection);
    }
}

void SliceProcessor::enqueueRxIq(const QVector<float> &samples) {
    if (samples.isEmpty()) return;
    enqueueRxIq(samples.constData(), samples.size() / 2);
}

int SliceProcessor::readFromDevice(ISdrDevice* dev, int maxSamples) {
    if (!dev || maxSamples <= 0) return 0;
    constexpr int kMaxStackSamples = 2048;
    if (maxSamples <= kMaxStackSamples) {
        float stackBuf[kMaxStackSamples * 2];
        int read = dev->readRxIq(m_receiver, stackBuf, maxSamples);
        if (read > 0) {
            enqueueRxIq(stackBuf, read);
        }
        return read;
    }
    std::vector<float> buf(maxSamples * 2);
    int read = dev->readRxIq(m_receiver, buf.data(), maxSamples);
    if (read > 0) {
        enqueueRxIq(buf.data(), read);
    }
    return read;
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
}

void SliceProcessor::stop() {

	m_mutex.lock();
	m_stopped = true;
	m_mutex.unlock();
	m_rxRing.clear();
	m_tciAudioRing.clear();
}

void SliceProcessor::stopAudio()
{
#ifdef USE_INTERNAL_AUDIO
	// Call only after DSP threads have stopped writing.
	if (m_audioOutput)
		m_audioOutput->stop();
#endif
}

void SliceProcessor::dspProcessing() {
    constexpr size_t kBlockFloats = BUFFER_SIZE * 2;
    float blockBuf[kBlockFloats];

    while (m_rxRing.availableRead() >= kBlockFloats) {
        m_rxRing.read(blockBuf, kBlockFloats);

        {
            QMutexLocker locker(&m_mutex);
            if (m_rateTransitionDropBuffers > 0) {
                --m_rateTransitionDropBuffers;
                continue;
            }
        }

        ++m_dspCallCount;

        cpx* inPtr = inBuf.data();
        const float* rawPtr = blockBuf;
        const bool isSoapy = (set && set->getHWInterface() == QSDR::SoapySDR);
        const bool soapyDcRemove = (isSoapy && set && set->getSoapyAutoCalibrate());
        const bool negateQ = isSoapy;

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

    m_dspPending.store(false, std::memory_order_release);

    if (m_rxRing.availableRead() >= kBlockFloats && trySetDspPending()) {
        QMetaObject::invokeMethod(this, "dspProcessing", Qt::QueuedConnection);
    }
}

void SliceProcessor::dspProcessingCore() {
    {
        QMutexLocker locker(&m_mutex);
        if (m_stopped)
            return;
    }

    // 1. TCI raw IQ tap before WDSP consumes inBuf
    QVector<float> tciIqFrame;
    if (set->tciIqActive())
        tciIqFrame = interleaveFromCPX(inBuf);

    // 2. WDSP channel execution
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
        m_dspTimeAccum = 0.0;
        m_dspTimeMin = 1e9;
        m_dspTimeMax = 0.0;
    }

    // 3. Spectrum & panadapter pass
    const bool transmitting = set->is_transmitting() || (m_state != RadioState::RX);
    processSpectrumPass(transmitting);

    // 4. S-meter and audio pass (for active receiver)
    if (m_receiver == set->getCurrentReceiver()) {
        processMeterPass();

        int audioSamplesThisCall = m_audiobuffersize;
        if (set->getHWInterface() == QSDR::SoapySDR && m_soapyInputSampleRate > 0) {
            audioSamplesThisCall = std::max(1,
                static_cast<int>((static_cast<long long>(BUFFER_SIZE) * 48000LL) / m_soapyInputSampleRate));
        }

        processAudioPass(audioSamplesThisCall);
    }

    // 5. Emit captured raw IQ for TCI
    if (!tciIqFrame.isEmpty())
        emit rxIqSamples(m_receiver, tciIqFrame, m_samplerate);
}

void SliceProcessor::processSpectrumPass(bool transmitting) {
    if (highResTimer->getElapsedTimeInMicroSec() < getDisplayDelay())
        return;

    int spectrumDataReady = 0;
    bool txPixelsRequested = false;

    if (transmitting) {
        if (set->getHWInterface() == QSDR::SoapySDR && !set->getTxFullDuplex()) {
            spectrumDataReady = 0;
        } else {
            txPixelsRequested = true;
            WdspTxChannel::getSpectrumPixels(TX_ID, qtwdsp->spectrumBuffer.data(), spectrumDataReady);
            if (spectrumDataReady) {
                prepareTxPanadapterSpectrum(qtwdsp->spectrumBuffer, m_samplerate);
                m_lastTxSpectrum = qtwdsp->spectrumBuffer;
                m_haveLastTxSpectrum = true;
            } else if (m_haveLastTxSpectrum) {
                qtwdsp->spectrumBuffer = m_lastTxSpectrum;
                spectrumDataReady = 1;
            }
        }
    } else {
        m_haveLastTxSpectrum = false;
        qtwdsp->getSpectrumPixels(qtwdsp->spectrumBuffer.data(), spectrumDataReady);
    }

    if (spectrumDataReady) {
        newSpectrum = qtwdsp->spectrumBuffer;
        emit spectrumBufferChanged(m_receiver, newSpectrum);
    }

    static const bool txPanDiagEnabled = (qEnvironmentVariableIntValue("CUSDR_TX_DIAG") != 0);
    if (txPanDiagEnabled && txPixelsRequested && (m_dspCallCount % 100) == 1) {
        qDebug().nospace() << "[TX-PAN-DIAG] rx=" << m_receiver
                           << " mode=" << (m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver))
                           << " state=" << m_state.load()
                           << " txPixels=" << (spectrumDataReady ? "yes" : "no")
                           << " held=" << m_haveLastTxSpectrum;
    }
    highResTimer->start();
}

void SliceProcessor::processMeterPass() {
    if (m_smeterTime.elapsed() < 25)
        return;

    m_sMeterValue = qtwdsp->getSMeterInstValue();
    m_sMeterPeakValue = qtwdsp->getSMeterPeakValue();
    if (m_sliceModel) {
        m_sliceModel->setSMeterValue(m_sMeterValue);
        m_sliceModel->setSMeterPeakValue(m_sMeterPeakValue);
    }
    emit sMeterValueChanged(m_receiver, m_sMeterValue);
    emit sMeterPeakValueChanged(m_receiver, m_sMeterPeakValue);
    m_smeterTime.restart();
}

bool SliceProcessor::isRetuneMuted() const {
    if (set->getHWInterface() == QSDR::SoapySDR) {
        return m_retuneTimer.isValid() && (m_retuneTimer.elapsed() < m_audioMuteUntilMs);
    }
    return false;
}

void SliceProcessor::deliverInternalAudio(const float *soundcardStereo, int soundcardCount,
                                          const float *tciStereo, int tciCount) {
#ifdef USE_INTERNAL_AUDIO
    if (isRetuneMuted())
        return;
    if (m_audioOutput && soundcardStereo && soundcardCount > 0)
        m_audioOutput->writeAudio(soundcardStereo, soundcardCount);
    if (tciStereo && tciCount > 0) {
        m_tciAudioRing.writeDropOldest(tciStereo, static_cast<size_t>(tciCount));
        emit tciAudioReady(m_receiver);
        if (isSignalConnected(QMetaMethod::fromSignal(&SliceProcessor::rxAudioSamples))) {
            QVector<float> legacyVec(tciCount);
            std::memcpy(legacyVec.data(), tciStereo, tciCount * sizeof(float));
            emit rxAudioSamples(m_receiver, std::move(legacyVec), 48000);
        }
    }
#else
    Q_UNUSED(soundcardStereo)
    Q_UNUSED(soundcardCount)
    Q_UNUSED(tciStereo)
    Q_UNUSED(tciCount)
#endif
}

void SliceProcessor::synthesizeCwSidetone(int n) {
    const int vol = set->getCwSidetoneVolume();
    const double freqHz = static_cast<double>(set->getCwSidetoneFreq());
    const double phaseInc = 2.0 * M_PI * freqHz / 48000.0;
    const double gain = vol / 127.0;
    cpx* buf = audioOutputBuf.data();
    for (int i = 0; i < n; ++i) {
        const bool keyActive = (m_cwKeyActive.load() != 0);
        if (keyActive) {
            if (m_sidetoneShape < 250) ++m_sidetoneShape;
        } else if (m_sidetoneShape > 0) {
            --m_sidetoneShape;
        }

        int hold = m_cwMuteHold.load();
        if (hold > 0) m_cwMuteHold.store(hold - 1);
        const bool rxMuted = (hold > 0);

        if (rxMuted) {
            const double ramp = m_sidetoneShape / 250.0;
            const double s = (vol > 0 && m_sidetoneShape > 0) ? gain * ramp * std::sin(m_sidetonePhase) : 0.0;
            buf[i].re = static_cast<float>(s);
            buf[i].im = static_cast<float>(s);
        }
        m_sidetonePhase += phaseInc;
        if (m_sidetonePhase >= 2.0 * M_PI) m_sidetonePhase -= 2.0 * M_PI;
    }
}

void SliceProcessor::processDigitalVoicePass(const float* monoIn, int count) {
    bool wroteAudio = false;

#ifdef HAVE_CODEC2
    if (m_dvDemodulator) {
        QVector<float> speech = m_dvDemodulator->processSamples(monoIn, count);
        if (!speech.isEmpty()) {
            deliverInternalAudio(speech.constData(), speech.size(),
                                 speech.constData(), speech.size());
            wroteAudio = true;
        }
        if (m_dvDemodulator->isSync())
            m_freeDVRxFrames += 1;

        if ((m_dspCallCount % 50) == 1) {
            set->setFreeDVStatus(
                m_receiver,
                m_dvDemodulator->isSync(),
                m_dvDemodulator->getSNR(),
                m_freeDVRxFrames);
        }
    }
#endif

    if (!wroteAudio) {
        if (m_soundcardScratch.size() < static_cast<size_t>(count * 2))
            m_soundcardScratch.resize(count * 2);
        float* p = m_soundcardScratch.data();
        for (int i = 0; i < count; ++i) {
            const float s = monoIn[i];
            *p++ = s;
            *p++ = s;
        }
        deliverInternalAudio(m_soundcardScratch.data(), count * 2,
                             m_soundcardScratch.data(), count * 2);
    }
}

void SliceProcessor::processAudioPass(int audioSamplesThisCall) {
#ifdef USE_INTERNAL_AUDIO
    const DSPMode dspMode = m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver);

    if (dspMode != DSPMode::FDV) {
        const int n = audioSamplesThisCall;

        if ((dspMode == DSPMode::CWU || dspMode == DSPMode::CWL) && !set->isInternalCw()) {
            synthesizeCwSidetone(n);
        }

        if (m_soundcardScratch.size() < static_cast<size_t>(n * 2))
            m_soundcardScratch.resize(n * 2);
        if (m_tciAudioScratch.size() < static_cast<size_t>(n * 2))
            m_tciAudioScratch.resize(n * 2);

        const cpx* inData = audioOutputBuf.constData();
        float* scOut = m_soundcardScratch.data();
        float* tciOut = m_tciAudioScratch.data();
        for (int i = 0; i < n; ++i) {
            const float re = static_cast<float>(inData[i].re);
            const float im = static_cast<float>(inData[i].im);
            *scOut++ = re;
            *scOut++ = im;
            *tciOut++ = re;
            *tciOut++ = re;
        }

        deliverInternalAudio(m_soundcardScratch.data(), n * 2,
                             m_tciAudioScratch.data(), n * 2);

        if (m_cwDecoder && m_cwDecoder->isEnabled() && (dspMode == DSPMode::CWL || dspMode == DSPMode::CWU)) {
            if (m_monoScratch.size() < static_cast<size_t>(n))
                m_monoScratch.resize(n);
            float* mono = m_monoScratch.data();
            for (int i = 0; i < n; ++i)
                mono[i] = static_cast<float>(inData[i].re);
            m_cwDecoder->processAudio(mono, n, 48000);
        }
    } else {
        if (m_monoScratch.size() < static_cast<size_t>(audioSamplesThisCall))
            m_monoScratch.resize(audioSamplesThisCall);
        float* mono = m_monoScratch.data();
        const cpx* src = audioOutputBuf.constData();
        for (int i = 0; i < audioSamplesThisCall; ++i)
            mono[i] = static_cast<float>(src[i].re);

        processDigitalVoicePass(mono, audioSamplesThisCall);
    }
#endif // USE_INTERNAL_AUDIO

    if (set->getHWInterface() != QSDR::SoapySDR) {
        emit audioBufferSignal(m_receiver, audioOutputBuf, audioSamplesThisCall);
    }
}

void SliceProcessor::setFreeDVMode(int rx, int mode) {
#ifdef HAVE_CODEC2
	if (rx != m_receiver) return;
	if (m_freeDVMode == mode && m_dvDemodulator) return;

	m_freeDVMode = mode;
	m_freeDVRxFrames = 0;
	m_dvDemodulator.reset();

	if (m_freeDVMode == 100) {
#ifdef HAVE_RADE
		m_dvDemodulator = std::make_unique<RadeProcessor>();
#endif
	} else {
		m_dvDemodulator = std::make_unique<FreeDVProcessor>(m_freeDVMode);
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

		// Flush the ring buffer and drop a few buffers after any rate transition so
		// fexchange0 is not called on the channel while it is being rebuilt.
		m_rxRing.clear();
		m_rateTransitionDropBuffers = HIGH_RATE_TRANSITION_DROP_BUFFERS;

        // Dual-rate: HB rsmpin → 48 kHz DSP demod rate.
        const DSPMode mode = m_sliceModel ? m_sliceModel->dspMode() : set->getDSPMode(m_receiver);
        qtwdsp->setSampleRate(m_samplerate, QWDSPEngine::preferredDspRate(mode, m_samplerate));

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

void SliceProcessor::cwKeyDown(int state)
{
    m_cwKeyActive.store(state ? 1 : 0);
    if (state) {
        // Refresh the mute-hold countdown on every key-down.
        // 2 * word-space at 5 wpm = 2 * 7 * 240ms = 3360ms = ~161280 samples.
        // Use a generous 96000 (2 sec) — the next key-down refreshes it before expiry.
        m_cwMuteHold.store(96000);
    }
    // On key-up we leave m_cwMuteHold running so RX stays muted through inter-element
    // gaps. It expires naturally if no new key-down arrives within ~2 seconds.
}
