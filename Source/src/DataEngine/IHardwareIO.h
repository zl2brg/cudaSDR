/**
 * @file  IHardwareIO.h
 * @brief Abstract interface for SDR hardware transport handlers.
 *
 * Concrete implementations:
 *   - DataIO          : HPSDR Protocol 1 (Metis / Hermes UDP)
 *   - Protocol2Handler: HPSDR Protocol 2 (TCP EP6 streams)  [stub]
 *   - SoapySDRHandler : SoapySDR abstraction layer          [stub]
 */

#ifndef IHARDWAREIO_H
#define IHARDWAREIO_H

#include <QObject>

class IHardwareIO : public QObject {
    Q_OBJECT

public:
    explicit IHardwareIO(QObject* parent = nullptr) : QObject(parent) {}
    ~IHardwareIO() override = default;

    // Non-copyable
    IHardwareIO(const IHardwareIO&)            = delete;
    IHardwareIO& operator=(const IHardwareIO&) = delete;

public slots:
    /**
     * @brief Initialize the hardware transport (socket bind, stream open, etc.).
     *        Connected to the owning QThread::started() signal by DataEngine.
     */
    virtual void initIO() = 0;

    /** @brief Stop the transport and release all resources. */
    virtual void stop() = 0;

    /**
     * @brief Write one outgoing audio/IQ frame to the device.
     *        Called by DataProcessor after encoding a full output buffer.
     */
    virtual void writeData() = 0;

    /**
     * @brief Forward an encoded audio output buffer to the device speaker path.
     * @param buf  Pointer to a 512-byte HPSDR-format output buffer.
     */
    virtual void sendAudio(unsigned char* buf) = 0;

    /**
     * @brief Send per-receiver frequency initialisation frames.
     * @param rx  Zero-based receiver index.
     */
    virtual void sendInitFrames(int rx) = 0;

    /**
     * @brief Send a start/stop command byte to the device.
     * @param cmd  0x00 = stop, 0x01 = start (no wideband), 0x03 = start (with wideband).
     */
    virtual void sendCommand(char cmd) = 0;

    /**
     * @brief Update the number of wideband capture buffers.
     * @param val  New buffer count (see Settings::getWidebandBuffers).
     */
    virtual void set_wbBuffers(int val) = 0;

signals:
    /**
     * @brief Emitted whenever one complete IQ frame has been placed on the
     *        DataEngine::io.iq_queue, ready for DataProcessor to consume.
     */
    void readydata();

    /** @brief Human-readable status / error text for the UI message bar. */
    void messageEvent(QString message);
};

#endif // IHARDWAREIO_H
