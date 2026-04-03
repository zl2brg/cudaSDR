#ifndef CUSDR_CODEC2PROCESSOR_H
#define CUSDR_CODEC2PROCESSOR_H

#include <cstdint>
#include <memory>
#include <vector>

extern "C" {
#include <codec2/codec2.h>
}

class Codec2Processor {
public:
    enum Mode {
        MODE_3200 = CODEC2_MODE_3200,  // 3200 bps
        MODE_2400 = CODEC2_MODE_2400,  // 2400 bps
        MODE_1600 = CODEC2_MODE_1600,  // 1600 bps
        MODE_1400 = CODEC2_MODE_1400,  // 1400 bps
        MODE_1300 = CODEC2_MODE_1300,  // 1300 bps
        MODE_700C = CODEC2_MODE_700C   // 700 bps
    };

    explicit Codec2Processor(Mode mode = MODE_1600);
    ~Codec2Processor();

    // Disable copy
    Codec2Processor(const Codec2Processor&) = delete;
    Codec2Processor& operator=(const Codec2Processor&) = delete;

    // Encode PCM audio to codec2 bits
    // Input: 16-bit PCM samples (8 kHz)
    // Returns number of bits encoded, or -1 on error
    int encode(const int16_t* pcm_in, uint8_t* bits_out);

    // Decode codec2 bits to PCM audio
    // Input: codec2 bits
    // Returns number of PCM samples decoded (or -1 on error)
    int decode(const uint8_t* bits_in, int16_t* pcm_out);

    // Get frame sizes
    int getFrameSamples() const;     // samples per frame (typically 320)
    int getEncodedBytes() const;     // bytes of encoded data per frame
    int getBitsPerFrame() const;     // number of bits per frame

    // Change mode
    void setMode(Mode mode);
    Mode getMode() const { return m_currentMode; }

private:
    CODEC2* m_codec2;
    Mode m_currentMode;

    void initCodec2(Mode mode);
    void cleanupCodec2();
};

#endif // CUSDR_CODEC2PROCESSOR_H
