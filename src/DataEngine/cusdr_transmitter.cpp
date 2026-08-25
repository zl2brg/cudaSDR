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
            this, [this]() { SetTXAPHROTAutoReset(this->id); });

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
}



bool  Transmitter::create_transmitter(int id, int buffer_size, int fft_size, int fps, int width, int height) {

    int protocol = ORIGINAL_PROTOCOL;
    // Position 0 = pre-emphasis before FM modulator (WDSP default path).
    int pre_emphasize = 0;
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

    this->filter_low=0;
    this->filter_high=2500;

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

    OpenChannel(this->id,
                this->buffer_size,
                2048,
                this->mic_sample_rate,
                this->mic_dsp_rate,
                this->iq_output_rate,
                1, // transmit
                0, // run
                0.010, 0.025, 0.0, 0.010, 0);

    // create_txa already uses nc = max(2048, dsp_size). Only bump if requested larger.
    if (this->fft_size > 2048)
        TXASetNC(this->id, this->fft_size);
    TXASetMP(this->id, this->low_latency);


/*    int mode=vfo[VFO_A].mode;
    if(split) {
        mode=vfo[VFO_B].mode;
    }
*/
    SetTXABandpassWindow(this->id, 1);
    SetTXABandpassRun(this->id, 1);

    SetTXAFMEmphPosition(this->id,pre_emphasize);
    applyFmPreEmphasis();
    applyPhaseRotator();
    applyTxEq();
    applyCfc();

    SetTXACFIRRun(this->id, protocol==NEW_PROTOCOL?1:0); // turned on if new protocol

    // WDSP defaults CTCSS ON at 100 Hz — apply persisted UI setting (0 Hz = off).
    applyCtcss();
    SetTXAAMSQRun(this->id, 0);
    SetTXAosctrlRun(this->id, 0);

    // A gentler ALC profile reduces audible pumping/background lift.
    SetTXAALCAttack(this->id, 2);
    SetTXAALCDecay(this->id, 120);
    SetTXAALCSt(this->id, 1); // turn it on (always on)

    SetTXALevelerAttack(this->id, 1);
    SetTXALevelerDecay(this->id, 500);
    SetTXALevelerTop(this->id, 1.0);
    SetTXALevelerSt(this->id, tx_leveler);

    SetTXAPreGenMode(this->id, 0);
    SetTXAPreGenToneMag(this->id, 0.0);
    SetTXAPreGenToneFreq(this->id, 0.0);
    SetTXAPreGenRun(this->id, 0);

    SetTXAPostGenMode(this->id, 0);
    SetTXAPostGenToneMag(this->id, tone_level);
    SetTXAPostGenTTMag(this->id, tone_level,tone_level);
    SetTXAPostGenToneFreq(this->id, 1000.0);
    SetTXAPostGenRun(this->id, 0);

    const double initialMicLevel = set->getMicInputLevel();
    mic_gain = initialMicLevel;
    SetTXAPanelGain1(this->id, micSliderToPanelGain(initialMicLevel));
    SetTXAPanelRun(this->id, 1);

    const int fmDev = m_txModel ? m_txModel->fmDeviation()
                                : static_cast<int>(set->getFMDeveation());
    SetTXAFMDeviation(this->id, fmDev);
    const int amPercent = m_txModel ? m_txModel->amCarrierLevel()
                                    : qRound(set->getAMCarrierLevel() <= 1.0
                                                 ? set->getAMCarrierLevel() * 100.0
                                                 : set->getAMCarrierLevel());
    SetTXAAMCarrierLevel(this->id, amCarrierPercentToLevel(amPercent));
    transmitter_set_audio_compression(m_txModel ? m_txModel->audioCompression()
                                                : static_cast<int>(set->getAudioCompression()));
    XCreateAnalyzer(this->id, &rc, 262144, 1, 1, const_cast<char*>(""));
    if (rc != 0) {
        fprintf(stderr, "XCreateAnalyzer id=%d failed: %d\n",this->id,rc);
    } else {
        init_analyser(this->id);
    }
    return true;
}

void Transmitter::setDSPMode(int id, DSPMode dspMode) {
    Q_UNUSED(id)
    mode = dspMode;
    const DSPMode wdspMode = resolveWDSPMode(mode, set->getCtrFrequency(set->getCurrentReceiver()));
    TRANSMITTER_DEBUG << "[TX] DSP mode set to" << dspMode << "(WDSP:" << wdspMode << ")";
    SetTXAMode(this->id, wdspMode);
    applyFmPreEmphasis(); // SetTXAMode forces pre-emph on for FM; honor user setting.
    auto filter = getFilterFromDSPMode(set->getDefaultFilterList(), wdspMode);
    tx_set_filter(filter.filterLo, filter.filterHi);
}





void Transmitter::set_fm_deviation(double level) {
    SetTXAFMDeviation(this->id, level);
    TRANSMITTER_DEBUG << "Set Tx FM deveation " << level;

}

void Transmitter::applyFmPreEmphasis()
{
    // WDSP SetTXAMode(FM) forces preemph.run=1; re-apply user preference after mode changes.
    const bool enabled = m_txModel ? m_txModel->fmPreEmphasis()
                                   : (set->getFMpreemphesis() != 0.0);
    const int run = enabled ? 1 : 0;
    SetTXAFMEmphRun(this->id, run);
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
    SetTXAPHROTRun(this->id, run);
    SetTXAPHROTAutoMode(this->id, autoMode);
    syncPhaseRotatorTimer();
    TRANSMITTER_DEBUG << "Audio Phase Rotator " << (run ? "on" : "off")
                      << " auto=" << (autoMode ? "on" : "off");
}

void Transmitter::applyTxEq()
{
    const bool enabled = m_txModel ? m_txModel->txEqEnabled() : set->getTxEqEnabled();
    const int curveDeg = m_txModel ? m_txModel->txEqCurveDeg() : set->getTxEqCurveDeg();
    if (enabled) {
        const QVector<int> bands = m_txModel ? m_txModel->txEqBands() : set->getTxEqBands();
        int txeq[11];
        for (int i = 0; i < 11; ++i)
            txeq[i] = (i < bands.size()) ? bands.at(i) : 0;
        // GrphEQ10 loads F/G; Curve selects linear (deg=0) or NURBS.
        SetTXAGrphEQ10(this->id, txeq);
        SetTXAEQCurve(this->id, curveDeg, 0, 0);
        SetTXAEQRun(this->id, 1);
    } else {
        SetTXAEQRun(this->id, 0);
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
    const int n = qMin(freqs.size(), qMin(levels.size(), post.size()));
    if (n > 0) {
        QVector<double> F = freqs.mid(0, n);
        QVector<double> G = levels.mid(0, n);
        QVector<double> E = post.mid(0, n);
        SetTXACFCOMPprofile(this->id, n, F.data(), G.data(), E.data());
    }
    SetTXACFCOMPPrecomp(this->id, precomp);
    SetTXACFCOMPPrePeq(this->id, prePeq);
    SetTXACFCOMPCompCurve(this->id, deg, 0, 0);
    SetTXACFCOMPPeqCurve(this->id, deg, 0, 0);
    // Post-EQ requires compressor run; force CFC on when only Peq is requested.
    SetTXACFCOMPRun(this->id, (run || peq) ? 1 : 0);
    SetTXACFCOMPPeqRun(this->id, peq ? 1 : 0);
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
    if (!run || !autoMode)
        return;
    double in_pos = 0, in_neg = 0, in_ratio = 0;
    double out_pos = 0, out_neg = 0, out_ratio = 0;
    double current_fc = 0, auto_step = 0;
    GetTXAPHROTAsymmetry(this->id,
                         &in_pos, &in_neg, &in_ratio,
                         &out_pos, &out_neg, &out_ratio,
                         &current_fc, &auto_step);
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
    SetTXACTCSSFreq(this->id, this->ctcss_frequency);
    SetTXACTCSSRun(this->id, this->ctcss);
    TRANSMITTER_DEBUG << "CTCSS" << (this->ctcss ? "on" : "off") << "freq" << this->ctcss_frequency;
}

void Transmitter::setRadioState(RadioState state)
{
    switch(state) {

    case RadioState::MOX: {
        const DSPMode wdspMode = resolveWDSPMode(mode, set->getCtrFrequency(set->getCurrentReceiver()));
        auto filter = getFilterFromDSPMode(set->getDefaultFilterList(), wdspMode);
        tx_set_filter(filter.filterLo, filter.filterHi);
        SetTXAPostGenRun(this->id, 0);
        SetTXAMode(this->id, wdspMode);
        applyFmPreEmphasis();
        applyPhaseRotator();
        SetTXAPanelGain1(this->id, micSliderToPanelGain(mic_gain));
        SetTXAPanelRun(this->id, 1);
        SetTXABandpassWindow(this->id, 1);
        SetTXABandpassRun(this->id, 1);
        SetChannelState(TX_ID, 1, 1);
        TRANSMITTER_DEBUG << "MOX: TX channel started";
        break;
    }

    case RadioState::TUNE: {
        // Tone generator for TUNE
        const DSPMode wdspModeTune = resolveWDSPMode(mode, set->getCtrFrequency(set->getCurrentReceiver()));
        auto filter = getFilterFromDSPMode(set->getDefaultFilterList(), wdspModeTune);
        tx_set_filter(filter.filterLo, filter.filterHi);
        SetTXAPostGenToneFreq(this->id, 1000);
        SetTXAPostGenToneMag(this->id, 0.5);
        SetTXAPostGenMode(this->id, 0);
        SetTXAPostGenRun(this->id, 1);
        SetTXAMode(this->id, wdspModeTune);
        applyFmPreEmphasis();
        applyPhaseRotator();
        SetTXAPanelGain1(this->id, micSliderToPanelGain(mic_gain));
        SetTXAPanelRun(this->id, 1);
        SetTXABandpassWindow(this->id, 1);
        SetTXABandpassRun(this->id, 1);
        SetChannelState(TX_ID, 1, 1);
        TRANSMITTER_DEBUG << "TUNE: TX channel started with tone";
        break;
    }

    case RadioState::RX:
    default:
        SetTXAPostGenRun(this->id, 0);
        SetChannelState(TX_ID, 0, 1);
        SetChannelState(0, 1, 1);
        TRANSMITTER_DEBUG << "RX: TX channel stopped";
        break;
    }
}


     void Transmitter::tx_set_filter(double filter_low,double filter_high){
         TRANSMITTER_DEBUG << "Set Tx filter:Low " << filter_low << " High: " << filter_high;
         SetTXABandpassFreqs(this->id, filter_low,filter_high);
     }


     void Transmitter::transmitter_set_am_carrier_level(double level ) {
         TRANSMITTER_DEBUG << "Set Am Carrier Level " << level;
         SetTXAAMCarrierLevel(this->id, level);
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
    SetTXAPanelGain1(this->id, micSliderToPanelGain(mic_gain));

}

void Transmitter::transmitter_set_audio_compression(int level)
{
    const int compressionDb = compressionSliderToDb(level);
    compressor_level = static_cast<float>(compressionDb);
    compressor = (compressionDb > 0) ? 1 : 0;
    SetTXACompressorGain(this->id, compressionDb);
    SetTXACompressorRun(this->id, compressor);
    TRANSMITTER_DEBUG << "Set Tx compression " << compressionDb << " dB run=" << compressor;
}



     void Transmitter::init_analyser(int tx) {
         Q_UNUSED(tx)
         int flp[] = {0};
         double keep_time = 0.1;
         int n_pixout=1;
         int spur_elimination_ffts = 1;
         int data_type = 1;
         int fft_size = 2048;
         int window_type = 4;
         double kaiser_pi = 14.0;
         int overlap = 0;
         int clip = 0;
         int span_clip_l = 0;
         int span_clip_h = 0;
         int stitches = 1;
         int calibration_data_set = 0;
         double span_min_freq = 0.0;
         double span_max_freq = 0.0;

         int max_w = fft_size + (int) min(keep_time * (double) this->fps, keep_time * (double) fft_size * (double) this->fps);

         overlap = (int)max(0.0, ceil(fft_size - (double)this->mic_sample_rate / (double)this->fps));

         TRANSMITTER_DEBUG << "SetAnalyzer id=" << this->id << " buffer_size=" << output_samples << " overlap=" << overlap;


         SetAnalyzer(this->id,
                     n_pixout,
                     spur_elimination_ffts, //number of LO frequencies = number of ffts used in elimination
                     data_type, //0 for real input data (I only); 1 for complex input data (I & Q)
                     flp, //vector with one elt for each LO frequency, 1 if high-side LO, 0 otherwise
                     fft_size, //size of the fft, i.e., number of input samples
                     1024, //number of samples transferred for each OpenBuffer()/CloseBuffer()
                     window_type, //integer specifying which window function to use
                     kaiser_pi, //PiAlpha parameter for Kaiser window
                     overlap, //number of samples each fft (other than the first) is to re-use from the previous
                     clip, //number of fft output bins to be clipped from EACH side of each sub-span
                     span_clip_l, //number of bins to clip from low end of entire span
                     span_clip_h, //number of bins to clip from high end of entire span
                     4096, //number of pixel values to return.  may be either <= or > number of bins
                     stitches, //number of sub-spans to concatenate to form a complete span
                     calibration_data_set, //identifier of which set of calibration data to use
                     span_min_freq, //frequency at first pixel value8192
                     span_max_freq, //frequency at last pixel value
                     max_w //max samples to hold in input ring buffers
                     );
     }


