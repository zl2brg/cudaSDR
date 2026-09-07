#ifndef WDSP_CHANNEL_H
#define WDSP_CHANNEL_H

#include <QMutex>
#include <QObject>
#include <atomic>

/**
 * @class WdspChannel
 * @brief Base RAII class managing WDSP channel lifecycle, state flags, and serialized FFTW operations.
 */
class WdspChannel {
public:
    explicit WdspChannel(int channelId);
    virtual ~WdspChannel();

    // Non-copyable, non-movable
    WdspChannel(const WdspChannel&) = delete;
    WdspChannel& operator=(const WdspChannel&) = delete;
    WdspChannel(WdspChannel&&) = delete;
    WdspChannel& operator=(WdspChannel&&) = delete;

    int channelId() const { return m_channelId; }
    bool isOpen() const { return m_isOpen.load(std::memory_order_acquire); }
    bool isRunning() const { return m_isRunning.load(std::memory_order_acquire); }
    bool hasAnalyzer() const { return m_analyzerCreated.load(std::memory_order_acquire); }

    /**
     * @brief Stop the WDSP channel run flag without waiting (non-blocking).
     *
     * Calling SetChannelState(id, 0, 0) immediately clears the channel run flag
     * without blocking on the internal DSP semaphore. Must be called while DSP
     * worker threads are still alive so in-flight fexchange0 calls can exit cleanly
     * before thread termination.
     */
    void stopChannel();

    /**
     * @brief Set channel run state.
     * @param run 1 to start, 0 to stop
     * @param wait 1 to block until channel semaphore releases, 0 for immediate return
     */
    void setChannelState(int run, int wait = 0);
    static void setChannelStateById(int channelId, int run, int wait = 0);

    /**
     * @brief Close channel and release all WDSP resources. Idempotent and thread-safe.
     */
    virtual void close() = 0;

    /**
     * @brief Global mutex serializing FFTW operations (OpenChannel, CloseChannel, XCreateAnalyzer, DestroyAnalyzer).
     */
    static QRecursiveMutex& wisdomMutex() { return s_wdspWisdomMutex; }

protected:
    const int m_channelId;
    std::atomic<bool> m_isOpen{false};
    std::atomic<bool> m_isRunning{false};
    std::atomic<bool> m_analyzerCreated{false};

    mutable QRecursiveMutex m_channelMutex;
    static QRecursiveMutex s_wdspWisdomMutex;
};

#endif // WDSP_CHANNEL_H
