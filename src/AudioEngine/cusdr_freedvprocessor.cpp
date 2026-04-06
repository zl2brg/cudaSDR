#include "cusdr_freedvprocessor.h"

extern "C" {
#include <codec2/freedv_api.h>
}

#include <QDebug>
#include <algorithm>

FreeDVProcessor::FreeDVProcessor(int freedvMode)
{
    m_fdv = freedv_open(freedvMode);
    if (!m_fdv) {
        qWarning() << "FreeDVProcessor: failed to open FreeDV mode" << freedvMode;
        return;
    }

    m_modemRate = freedv_get_modem_sample_rate(m_fdv);
    m_decFactor = 48000 / m_modemRate;

    int maxModem  = freedv_get_n_max_modem_samples(m_fdv);
    int maxSpeech = freedv_get_n_max_speech_samples(m_fdv);

    m_modemBuf.reserve(maxModem * 2);
    m_speechTmp.resize(maxSpeech);

    qDebug().nospace() << "FreeDVProcessor: mode=" << freedvMode
                       << " modemRate=" << m_modemRate
                       << " speechRate=" << freedv_get_speech_sample_rate(m_fdv)
                       << " decFactor=" << m_decFactor
                       << " nomModemSamples=" << freedv_get_n_nom_modem_samples(m_fdv)
                       << " maxSpeechSamples=" << maxSpeech;
}

FreeDVProcessor::~FreeDVProcessor()
{
    if (m_fdv) {
        freedv_close(m_fdv);
        m_fdv = nullptr;
    }
}

void FreeDVProcessor::decimate(const float* in48, int n_in)
{
    // Average every m_decFactor input samples → one int16 output.
    // This acts as a simple boxcar anti-aliasing filter before decimation.
    int n_out = n_in / m_decFactor;
    for (int i = 0; i < n_out; ++i) {
        float sum = 0.0f;
        const float* p = in48 + i * m_decFactor;
        for (int k = 0; k < m_decFactor; ++k)
            sum += p[k];
        float v = (sum / (float)m_decFactor) * 32767.0f;
        if (v >  32767.0f) v =  32767.0f;
        if (v < -32768.0f) v = -32768.0f;
        m_modemBuf.push_back(static_cast<int16_t>(v));
    }
}

void FreeDVProcessor::upsampleToStereo(const int16_t* speech, int n_speech,
                                        QVector<float>& out)
{
    // Zero-order hold: each 8 kHz sample is repeated m_decFactor times
    // to reach 48 kHz, then duplicated to stereo (L == R).
    const float scale = 1.0f / 32768.0f;
    out.reserve(out.size() + n_speech * m_decFactor * 2);
    for (int i = 0; i < n_speech; ++i) {
        float s = static_cast<float>(speech[i]) * scale;
        for (int k = 0; k < m_decFactor; ++k) {
            out.append(s); // L
            out.append(s); // R
        }
    }
}

QVector<float> FreeDVProcessor::processSamples(const float* audio48k, int n)
{
    QVector<float> result;
    if (!m_fdv)
        return result;

    decimate(audio48k, n);

    // freedv_nin() returns the number of modem samples needed for the NEXT
    // call.  Capture it BEFORE calling freedv_rx(), because freedv_rx()
    // updates the internal state (and may change what nin returns afterwards).
    while (static_cast<int>(m_modemBuf.size()) >= freedv_nin(m_fdv)) {
        int nin  = freedv_nin(m_fdv);
        int nout = freedv_rx(m_fdv, m_speechTmp.data(), m_modemBuf.data());

        // Remove the exactly nin samples that were consumed.
        m_modemBuf.erase(m_modemBuf.begin(), m_modemBuf.begin() + nin);

        // Update sync state and SNR estimate.
        int sync_flag = 0;
        freedv_get_modem_stats(m_fdv, &sync_flag, &m_snr);
        m_sync = (sync_flag != 0);

        if (nout > 0)
            upsampleToStereo(m_speechTmp.data(), nout, result);
    }

    return result;
}
