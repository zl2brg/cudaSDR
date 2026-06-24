#ifndef CUSDR_RADEPROCESSOR_H
#define CUSDR_RADEPROCESSOR_H

#include <vector>
#include <QVector>

struct rade;
typedef void* rade_text_t;

class RadeProcessor {
public:
    explicit RadeProcessor();
    ~RadeProcessor();

    RadeProcessor(const RadeProcessor&) = delete;
    RadeProcessor& operator=(const RadeProcessor&) = delete;

    QVector<float> processSamples(const float* audio48k, int n);

    bool  isSync() const { return m_sync; }
    float getSNR()  const { return m_snr;  }

private:
    struct rade* m_rade = nullptr;
    void*        m_fargan = nullptr;
    rade_text_t  m_radeText = nullptr;

    bool  m_sync = false;
    float m_snr = 0.0f;

    std::vector<int16_t> m_modemBuf;   // accumulated 8 kHz modem-rate samples
    std::vector<float>   m_speechBuf;  // accumulated speech features
    std::vector<float>   m_outBuf;     // output rate-levelling ring buffer

    void decimate(const float* in48, int n_in);
    void upsampleToStereo(const float* speech, int n_speech,
                          std::vector<float>& out);
};

#endif // CUSDR_RADEPROCESSOR_H
