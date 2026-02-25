/**
 * @file  IFrameDecoder.h
 * @brief Abstract interface for Protocol-specific IQ frame encode/decode.
 *
 * Implementations are responsible for:
 *   - Parsing raw incoming wire frames into receiver IQ buffers
 *     (de->RX[r]->inBuf) and hardware status registers (de->io.ccRx).
 *   - Encoding outgoing hardware control/command bytes into the
 *     de->io.output_buffer ready for sendAudio() / writeData().
 *
 * Concrete implementations:
 *   - Protocol1FrameDecoder : HPSDR Protocol 1 wire format
 *   - (future) Protocol2FrameDecoder : HPSDR Protocol 2 EP6 format
 *   - (future) SoapyFrameDecoder     : SoapySDR stream adapter
 */

#ifndef IFRAMEDECODER_H
#define IFRAMEDECODER_H

#include <QObject>
#include <QByteArray>

class IFrameDecoder : public QObject {
    Q_OBJECT

public:
    explicit IFrameDecoder(QObject* parent = nullptr) : QObject(parent) {}
    ~IFrameDecoder() override = default;

    IFrameDecoder(const IFrameDecoder&)            = delete;
    IFrameDecoder& operator=(const IFrameDecoder&) = delete;

    /**
     * @brief Decode one 512-byte (half-frame) incoming IQ buffer.
     *
     * Implementations must:
     *  - Parse receiver IQ triplets and populate de->RX[r]->inBuf.
     *  - Decode hardware C&C / status bytes into de->io.ccRx.
     *  - Trigger DSP processing on each receiver when BUFFER_SIZE samples
     *    have been accumulated (via QMetaObject::invokeMethod).
     *
     * @param buffer  Raw 512-byte half-frame from the hardware.
     */
    virtual void decodeInputFrame(const QByteArray& buffer) = 0;

    /**
     * @brief Set the number of receivers whose frequency frames must be
     *        force-sent during startup initialisation.
     *
     * Default no-op — only Protocol 1 overrides this.
     */
    virtual void setFirstTimeRxInit(int n) { Q_UNUSED(n) }

    /**
     * @brief Encode the next round of outgoing C&C bytes into
     *        de->io.output_buffer[0..2] (SYNC) and control_out[0..4].
     *
     * This implements the Protocol 1 round-robin send state machine
     * (frequency, drive level, configuration, Alex settings, etc.).
     * Must be called once per output TX frame, before writeData().
     */
    virtual void encodeControlBytes() = 0;

signals:
    /**
     * @brief Emitted when the decoded C&C dash/dot key state changes.
     * @param key    0 = dash, 1 = dot.
     * @param state  1 = key down, 0 = key up.
     */
    void keyerEvent(int key, int state);
};

#endif // IFRAMEDECODER_H
