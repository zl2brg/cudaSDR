#include "cusdr_codec2processor.h"
#include <cstring>
#include <iostream>

Codec2Processor::Codec2Processor(Mode mode)
    : m_codec2(nullptr), m_currentMode(mode)
{
    initCodec2(mode);
}

Codec2Processor::~Codec2Processor()
{
    cleanupCodec2();
}

void Codec2Processor::initCodec2(Mode mode)
{
    cleanupCodec2();
    m_codec2 = codec2_create(static_cast<int>(mode));
    if (!m_codec2) {
        std::cerr << "Failed to create codec2 instance with mode " << static_cast<int>(mode) << std::endl;
    }
    m_currentMode = mode;
}

void Codec2Processor::cleanupCodec2()
{
    if (m_codec2) {
        codec2_destroy(m_codec2);
        m_codec2 = nullptr;
    }
}

int Codec2Processor::encode(const int16_t* pcm_in, uint8_t* bits_out)
{
    if (!m_codec2 || !pcm_in || !bits_out) {
        return -1;
    }

    // codec2_encode expects bits to be packed in unsigned char array
    codec2_encode(m_codec2, bits_out, (int16_t*)pcm_in);
    return getEncodedBytes();
}

int Codec2Processor::decode(const uint8_t* bits_in, int16_t* pcm_out)
{
    if (!m_codec2 || !bits_in || !pcm_out) {
        return -1;
    }

    codec2_decode(m_codec2, pcm_out, (uint8_t*)bits_in);
    return getFrameSamples();
}

int Codec2Processor::getFrameSamples() const
{
    if (!m_codec2) return -1;
    return codec2_samples_per_frame(m_codec2);
}

int Codec2Processor::getEncodedBytes() const
{
    if (!m_codec2) return -1;
    return codec2_bytes_per_frame(m_codec2);
}

int Codec2Processor::getBitsPerFrame() const
{
    if (!m_codec2) return -1;
    return codec2_bits_per_frame(m_codec2);
}

void Codec2Processor::setMode(Mode mode)
{
    if (m_currentMode != mode) {
        initCodec2(mode);
    }
}
