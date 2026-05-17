#ifndef CUSDR_FREEDVPROCESSOR_H
#define CUSDR_FREEDVPROCESSOR_H

#include <vector>
#include <QVector>

// Forward-declare struct freedv to avoid pulling codec2/comp.h into the
// header chain (it would clash with the cuSDR COMP complex type).
struct freedv;

/**
 * FreeDVProcessor
 *
 * Wraps the libcodec2 FreeDV RX demodulator.
 *
 * Input:  mono float audio at 48 kHz (demodulated USB audio from WDSP,
 *         containing the FreeDV subcarrier tones).
 * Output: stereo float audio at 48 kHz containing the decoded speech,
 *         ready for ReceiverAudioOutput::writeAudio().
 *
 * The processor decimates the 48 kHz stream to the FreeDV modem sample rate
 * internally, accumulates samples until a full modem frame is available,
 * runs freedv_rx(), then zero-order-hold upsamples the decoded speech back
 * to 48 kHz stereo.
 */
class FreeDVProcessor {
public:
    // Default mode 0 = FREEDV_MODE_1600 (defined in freedv_api.h).
    explicit FreeDVProcessor(int freedvMode = 0);
    ~FreeDVProcessor();

    FreeDVProcessor(const FreeDVProcessor&) = delete;
    FreeDVProcessor& operator=(const FreeDVProcessor&) = delete;

    /**
     * Process n samples of demodulated USB audio at 48 kHz.
     * Returns decoded speech as stereo float at 48 kHz.
     * May return an empty vector when fewer than freedv_nin() modem samples
     * have accumulated.
     */
    QVector<float> processSamples(const float* audio48k, int n);

    bool  isSync() const { return m_sync; }
    float getSNR()  const { return m_snr;  }

private:
    struct freedv* m_fdv       = nullptr;
    int   m_modemRate           = 8000;
    int   m_decFactor           = 6;       // 48000 / m_modemRate  (input decimation)
    int   m_speechUpsample      = 6;       // 48000 / speechRate   (output upsample)
    bool  m_sync                = false;
    float m_snr                 = 0.0f;

    std::vector<int16_t> m_modemBuf;   // accumulated modem-rate samples
    std::vector<int16_t> m_speechTmp;  // scratch buffer for one freedv_rx call
    std::vector<float>   m_outBuf;     // output rate-levelling ring buffer

    // Decimate n_in float samples (48 kHz) → int16 at modem rate,
    // appended to m_modemBuf.
    void decimate(const float* in48, int n_in);

    // Zero-order-hold upsample n_speech int16 samples (modem speech rate)
    // to 48 kHz stereo float, appended to out.
    void upsampleToStereo(const int16_t* speech, int n_speech,
                          std::vector<float>& out);
};

#endif // CUSDR_FREEDVPROCESSOR_H
