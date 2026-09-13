/**
 * @file  SpscRingBuffer.h
 * @brief Lock-free, zero-allocation Single-Producer Single-Consumer (SPSC) ring buffer.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#ifndef SPSC_RING_BUFFER_H
#define SPSC_RING_BUFFER_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <type_traits>
#include <vector>

// Cache line size for padding/alignment to prevent false sharing
constexpr size_t kCacheLineSize = 64;

/**
 * @brief Lock-free Single-Producer Single-Consumer (SPSC) circular FIFO ring buffer.
 *
 * Guarantees wait-free O(1) concurrent push and pop between one producer thread
 * and one consumer thread without mutexes or dynamic memory allocation.
 *
 * @tparam T Element type (must be trivially copyable).
 */
template<typename T>
class SpscRingBuffer {
    static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable for SpscRingBuffer");

public:
    explicit SpscRingBuffer(size_t minCapacity = 65536)
    {
        // Round up to nearest power of 2 (at least 2)
        m_capacity = 2;
        while (m_capacity < minCapacity) {
            m_capacity <<= 1;
        }
        m_mask = m_capacity - 1;
        m_buffer = new T[m_capacity];
        m_writeIndex.store(0, std::memory_order_relaxed);
        m_readIndex.store(0, std::memory_order_relaxed);
    }

    ~SpscRingBuffer()
    {
        delete[] m_buffer;
    }

    SpscRingBuffer(const SpscRingBuffer&) = delete;
    SpscRingBuffer& operator=(const SpscRingBuffer&) = delete;

    size_t capacity() const noexcept { return m_capacity; }

    /** Number of elements currently ready to read (consumer thread). */
    size_t availableRead() const noexcept
    {
        const size_t w = m_writeIndex.load(std::memory_order_acquire);
        const size_t r = m_readIndex.load(std::memory_order_relaxed);
        const size_t diff = w - r;
        return (diff > m_capacity) ? m_capacity : diff;
    }

    /** Number of slots available for writing (producer thread). */
    size_t availableWrite() const noexcept
    {
        const size_t w = m_writeIndex.load(std::memory_order_relaxed);
        const size_t r = m_readIndex.load(std::memory_order_acquire);
        const size_t diff = w - r;
        return (diff < m_capacity) ? (m_capacity - diff) : 0;
    }

    bool isEmpty() const noexcept
    {
        return availableRead() == 0;
    }

    bool isFull() const noexcept
    {
        return availableWrite() == 0;
    }

    /**
     * @brief Writes up to count elements into the ring buffer.
     * @return Number of elements written (0 if full).
     */
    size_t write(const T* data, size_t count) noexcept
    {
        if (!data || count == 0)
            return 0;

        const size_t w = m_writeIndex.load(std::memory_order_relaxed);
        const size_t r = m_readIndex.load(std::memory_order_acquire);
        const size_t diff = w - r;
        const size_t avail = (diff < m_capacity) ? (m_capacity - diff) : 0;
        const size_t toWrite = std::min(count, avail);

        if (toWrite == 0)
            return 0;

        const size_t idx = w & m_mask;
        const size_t firstChunk = std::min(toWrite, m_capacity - idx);
        std::memcpy(&m_buffer[idx], data, firstChunk * sizeof(T));
        if (toWrite > firstChunk) {
            std::memcpy(&m_buffer[0], data + firstChunk, (toWrite - firstChunk) * sizeof(T));
        }

        m_writeIndex.store(w + toWrite, std::memory_order_release);
        return toWrite;
    }

    /** Number of elements dropped by writeDropOldest when buffer was full. */
    uint64_t dropCount() const noexcept
    {
        return m_dropCount.load(std::memory_order_relaxed);
    }

    void resetDropCount() noexcept
    {
        m_dropCount.store(0, std::memory_order_relaxed);
    }

    /**
     * @brief Writes elements, dropping oldest unread data if buffer space is insufficient.
     * Always writes all 'count' elements (clamped to capacity).
     * Advances the read index so consumers never see overwritten slots.
     */
    size_t writeDropOldest(const T* data, size_t count) noexcept
    {
        if (!data || count == 0)
            return 0;

        const size_t toWrite = std::min(count, m_capacity);
        const T* src = data + (count - toWrite);

        const size_t w = m_writeIndex.load(std::memory_order_relaxed);
        const size_t r = m_readIndex.load(std::memory_order_acquire);
        const size_t currentBuffered = w - r;
        const size_t overflowDrop = (currentBuffered > m_capacity) ? (currentBuffered - m_capacity) : 0;
        const size_t room = m_capacity - std::min(currentBuffered, m_capacity);
        const size_t dropForWrite = (toWrite > room) ? (toWrite - room) : 0;
        const size_t tailDrop = (count > m_capacity) ? (count - m_capacity) : 0;
        const size_t excess = overflowDrop + dropForWrite + tailDrop;
        if (excess > 0) {
            m_dropCount.fetch_add(excess, std::memory_order_relaxed);
        }

        const size_t idx = w & m_mask;
        const size_t firstChunk = std::min(toWrite, m_capacity - idx);
        std::memcpy(&m_buffer[idx], src, firstChunk * sizeof(T));
        if (toWrite > firstChunk) {
            std::memcpy(&m_buffer[0], src + firstChunk, (toWrite - firstChunk) * sizeof(T));
        }

        const size_t newW = w + toWrite;
        m_writeIndex.store(newW, std::memory_order_release);
        if (newW - r > m_capacity)
            storeReadIndexAtLeast(newW - m_capacity);
        return toWrite;
    }

    /**
     * @brief Reads up to maxCount elements from the ring buffer.
     * @return Number of elements actually read.
     */
    size_t read(T* dest, size_t maxCount) noexcept
    {
        if (!dest || maxCount == 0)
            return 0;

        const size_t w = m_writeIndex.load(std::memory_order_acquire);
        const size_t r = effectiveReadIndex(w);
        const size_t diff = w - r;
        const size_t toRead = std::min(maxCount, diff);
        if (toRead == 0)
            return 0;

        const size_t idx = r & m_mask;
        const size_t firstChunk = std::min(toRead, m_capacity - idx);
        std::memcpy(dest, &m_buffer[idx], firstChunk * sizeof(T));
        if (toRead > firstChunk) {
            std::memcpy(dest + firstChunk, &m_buffer[0], (toRead - firstChunk) * sizeof(T));
        }

        storeReadIndexAtLeast(r + toRead);
        return toRead;
    }

    /**
     * @brief Peeks at the next contiguous chunk of readable data without advancing read index.
     * Overrun is recovered locally; this does not mutate m_readIndex.
     * Pair with advanceRead(), which heals then advances.
     */
    const T* peekContiguous(size_t& contiguousCount) noexcept
    {
        const size_t w = m_writeIndex.load(std::memory_order_acquire);
        const size_t r = effectiveReadIndex(w);
        const size_t diff = w - r;

        if (diff == 0) {
            contiguousCount = 0;
            return nullptr;
        }

        const size_t idx = r & m_mask;
        contiguousCount = std::min(diff, m_capacity - idx);
        return &m_buffer[idx];
    }

    /**
     * @brief Advances read pointer by count (e.g. after peekContiguous).
     */
    void advanceRead(size_t count) noexcept
    {
        if (count == 0)
            return;
        const size_t w = m_writeIndex.load(std::memory_order_acquire);
        const size_t r = effectiveReadIndex(w);
        storeReadIndexAtLeast(r + count);
    }

    /** Clears all contents by synchronizing read index with write index. */
    void clear() noexcept
    {
        const size_t w = m_writeIndex.load(std::memory_order_relaxed);
        m_readIndex.store(w, std::memory_order_release);
    }

private:
    size_t effectiveReadIndex(size_t writeIndex) const noexcept
    {
        const size_t r = m_readIndex.load(std::memory_order_relaxed);
        const size_t diff = writeIndex - r;
        return (diff > m_capacity) ? (writeIndex - m_capacity) : r;
    }

    void storeReadIndexAtLeast(size_t minRead) noexcept
    {
        size_t cur = m_readIndex.load(std::memory_order_relaxed);
        while (cur < minRead) {
            if (m_readIndex.compare_exchange_weak(cur, minRead,
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed)) {
                return;
            }
        }
    }

    T* m_buffer = nullptr;
    size_t m_capacity = 0;
    size_t m_mask = 0;

    alignas(kCacheLineSize) std::atomic<size_t> m_writeIndex{0};
    alignas(kCacheLineSize) std::atomic<size_t> m_readIndex{0};
    alignas(kCacheLineSize) std::atomic<uint64_t> m_dropCount{0};
};

/**
 * @brief Accumulates from a ring until a full block is available.
 * Returns true and copies `count` elements to dest; otherwise false and
 * keeps partial data in residual for the next call.
 */
template<typename T>
inline bool spscReadBlock(SpscRingBuffer<T>& ring, std::vector<T>& residual, T* dest, size_t count)
{
    if (!dest || count == 0)
        return false;

    while (residual.size() < count) {
        const size_t need = count - residual.size();
        const size_t old = residual.size();
        residual.resize(old + need);
        const size_t got = ring.read(residual.data() + old, need);
        residual.resize(old + got);
        if (got == 0)
            break;
    }
    if (residual.size() < count)
        return false;

    std::memcpy(dest, residual.data(), count * sizeof(T));
    residual.erase(residual.begin(), residual.begin() + static_cast<std::ptrdiff_t>(count));
    return true;
}

#endif // SPSC_RING_BUFFER_H
