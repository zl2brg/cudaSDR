/**
* @file  cusdr_queue.h
* @brief queue header file for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2012-06-13
*/

/*
 *   adaptation from a code example from QtCentre: http://www.qtcentre.org/threads/26250-Thread-Safe-Queue-container...
 *   Copyright 2012 Hermann von Hasseln, DL3HVH
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CUSDR_QUEUE_H
#define CUSDR_QUEUE_H

#include <QQueue>
#include <QSemaphore>
#include <QMutex>

template<class T> class QHQueue {

public:
	QHQueue(int maxSize = 8192)
		: m_semFree(maxSize)
		, m_semUsed(0)
		, m_maxSize(maxSize)
	{
	}
	
	void enqueue(const T &value) {

	    m_semFree.acquire(1);
		m_mutex.lock();
		m_queue.enqueue(value);
		m_mutex.unlock();
		m_semUsed.release(1);
	}

	/** Non-blocking enqueue. Returns false if the queue is full. */
	bool tryEnqueue(const T &value) {
		if (!m_semFree.tryAcquire(1))
			return false;
		m_mutex.lock();
		m_queue.enqueue(value);
		m_mutex.unlock();
		m_semUsed.release(1);
		return true;
	}

	/** Enqueue, dropping the oldest item if full (never blocks). */
	void enqueueDropOldest(const T &value) {
		if (tryEnqueue(value))
			return;
		// Drop one oldest slot, then push the new value.
		if (m_semUsed.tryAcquire(1)) {
			m_mutex.lock();
			if (!m_queue.isEmpty())
				m_queue.dequeue();
			m_mutex.unlock();
			m_semFree.release(1);
		}
		tryEnqueue(value);
	}

    T dequeue() {
    
		m_semUsed.acquire(1);
        m_mutex.lock();
        T val = m_queue.dequeue();
        m_mutex.unlock();
        m_semFree.release(1);
        return val;
    }

    T head() {

        m_semUsed.acquire(1);
        m_mutex.lock();
        T val = m_queue.head();
        m_mutex.unlock();
        m_semUsed.release(1);
		return val;
    }

    bool isEmpty() const {

        return m_semUsed.available() == 0;
    }

    bool isFull() const {

        return m_semFree.available() == 0;
    }

    int count() const {

        return m_semUsed.available();
    }

	int maxSize() const {
		return m_maxSize;
	}

	void release_queue() {

		m_semUsed.release(1);
	}

    T tryHead() {

        bool t = m_semUsed.tryAcquire(1);
        if (!t)
            return T();

        m_mutex.lock();
			T val = m_queue.head();
        m_mutex.unlock();
        m_semUsed.release(1);

        return val;
    }

    T tryDequeue() {

        bool t = m_semUsed.tryAcquire(1);
        if (!t)
            return T();

        m_mutex.lock();
			T val = m_queue.dequeue();
        m_mutex.unlock();
        m_semFree.release(1);

        return val;
    }

	/** Discard all queued items without blocking. */
	void clear() {
		while (m_semUsed.tryAcquire(1)) {
			m_mutex.lock();
			if (!m_queue.isEmpty())
				m_queue.dequeue();
			m_mutex.unlock();
			m_semFree.release(1);
		}
	}

private:
    QQueue<T>	m_queue;
    QSemaphore	m_semFree;
    QSemaphore	m_semUsed;
    QMutex		m_mutex;
	int			m_maxSize;
};

#endif // CUSDR_QUEUE_H