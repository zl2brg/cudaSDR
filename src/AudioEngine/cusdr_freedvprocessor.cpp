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
    m_speechUpsample = 48000 / freedv_get_speech_sample_rate(m_fdv);

    int maxModem  = freedv_get_n_max_modem_samples(m_fdv);
    int maxSpeech = freedv_get_n_max_speech_samples(m_fdv);

    m_modemBuf.reserve(maxModem * 2);
    m_speechTmp.resize(maxSpeech);

    qDebug().nospace() << "FreeDVProcessor: mode=" << freedvMode
                       << " modemRate=" << m_modemRate
                       << " speechRate=" << freedv_get_speech_sample_rate(m_fdv)
                       << " decFactor=" << m_decFactor
                       << " speechUpsample=" << m_speechUpsample
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
                                        std::vector<float>& out)
{
    // Zero-order hold: each speech-rate sample is repeated m_speechUpsample times
    // to reach 48 kHz, then duplicated to stereo (L == R).
    const float scale = 1.0f / 32768.0f;
    out.reserve(out.size() + n_speech * m_speechUpsample * 2);
    for (int i = 0; i < n_speech; ++i) {
        float s = static_cast<float>(speech[i]) * scale;
        for (int k = 0; k < m_speechUpsample; ++k) {
            out.push_back(s); // L
            out.push_back(s); // R
        }
    }
}

QVector<float> FreeDVProcessor::processSamples(const float* audio48k, int n)
{
    if (!m_fdv)
        return QVector<float>(n * 2, 0.0f);

    decimate(audio48k, n);

    while (static_cast<int>(m_modemBuf.size()) >= freedv_nin(m_fdv)) {
        int nin  = freedv_nin(m_fdv);
        int nout = freedv_rx(m_fdv, m_speechTmp.data(), m_modemBuf.data());

        m_modemBuf.erase(m_modemBuf.begin(), m_modemBuf.begin() + nin);

        int sync_flag = 0;
        freedv_get_modem_stats(m_fdv, &sync_flag, &m_snr);
        m_sync = (sync_flag != 0);

        if (nout > 0)
            upsampleToStereo(m_speechTmp.data(), nout, m_outBuf);
    }

    // Pull exactly n*2 floats from the output ring buffer.
    // Pad with silence if the buffer doesn't have enough yet.
    const int want = n * 2;
    QVector<float> result(want, 0.0f);
    const int have = static_cast<int>(m_outBuf.size());
    const int copy = std::min(have, want);
    if (copy > 0) {
        std::copy(m_outBuf.begin(), m_outBuf.begin() + copy, result.begin());
        m_outBuf.erase(m_outBuf.begin(), m_outBuf.begin() + copy);
    }
    return result;
}
