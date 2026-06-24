#include "cusdr_radeprocessor.h"
#include "rade_api.h"
#include "rade_text.h"
extern "C" {
#include "fargan.h"
#include "lpcnet.h"
}
#include <QDebug>
#include <algorithm>
#include <cmath>
#if defined(Q_OS_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#endif

namespace {
bool radeVerboseEnabled() {
    static const bool enabled = qEnvironmentVariableIntValue("CUSDR_RADE_VERBOSE") != 0;
    return enabled;
}

#if defined(Q_OS_LINUX)
template <typename Fn>
auto runWithSuppressedCStdio(Fn&& fn) -> decltype(fn()) {
    if (radeVerboseEnabled())
        return fn();

    fflush(stdout);
    fflush(stderr);
    const int savedOut = dup(STDOUT_FILENO);
    const int savedErr = dup(STDERR_FILENO);
    const int devNull = open("/dev/null", O_WRONLY);
    if (devNull < 0 || savedOut < 0 || savedErr < 0) {
        if (devNull >= 0) close(devNull);
        if (savedOut >= 0) close(savedOut);
        if (savedErr >= 0) close(savedErr);
        return fn();
    }

    dup2(devNull, STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);
    close(devNull);

    auto result = fn();

    fflush(stdout);
    fflush(stderr);
    dup2(savedOut, STDOUT_FILENO);
    dup2(savedErr, STDERR_FILENO);
    close(savedOut);
    close(savedErr);
    return result;
}
#else
template <typename Fn>
auto runWithSuppressedCStdio(Fn&& fn) -> decltype(fn()) {
    return fn();
}
#endif
} // namespace

RadeProcessor::RadeProcessor() {
    char emptyModel[1] = {0};
    m_rade = rade_open(emptyModel, RADE_USE_C_DECODER);
    if (!m_rade) {
        qWarning() << "RadeProcessor: failed to open RADE v1 decoder";
        return;
    }

    m_fargan = new FARGANState();
    fargan_init((FARGANState*)m_fargan);
    
    // Warm up FARGAN state
    float zeros[320] = {0};
    float in_features[5 * NB_TOTAL_FEATURES] = {0};
    fargan_cont((FARGANState*)m_fargan, zeros, in_features);

    m_radeText = rade_text_create();
    
    m_modemBuf.reserve(rade_nin_max(m_rade) * 2);
    m_speechBuf.reserve(RADE_SPEECH_SAMPLE_RATE);
}

RadeProcessor::~RadeProcessor() {
    if (m_rade) rade_close(m_rade);
    if (m_fargan) delete (FARGANState*)m_fargan;
    if (m_radeText) rade_text_destroy(m_radeText);
}

void RadeProcessor::decimate(const float* in48, int n_in) {
    int n_out = n_in / 6;
    for (int i = 0; i < n_out; ++i) {
        float sum = 0.0f;
        const float* p = in48 + i * 6;
        for (int k = 0; k < 6; ++k)
            sum += p[k];
        float v = (sum / 6.0f) * 32767.0f;
        m_modemBuf.push_back(static_cast<int16_t>(std::clamp(v, -32768.f, 32767.f)));
    }
}

void RadeProcessor::upsampleToStereo(const float* speech, int n_speech, std::vector<float>& out) {
    out.reserve(out.size() + n_speech * 3 * 2);
    for (int i = 0; i < n_speech; ++i) {
        float s = speech[i];
        for (int k = 0; k < 3; ++k) {
            out.push_back(s); // L
            out.push_back(s); // R
        }
    }
}

QVector<float> RadeProcessor::processSamples(const float* audio48k, int n) {
    if (!m_rade)
        return QVector<float>(n * 2, 0.0f);

    decimate(audio48k, n);

    int nin = rade_nin(m_rade);
    std::vector<RADE_COMP> inputCplx(nin);
    std::vector<float> featuresOut(rade_n_features_in_out(m_rade));
    std::vector<float> eooOut(rade_n_eoo_bits(m_rade));

    while (static_cast<int>(m_modemBuf.size()) >= nin) {
        for (int i = 0; i < nin; ++i) {
            inputCplx[i].real = m_modemBuf[i] / 32767.0f;
            inputCplx[i].imag = 0.0f;
        }
        m_modemBuf.erase(m_modemBuf.begin(), m_modemBuf.begin() + nin);

        int hasEooOut = 0;
        const int nout = runWithSuppressedCStdio([&]() {
            return rade_rx(m_rade, featuresOut.data(), &hasEooOut, eooOut.data(), inputCplx.data());
        });

        m_sync = (rade_sync(m_rade) != 0);
        if (m_sync) {
            m_snr = rade_snrdB_3k_est(m_rade);
        }

        if (hasEooOut) {
            rade_text_rx(m_radeText, eooOut.data(), rade_n_eoo_bits(m_rade) / 2);
        } else if (nout > 0) {
            // Push features into temporary buffer and synthesize when frame is full
            for (int i = 0; i < nout; ++i) {
                m_speechBuf.push_back(featuresOut[i]);
            }
            
            while (m_speechBuf.size() >= NB_TOTAL_FEATURES) {
                float fpcm[LPCNET_FRAME_SIZE] = {0};
                fargan_synthesize((FARGANState*)m_fargan, fpcm, m_speechBuf.data());
                m_speechBuf.erase(m_speechBuf.begin(), m_speechBuf.begin() + NB_TOTAL_FEATURES);
                upsampleToStereo(fpcm, LPCNET_FRAME_SIZE, m_outBuf);
            }
        }
        nin = rade_nin(m_rade);
    }

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
