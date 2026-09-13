/* Copyright (C)
 *
 * Simon Eatough ZL2BRG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef CUDASDR_RX_IQ_INGEST_H
#define CUDASDR_RX_IQ_INGEST_H

#include "DataEngine/ISdrDevice.h"

#include <QMap>
#include <QMutex>
#include <QVector>

#include <algorithm>
#include <cstring>

/**
 * @brief Shared RX IQ ingest for ISdrDevice adapters.
 *
 * Push path: notify() invokes the registered callback and does not buffer.
 * Pull path: with no callback, notify() appends to a bounded per-RX buffer
 *            and read() copies from that buffer without firing the callback.
 */
class RxIqIngest {
public:
    static constexpr int kMaxComplexSamples = 16384;

    void setCallback(ISdrDevice::RxIqCallback callback)
    {
        QMutexLocker locker(&m_mutex);
        m_callback = std::move(callback);
    }

    void notify(int rx, const float* buffer, int count)
    {
        if (!buffer || count <= 0)
            return;

        ISdrDevice::RxIqCallback cb;
        {
            QMutexLocker locker(&m_mutex);
            cb = m_callback;
            if (!cb)
                appendLocked(rx, buffer, count);
        }
        if (cb)
            cb(rx, buffer, count);
    }

    int read(int rx, float* destination, int maxSamples)
    {
        if (!destination || maxSamples <= 0)
            return 0;

        QMutexLocker locker(&m_mutex);
        auto it = m_buffers.find(rx);
        if (it == m_buffers.end() || it.value().isEmpty())
            return 0;

        auto& buf = it.value();
        const int available = buf.size() / 2;
        const int toCopy = std::min(maxSamples, available);
        std::memcpy(destination, buf.constData(), static_cast<size_t>(toCopy) * 2 * sizeof(float));
        buf.remove(0, toCopy * 2);
        return toCopy;
    }

    void append(int rx, const float* buffer, int count)
    {
        if (!buffer || count <= 0)
            return;
        QMutexLocker locker(&m_mutex);
        appendLocked(rx, buffer, count);
    }

    int bufferedComplexSamples(int rx) const
    {
        QMutexLocker locker(&m_mutex);
        auto it = m_buffers.constFind(rx);
        if (it == m_buffers.cend())
            return 0;
        return it.value().size() / 2;
    }

private:
    void appendLocked(int rx, const float* buffer, int count)
    {
        auto& q = m_buffers[rx];
        const int incoming = count * 2;
        const int maxFloats = kMaxComplexSamples * 2;
        if (q.size() + incoming > maxFloats) {
            const int overflow = q.size() + incoming - maxFloats;
            if (overflow >= q.size())
                q.clear();
            else
                q.remove(0, overflow);
        }
        const int oldSize = q.size();
        q.resize(oldSize + incoming);
        std::memcpy(q.data() + oldSize, buffer, static_cast<size_t>(incoming) * sizeof(float));
    }

    mutable QMutex m_mutex;
    ISdrDevice::RxIqCallback m_callback;
    QMap<int, QVector<float>> m_buffers;
};

#endif // CUDASDR_RX_IQ_INGEST_H
