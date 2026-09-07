//
// Created by Simon Eatough ZL2BRG on 5/08/21.

//

/* Copyright (C)
*
* Simon Eatough zl2brg
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
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

#define ORIGINAL_PROTOCOL 0
#define NEW_PROTOCOL 1

#include "cusdr_transmitter.h"
#include "Models/RadioModel.h"
#include "Util/settings_utils.h"

namespace {
double micSliderToPanelGain(const double sliderValue)
{
    // UI/settings store mic level as 0..100. Map 50 -> unity gain.
    const double clamped = qBound(0.0, sliderValue, 100.0);
    return clamped / 50.0;
}

int compressionSliderToDb(const int sliderValue)
{
    return qBound(0, sliderValue, 20);
}

double amCarrierPercentToLevel(int percent)
{
    return qBound(0, percent, 100) / 100.0;
}
}

Transmitter::Transmitter(int transmitter)
    : QObject()
    , set(Settings::instance())
    , m_txModel(set->radioModel() ? set->radioModel()->transmit() : nullptr)
    , m_asteps(0)
    , m_bsteps(0)
    , mode(set ? set->getDSPMode(set->getCurrentReceiver()) : USB)
{
    Q_UNUSED(transmitter)
    m_phrotStatusTimer = new QTimer(this);
    m_phrotStatusTimer->setInterval(500);
    connect(m_phrotStatusTimer, &QTimer::timeout, this, &Transmitter::updatePhaseRotatorStatus);
    // fft_size feeds TXASetNC; keep 2048 so create_fircore does not rebuild
    // every TX bandpass at 4096 taps (and optional min-phase plans of 16384).
    create_transmitter(TX_ID, DSP_SAMPLE_SIZE, 2048, 10, 2048, 100);
    setupConnections();
}

Transmitter::~Transmitter() {
    if (m_channel) {
        m_channel->close();
    }
}

void Transmitter::process(const double *audioIn, double *iqOut, int &error)
{
    if (m_channel) {
        m_channel->process(audioIn, iqOut, error);
    } else {
        error = -1;
    }
}

void Transmitter::pushSpectrum(const double *iqData)
{
    if (m_channel) {
        m_channel->pushSpectrum(iqData);
    }
}

bool Transmitter::getSpectrumPixels(float *pixels, int &ready)
{
    if (m_channel) {
        return m_channel->getSpectrumPixels(pixels, ready);
    }
    ready = 0;
    return false;
}

void Transmitter::stopChannel()
{
    if (m_channel) {
        m_channel->stopChannel();
    }
}



// Sine tone generator:
// somewhat improved, and provided two siblings
// for generating side tones simultaneously on the
// HPSDR board and local audio.

constexpr double TWOPIOVERSAMPLERATE = 0.0001308996938995747;  // 2 Pi / 48000

double Transmitter::getNextSideToneSample() {
    double angle = (m_asteps * cw_keyer_sidetone_frequency) * TWOPIOVERSAMPLERATE;
    if (++m_asteps == 48000) m_asteps = 0;
    return sin(angle);
}

double Transmitter::getNextInternalSideToneSample() {
    double angle = (m_bsteps * cw_keyer_sidetone_frequency) * TWOPIOVERSAMPLERATE;
    if (++m_bsteps == 48000) m_bsteps = 0;
    return sin(angle);
}



void Transmitter::setupConnections() {
    connect(set, &Settings::dspModeChanged,
            this, &Transmitter::setDSPMode);

    connect(set, &Settings::currentReceiverChanged,
            this, [this](int) { programTxa(); });

    connect(set, &Settings::radioStateChanged,
            this, &Transmitter::setRadioState);

    // Mic gain still lives on Settings (main-window slider).
    connect(set, &Settings::micInputLevelChanged,
            this, &Transmitter::transmitter_set_mic_level);

    if (!m_txModel)
        return;

    connect(m_txModel, &TransmitModel::fmDeviationChanged,
            this, [this](int hz) { set_fm_deviation(static_cast<double>(hz)); });

    connect(m_txModel, &TransmitModel::fmPreEmphasisChanged,
            this, [this](bool) { applyFmPreEmphasis(); });

    connect(m_txModel, &TransmitModel::phaseRotatorChanged,
            this, [this](bool) { applyPhaseRotator(); });

    connect(m_txModel, &TransmitModel::phaseRotatorAutoChanged,
            this, [this](bool) { applyPhaseRotator(); });

    connect(m_txModel, &TransmitModel::phaseRotatorAutoResetRequested,
            this, [this]() { if (m_channel) m_channel->resetPhaseRotatorAuto(); });

    connect(m_txModel, &TransmitModel::txEqChanged,
            this, [this]() { applyTxEq(); });

    connect(m_txModel, &TransmitModel::cfcChanged,
            this, [this]() { applyCfc(); });

    connect(m_txModel, &TransmitModel::ctcssToneHzChanged,
            this, [this](int) { applyCtcss(); });

    connect(m_txModel, &TransmitModel::amCarrierLevelChanged,
            this, [this](int percent) {
                transmitter_set_am_carrier_level(amCarrierPercentToLevel(percent));
            });

    connect(m_txModel, &TransmitModel::audioCompressionChanged,
            this, &Transmitter::transmitter_set_audio_compression);

    connect(m_txModel, &TransmitModel::txFilterLowChanged,
            this, [this](int) { applyTxFilter(); });

    connect(m_txModel, &TransmitModel::txFilterHighChanged,
            this, [this](int) { applyTxFilter(); });

    connect(m_txModel, &TransmitModel::txUseRxFilterChanged,
            this, [this](bool) { applyTxFilter(); });

    if (set) {
        connect(set, &Settings::filterFrequenciesChanged,
                this, [this](int rx, qreal, qreal) {
                    if (rx == set->getCurrentReceiver()) {
                        const bool useRx = m_txModel ? m_txModel->txUseRxFilter() : set->getTxUseRxFilter();
                        if (useRx) applyTxFilter();
                    }
                });
    }
}



bool  Transmitter::create_transmitter(int id, int buffer_size, int fft_size, int fps, int width, int height) {

    int protocol = ORIGINAL_PROTOCOL;
    this->id = id;
    this->dac=0;
    this->buffer_size=buffer_size;
    this->fft_size=fft_size;
    this->fps=fps;

    switch(protocol) {
        case ORIGINAL_PROTOCOL:
            this->mic_sample_rate=48000;
            this->mic_dsp_rate=48000;
            this->iq_output_rate=48000;
            this->output_samples=this->buffer_size;
            this->pixels=width; // to allow 48k to 24k conversion
            break;
            case NEW_PROTOCOL:
                this->mic_sample_rate=48000;
                this->mic_dsp_rate=96000;
                this->iq_output_rate=192000;
                this->output_samples=this->buffer_size*4;
                this->pixels=width*4; // to allow 192k to 24k conversion
                break;
#ifdef SOAPYSDR
                case SOAPYSDR_PROTOCOL:
                    this->mic_sample_rate=48000;
                    this->mic_dsp_rate=96000;
                    this->iq_output_rate=radio_sample_rate;
                    this->buffer_size=1024;
                    this->output_samples=1024*(this->iq_output_rate/this->mic_sample_rate);
                    this->pixels=width*8; // to allow 384k to 24k conversion
                    break;
#endif
    }
    this->width=width;
    this->height=height;
    this->display_panadapter=1;
    this->display_waterfall=0;

    this->panadapter_high=0;
    this->panadapter_low=-60;

    this->displaying=0;

    this->alex_antenna=0;

    TRANSMITTER_DEBUG << "create_transmitter: id=" << id << " buffer_size=" << buffer_size << " mic_sample_rate=" << mic_sample_rate << " mic_dsp_rate=" << mic_dsp_rate << " iq_output_rate=" << iq_output_rate << " output_samples=" << output_samples << " fps=" << fps;

    this->filter_low = 100;
    this->filter_high = 2900;

    this->out_of_band=0;

    this->low_latency=0;

    this->twotone=0;
    this->puresignal=0;
    this->feedback=0;
    this->auto_on=0;
    this->single_on=0;

    this->attenuation=0;
    this->ctcss=0;
    this->ctcss_frequency=100.0;

    this->deviation=2500;
    this->am_carrier_level=0.5;


#ifdef FREEDV
    strcpy(this->freedv_text_data,"Call, Name and Location");
    this->freedv_samples=0;
#endif

    this->drive=set->get_tx_drivelevel();
    this->tune_percent= 10;
    this->tune_use_drive=0;

    this->compressor=0;
    this->compressor_level = m_txModel ? m_txModel->audioCompression()
                                       : set->getAudioCompression();

    this->local_microphone=0;




    // allocate buffers

    this->samples=0;
  //if w_shape_buffer48) free(cw_shape_buffer48);
  //if (cw_shape_buffer192) free(cw_shape_buffer192);
    //
    // We need this one both for old and new protocol, since
    // is is also used to shape the audio samples
    if (protocol == NEW_PROTOCOL) {
        // We need this buffer for the new protocol only, where it is only
        // used to shape the TX envelope
    }
    TRANSMITTER_DEBUG << "transmitter: buffers allocated";

    TRANSMITTER_DEBUG << "create_transmitter: OpenChannel id=" << id << " buffer_size=" << buffer_size << " fft_size=" << fft_size << " sample_rate=" << mic_sample_rate << " dspRate=" << mic_dsp_rate << " outputRate=" << iq_output_rate;

    m_channel = std::make_unique<WdspTxChannel>(this->id);
    if (!m_channel->open(this->buffer_size, this->fft_size, this->mic_sample_rate, this->mic_dsp_rate, this->iq_output_rate, protocol, this->low_latency != 0)) {
        qWarning() << "Failed to open WdspTxChannel for transmitter" << this->id;
        return false;
    }

    applyPhaseRotator();
    applyTxEq();
    applyCfc();
    m_channelCreated = true;
    programTxa();

    applyCtcss();

    const double initialMicLevel = set->getMicInputLevel();
    mic_gain = initialMicLevel;
    transmitter_set_mic_level(initialMicLevel);

    const int fmDev = m_txModel ? m_txModel->fmDeviation()
                                : static_cast<int>(set->getFMDeveation());
    set_fm_deviation(fmDev);
    const int amPercent = m_txModel ? m_txModel->amCarrierLevel()
                                    : qRound(set->getAMCarrierLevel() <= 1.0
                                                 ? set->getAMCarrierLevel() * 100.0
                                                 : set->getAMCarrierLevel());
    transmitter_set_am_carrier_level(amCarrierPercentToLevel(amPercent));
    transmitter_set_audio_compression(m_txModel ? m_txModel->audioCompression()
                                                : static_cast<int>(set->getAudioCompression()));

    return true;
}

void Transmitter::setDSPMode(int id, DSPMode dspMode) {
    Q_UNUSED(id)
    mode = dspMode;
    TRANSMITTER_DEBUG << "[TX] DSP mode set to" << dspMode;
    programTxa();
}





void Transmitter::set_fm_deviation(double level) {
    if (m_channel) {
        m_channel->setFmDeviation(level);
    }
    TRANSMITTER_DEBUG << "Set Tx FM deveation " << level;
}

void Transmitter::applyFmPreEmphasis()
{
    // WDSP SetTXAMode(FM) forces preemph.run=1; re-apply user preference after mode changes.
    const bool enabled = m_txModel ? m_txModel->fmPreEmphasis()
                                   : (set->getFMpreemphesis() != 0.0);
    const int run = enabled ? 1 : 0;
    if (m_channel) {
        m_channel->setFmPreEmphasis(0, enabled);
    }
    TRANSMITTER_DEBUG << "FM pre-emphasis " << (run ? "on" : "off");
}

void Transmitter::applyPhaseRotator()
{
    const bool enabled = m_txModel ? m_txModel->phaseRotator()
                                   : (set->getPhaseRotator() != 0);
    const bool autoOn = m_txModel ? m_txModel->phaseRotatorAuto()
                                  : set->getPhaseRotatorAuto();
    const int run = enabled ? 1 : 0;
    const int autoMode = (run && autoOn) ? 1 : 0;
    if (m_channel) {
        m_channel->setPhaseRotator(enabled, autoOn);
    }
    syncPhaseRotatorTimer();
    TRANSMITTER_DEBUG << "Audio Phase Rotator " << (run ? "on" : "off")
                      << " auto=" << (autoMode ? "on" : "off");
}

void Transmitter::applyTxEq()
{
    const bool enabled = m_txModel ? m_txModel->txEqEnabled() : set->getTxEqEnabled();
    const int curveDeg = m_txModel ? m_txModel->txEqCurveDeg() : set->getTxEqCurveDeg();
    if (m_channel) {
        const QVector<int> bands = m_txModel ? m_txModel->txEqBands() : set->getTxEqBands();
        m_channel->setTxEq(bands, curveDeg, enabled);
    }
    TRANSMITTER_DEBUG << "TX EQ " << (enabled ? "on" : "off")
                      << " curveDeg=" << curveDeg;
}

void Transmitter::applyCfc()
{
    const bool run = m_txModel ? m_txModel->cfcEnabled() : set->getCfcEnabled();
    const bool peq = m_txModel ? m_txModel->cfcPeqEnabled() : set->getCfcPeqEnabled();
    const QVector<double> freqs = set->getCfcFreqs(); // frequencies stay on Settings
    const QVector<double> levels = m_txModel ? m_txModel->cfcLevels() : set->getCfcLevels();
    const QVector<double> post = m_txModel ? m_txModel->cfcPost() : set->getCfcPost();
    const double precomp = m_txModel ? m_txModel->cfcPrecomp() : set->getCfcPrecomp();
    const double prePeq = m_txModel ? m_txModel->cfcPrePeq() : set->getCfcPrePeq();
    const int deg = m_txModel ? m_txModel->cfcCurveDeg() : set->getCfcCurveDeg();
    if (m_channel) {
        m_channel->setCfc(run, peq, freqs, levels, post, precomp, prePeq, deg);
    }
    TRANSMITTER_DEBUG << "TX CFC run=" << (run || peq) << " peq=" << peq
                      << " precomp=" << precomp
                      << " prepeq=" << prePeq
                      << " curveDeg=" << deg;
}

void Transmitter::syncPhaseRotatorTimer()
{
    if (!m_phrotStatusTimer)
        return;
    const bool run = m_txModel ? m_txModel->phaseRotator()
                               : (set->getPhaseRotator() != 0);
    const bool autoMode = m_txModel ? m_txModel->phaseRotatorAuto()
                                    : set->getPhaseRotatorAuto();
    if (run && autoMode)
        m_phrotStatusTimer->start();
    else {
        m_phrotStatusTimer->stop();
        if (!run) {
            if (m_txModel)
                m_txModel->setPhaseRotatorStatus(QString());
            else
                set->setPhaseRotatorStatus(QString());
        }
    }
}

void Transmitter::updatePhaseRotatorStatus()
{
    const bool run = m_txModel ? m_txModel->phaseRotator()
                               : (set->getPhaseRotator() != 0);
    const bool autoMode = m_txModel ? m_txModel->phaseRotatorAuto()
                                    : set->getPhaseRotatorAuto();
    if (!run || !autoMode || !m_channel)
        return;
    double in_pos = 0, in_neg = 0, in_ratio = 0;
    double out_pos = 0, out_neg = 0, out_ratio = 0;
    double current_fc = 0, auto_step = 0;
    if (!m_channel->getPhaseRotatorAsymmetry(&in_pos, &in_neg, &in_ratio,
                                            &out_pos, &out_neg, &out_ratio,
                                            &current_fc, &auto_step)) {
        return;
    }
    const QString status =
        QStringLiteral("Asym %1 → %2  fc %3 Hz")
            .arg(in_ratio, 0, 'f', 2)
            .arg(out_ratio, 0, 'f', 2)
            .arg(current_fc, 0, 'f', 0);
    if (m_txModel)
        m_txModel->setPhaseRotatorStatus(status);
    else
        set->setPhaseRotatorStatus(status);
}

void Transmitter::applyCtcss()
{
    const int hz = m_txModel ? m_txModel->ctcssToneHz() : set->getCtcssToneHz();
    this->ctcss_frequency = static_cast<double>(hz);
    this->ctcss = (hz > 0) ? 1 : 0;
    if (m_channel) {
        m_channel->setCtcss(this->ctcss_frequency, this->ctcss != 0);
    }
    TRANSMITTER_DEBUG << "CTCSS" << (this->ctcss ? "on" : "off") << "freq" << this->ctcss_frequency;
}

void Transmitter::setRadioState(RadioState state)
{
    switch(state) {

    case RadioState::MOX: {
        if (m_channel) {
            m_channel->setPostGen(0, 0.0, 0.0, false);
            programTxa();
            applyPhaseRotator();
            m_channel->setMicGain(micSliderToPanelGain(mic_gain));
            m_channel->setBandpassWindow(1);
            m_channel->setTxRun(true);
        }
        TRANSMITTER_DEBUG << "MOX: TX channel started with mode" << this->mode;
        break;
    }

    case RadioState::TUNE: {
        // Tone generator for TUNE
        if (m_channel) {
            m_channel->setPostGen(0, 1000.0, 0.5, true);
            programTxa();
            applyPhaseRotator();
            m_channel->setMicGain(micSliderToPanelGain(mic_gain));
            m_channel->setBandpassWindow(1);
            m_channel->setTxRun(true);
        }
        TRANSMITTER_DEBUG << "TUNE: TX channel started with tone, mode" << this->mode;
        break;
    }

    case RadioState::RX:
    default:
        if (m_channel) {
            m_channel->setPostGen(0, 0.0, 0.0, false);
            m_channel->setTxRun(false);
        }
        WdspChannel::setChannelStateById(0, 1, 1);
        TRANSMITTER_DEBUG << "RX: TX channel stopped";
        break;
    }
}


void Transmitter::programTxa() {
    if (!m_channelCreated)
        return;
    const int rx = set ? set->getCurrentReceiver() : 0;
    if (set) this->mode = set->getDSPMode(rx);
    const DSPMode wdspMode = resolveWDSPMode(this->mode, set ? set->getCtrFrequency(rx) : 0);
    // Mode first (may rebuild with stale Hz), then freqs so SetTXABandpassFreqs
    // always installs the passband for the live sideband.
    if (m_channel) {
        m_channel->setMode(wdspMode);
    }
    applyTxFilter();
    applyFmPreEmphasis();
}

void Transmitter::applyTxFilter() {
    if (!m_channelCreated)
        return;
    const int rx = set ? set->getCurrentReceiver() : 0;
    if (set) this->mode = set->getDSPMode(rx);
    const DSPMode boundsMode = resolveWDSPMode(this->mode, set ? set->getCtrFrequency(rx) : 0);
    const int txLow = m_txModel ? m_txModel->txFilterLow() : (set ? set->getTxFilterLow() : 100);
    const int txHigh = m_txModel ? m_txModel->txFilterHigh() : (set ? set->getTxFilterHigh() : 2900);
    const bool useRx = m_txModel ? m_txModel->txUseRxFilter() : (set ? set->getTxUseRxFilter() : false);
    const double rxLo = set ? set->getFilterLo(rx) : 150.0;
    const double rxHi = set ? set->getFilterHi(rx) : 3050.0;

    const auto bounds = SettingsUtils::calculateTxFilterBounds(boundsMode, txLow, txHigh, useRx, rxLo, rxHi);
    this->filter_low = bounds.low;
    this->filter_high = bounds.high;
    tx_set_filter(bounds.low, bounds.high);
}

void Transmitter::tx_set_filter(double filter_low, double filter_high) {
    TRANSMITTER_DEBUG << "Set Tx filter:Low " << filter_low << " High: " << filter_high;
    if (m_channel) {
        m_channel->setFilter(filter_low, filter_high);
    }
}


     void Transmitter::transmitter_set_am_carrier_level(double level ) {
         TRANSMITTER_DEBUG << "Set Am Carrier Level " << level;
         if (m_channel) {
             m_channel->setAmCarrierLevel(level);
         }
     }

     long Transmitter::get_CtrFrequency(long rx_frequency, long repeater_offset, bool repeater_mode) {
        if (repeater_mode) {
            return rx_frequency + repeater_offset;
        }
        else return rx_frequency;

}


void Transmitter::transmitter_set_mic_level(int level){
    TRANSMITTER_DEBUG << "Set Tx mic level" << level;
    mic_gain = level * 1.0;
    if (m_channel) {
        m_channel->setMicGain(micSliderToPanelGain(mic_gain));
    }
}

void Transmitter::transmitter_set_audio_compression(int level)
{
    const int compressionDb = compressionSliderToDb(level);
    compressor_level = static_cast<float>(compressionDb);
    compressor = (compressionDb > 0) ? 1 : 0;
    if (m_channel) {
        m_channel->setAudioCompression(compressionDb, compressor != 0);
    }
    TRANSMITTER_DEBUG << "Set Tx compression " << compressionDb << " dB run=" << compressor;
}
