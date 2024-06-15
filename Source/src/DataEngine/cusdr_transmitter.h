/* Copyright (C)
*
* Simon Eatough ZL2VH
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
*/


#ifndef CUDASDR_CUSDR_TRANSMITTER_H
#define CUDASDR_CUSDR_TRANSMITTER_H


#include "cusdr_settings.h"
#include "Util/cusdr_highResTimer.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "cusdr_hamDatabase.h"

#define LOG_TRANSMITTER

#ifdef LOG_TRANSMITTER
#define TRANSMITTER_DEBUG qDebug().nospace() << "Transmitter::\t"
#else
#   define TRANSMITTER_DEBUG nullDebug()
#endif


class Transmitter : public QObject {

    Q_OBJECT
public:
    explicit Transmitter(int transmitter = 0);
    ~Transmitter() override;
     double getNextInternalSideToneSample();
     double getNextSideToneSample();


private:
    void	setupConnections();
    bool createTransmitter(int id, int buffer_size, int fft_size, int fps, int width, int height);
    void init_analyser(int id);
    void reconfigure_transmitter(int id,int height);
    void set_mode(int  tx, int m);
    void tx_set_filter(double low,double high);


    void add_freedv_mic_sample(int tx,short mic_sample);

    void transmitter_save_state(int tx);
    void transmitter_set_out_of_band(int tx);

    void tx_set_mode(DSPMode mode);
    void tx_set_ps(int tx,int state);
    void tx_set_twotone(int tx,int state);

    void transmitter_set_compressor_level(int tx,double level);
    void transmitter_set_compressor(int tx,int state);

    void tx_set_ps_sample_rate(int tx,int rate);
    void add_ps_iq_samples(int tx, double i_sample_0,double q_sample_0, double i_sample_1, double q_sample_1);

    void cw_hold_key(int state);
    long get_CtrFrequency(long rx_frequency,long repeater_offset, bool repeater_mode);

    double cw_shape_buffer48[BUFFER_SIZE];
    double cw_shape_buffer192[BUFFER_SIZE];
    double cw_keyer_sidetone_frequency = 1000;




public slots:
    void setDSPMode(QObject *sender, int id, DSPMode dspMode);
    void setRadioState(RadioState state);


private slots:
    void set_fm_deviation(double level);
    void transmitter_set_am_carrier_level(double level);
    void tx_set_pre_emphasize(int tx,int state);
    void transmitter_set_mic_level(QObject *object, int level);

private:
    Settings*				set;
    int id;
    int enable_tx_equalizer;
    int tx_equalizer[4];

    int enable_rx_equalizer;
    int rx_equalizer[4];
    int mic_sample_rate;
    int mic_dsp_rate;
    int iq_output_rate;
    int buffer_size;
    int fft_size;
    int pixels;
    int samples;
    int output_samples;
    double mic_input_buffer[BUFFER_SIZE];
    double iq_output_buffer[BUFFER_SIZE];
    double mic_gain = 1;

    float pixel_samples[BUFFER_SIZE];
    int display_panadapter;
    int display_waterfall;
    int update_timer_id;

    DSPMode  mode;
    double filter_low;
    double filter_high;

    int rc;
    int tx_cfir;
    int tx_leveler = 0;
    double tone_level = 1.0;
    int tx_filter_low;
    int tx_filter_high;
    int drive_level = 10;
    int local_microphone;
    int input_device;

    int out_of_band;
    int low_latency;

    int twotone;
    int puresignal;
    int feedback;
    int auto_on;
    int single_on;

    double ctcss_frequency;
    int ctcss;

    int deviation;

    double am_carrier_level;

    int attenuation;

    int drive;
    int tune_use_drive;
    int tune_percent;
    int width;
    int height;
    int panadapter_high;
    int panadapter_low;
    int displaying;
    int alex_antenna;
    int compressor = 0;
    float compressor_level = 0;
    int dac;
    int fps;

#ifdef FREEDV
    char freedv_text_data[64];
    int freedv_text_index;
    int freedv_samples;
#endif



};


#endif //CUDASDR_CUSDR_TRANSMITTER_H
