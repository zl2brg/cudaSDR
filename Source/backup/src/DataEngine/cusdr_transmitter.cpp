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

Transmitter::Transmitter( int transmitter )
: QObject()
, set(Settings::instance())
{
    create_transmitter(TX_ID,DSP_SAMPLE_SIZE,4096,10,2048,100);
    setupConnections();

}

Transmitter::~Transmitter() {

}



// Sine tone generator:
// somewhat improved, and provided two siblings
// for generating side tones simultaneously on the
// HPSDR board and local audio.

#define TWOPIOVERSAMPLERATE 0.0001308996938995747;  // 2 Pi / 48000

static long asteps = 0;
static long bsteps = 0;

double Transmitter::getNextSideToneSample() {
    double angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
    if (++asteps == 48000) asteps = 0;
    return sin(angle);
}

double Transmitter::getNextInternalSideToneSample() {

    double angle = (bsteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
    if (++bsteps == 48000) bsteps = 0;
    return sin(angle);
}



void Transmitter::setupConnections() {

        CHECKED_CONNECT(
                set,
                SIGNAL(dspModeChanged(QObject *, int, DSPMode)),
                this,
                SLOT(setDSPMode(QObject *,int,  DSPMode)));

        CHECKED_CONNECT(
                set,
                SIGNAL(radioStateChanged(RadioState)),
                this,
                SLOT(setRadioState(RadioState)));

        CHECKED_CONNECT(
            set,
            SIGNAL(fmdeveationchanged(double)),
            this,
            SLOT(set_fm_deviation(double)));

        CHECKED_CONNECT(
            set,
            SIGNAL(amCarrierlevelchanged(double)),
            this,
            SLOT(transmitter_set_am_carrier_level(double)));


    CHECKED_CONNECT(
            set,
            SIGNAL(micInputLevelChanged(QObject *, int)),
            this,
            SLOT(transmitter_set_mic_level(QObject *, int)));


}



bool  Transmitter::create_transmitter(int id, int buffer_size, int fft_size, int fps, int width, int height) {

    int protocol = ORIGINAL_PROTOCOL;
    int pre_emphasize =0;
    int enable_tx_equalizer = 0;
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

    fprintf(stderr,"create_transmitter: id=%d buffer_size=%d mic_sample_rate=%d mic_dsp_rate=%d iq_output_rate=%d output_samples=%d fps=%d\n",this->id, this->buffer_size, this->mic_sample_rate, this->mic_dsp_rate, this->iq_output_rate, this->output_samples,this->fps);

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
    this->compressor_level=set->getAudioCompression();

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
    fprintf(stderr,"transmitter: allocate buffers: mic_input_buffer=%p iq_output_buffer=%p pixels=%p\n",this->mic_input_buffer,this->iq_output_buffer,this->pixel_samples);

    fprintf(stderr,"create_transmitter: OpenChannel id=%d buffer_size=%d fft_size=%d sample_rate=%d dspRate=%d outputRate=%d\n",
            this->id,
            this->buffer_size,
            2048, // this->fft_size,
            this->mic_sample_rate,
            this->mic_dsp_rate,
            this->iq_output_rate);

    OpenChannel(this->id,
                this->buffer_size,
                2048, // this->fft_size,
                this->mic_sample_rate,
                this->mic_dsp_rate,
                this->iq_output_rate,
                1, // transmit
                0, // run
                0.010, 0.025, 0.0, 0.010, 0);

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

    SetTXACFIRRun(this->id, protocol==NEW_PROTOCOL?1:0); // turned on if new protocol
    if(enable_tx_equalizer) {
        SetTXAGrphEQ(this->id, tx_equalizer);
        SetTXAEQRun(this->id, 1);
    } else {
        SetTXAEQRun(this->id, 0);
    }

 //   transmitter_set_ctcss(this->id,this->ctcss,this->ctcss_frequency);
    SetTXAAMSQRun(this->id, 0);
    SetTXAosctrlRun(this->id, 0);

    SetTXAALCAttack(this->id, 1);
    SetTXAALCDecay(this->id, 10);
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

    SetTXAPanelGain1(this->id,pow(10.0, mic_gain/20.0));
    SetTXAPanelRun(this->id, 1);

    SetTXAFMDeviation(this->id, set->getFMDeveation());
    SetTXAAMCarrierLevel(this->id, 0.5);
    SetTXACompressorGain(this->id, 0);
    SetTXACompressorRun(this->id, 0);
    XCreateAnalyzer(this->id, &rc, 262144, 1, 1, "");
    if (rc != 0) {
        fprintf(stderr, "XCreateAnalyzer id=%d failed: %d\n",this->id,rc);
    } else {
        init_analyser(this->id);
    }
    return true;
}

void Transmitter::reconfigure_transmitter(int tx, int height) {

}

void Transmitter::setDSPMode(QObject *sender,int id, DSPMode dspMode) {
Q_UNUSED(sender)
mode = dspMode;
    SetTXAMode(this->id, mode);
    tx_set_filter(getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterLo,getFilterFromDSPMode(set->getDefaultFilterList(), mode).filterHi);
}




void Transmitter::transmitter_set_ctcss(int tx, int, double)
{


}

void Transmitter::set_fm_deviation(double level) {
    SetTXAFMDeviation(this->id, level);
    TRANSMITTER_DEBUG << "Set Tx FM deveation " << level;

}

void Transmitter::setRadioState(RadioState state)
{

    switch(state){

case RadioState::MOX:
     SetChannelState(TX_ID,1,1);
     //SetChannelState(0,0,1);
     SetTXAPostGenRun(this->id, 0);

    break;
case RadioState::TUNE:
    SetChannelState(TX_ID,1,1);
   // SetChannelState(0,0,1);
    SetTXAPostGenRun(this->id, 1);
    break;

case RadioState::RX:
default:
     SetChannelState(TX_ID,0,1);
     SetChannelState(0,1,1);
     SetTXAPostGenRun(this->id, 0);

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

     void Transmitter::tx_set_pre_emphasize(int tx, int state) {

     }

     void Transmitter::add_freedv_mic_sample(int tx, short mic_sample) {

     }

     void Transmitter::transmitter_save_state(int tx) {

     }

     void Transmitter::transmitter_set_out_of_band(int tx) {

     }

    void Transmitter::tx_set_ps(int tx, int state) {

     }

     void Transmitter::tx_set_twotone(int tx, int state) {

     }

     void Transmitter::transmitter_set_compressor_level(int tx, double level) {

     }

     void Transmitter::transmitter_set_compressor(int tx, int state) {

     }

     void Transmitter::tx_set_ps_sample_rate(int tx, int rate) {

     }

     void Transmitter::add_ps_iq_samples(int tx, double i_sample_0, double q_sample_0, double i_sample_1,
                                         double q_sample_1) {

     }

     void Transmitter::cw_hold_key(int state) {

     }

     long Transmitter::get_CtrFrequency(long rx_frequency, long repeater_offset, bool repeater_mode) {
        if (repeater_mode) {
            return rx_frequency + repeater_offset;
        }
        else return rx_frequency;

}


void Transmitter::transmitter_set_mic_level(QObject *object, int level){
    TRANSMITTER_DEBUG << "Set Tx mic level" << level;
    mic_gain = level * 1.0;
    SetTXAPanelGain1(this->id,pow(10.0, mic_gain/20.0));

}



     void Transmitter::init_analyser(int tx) {
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
         int pixels=this->pixels;
         int stitches = 1;
         int avm = 0;
         double tau = 0.001 * 120.0;
         int calibration_data_set = 0;
         double span_min_freq = 0.0;
         double span_max_freq = 0.0;

         int max_w = fft_size + (int) min(keep_time * (double) this->fps, keep_time * (double) fft_size * (double) this->fps);

         overlap = (int)max(0.0, ceil(fft_size - (double)this->mic_sample_rate / (double)this->fps));

         fprintf(stderr,"SetAnalyzer id=%d buffer_size=%d overlap=%d\n",this->id,this->output_samples,overlap);


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


