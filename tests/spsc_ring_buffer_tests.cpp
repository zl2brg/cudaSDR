/**
 * @file  spsc_ring_buffer_tests.cpp
 * @brief Comprehensive unit tests for SpscRingBuffer and ReceiverAudioOutput ring buffer.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#include <QtTest/QtTest>
#include <thread>
#include <vector>
#include <numeric>

#include "Util/SpscRingBuffer.h"
#include "DataEngine/receiveraudiooutput.h"

class SpscRingBufferTests : public QObject {
    Q_OBJECT

private slots:
    void testCapacityPowerOfTwo();
    void testBasicWriteRead();
    void testWrapAroundWriteRead();
    void testWriteFullRejection();
    void testWriteDropOldest();
    void testWriteDropOldestPeekWithoutConsumerHeal();
    void testSpscReadBlock();
    void testPeekContiguousAndAdvance();
    void testClear();
    void testConcurrentProducerConsumerStress();
    void testReceiverAudioOutputIntegration();
};

void SpscRingBufferTests::testCapacityPowerOfTwo()
{
    SpscRingBuffer<int> buf0(0);
    QCOMPARE(buf0.capacity(), size_t(2));

    SpscRingBuffer<int> buf1(1);
    QCOMPARE(buf1.capacity(), size_t(2));

    SpscRingBuffer<int> buf7(7);
    QCOMPARE(buf7.capacity(), size_t(8));

    SpscRingBuffer<int> buf1024(1024);
    QCOMPARE(buf1024.capacity(), size_t(1024));

    SpscRingBuffer<int> buf1025(1025);
    QCOMPARE(buf1025.capacity(), size_t(2048));

    QVERIFY(buf1024.isEmpty());
    QVERIFY(!buf1024.isFull());
    QCOMPARE(buf1024.availableRead(), size_t(0));
    QCOMPARE(buf1024.availableWrite(), size_t(1024));
}

void SpscRingBufferTests::testBasicWriteRead()
{
    SpscRingBuffer<float> buf(16);
    QCOMPARE(buf.capacity(), size_t(16));

    float writeData[5] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
    size_t written = buf.write(writeData, 5);
    QCOMPARE(written, size_t(5));
    QCOMPARE(buf.availableRead(), size_t(5));
    QCOMPARE(buf.availableWrite(), size_t(11));
    QVERIFY(!buf.isEmpty());
    QVERIFY(!buf.isFull());

    float readData[8] = {0};
    size_t read = buf.read(readData, 8);
    QCOMPARE(read, size_t(5));
    QCOMPARE(buf.availableRead(), size_t(0));
    QCOMPARE(buf.availableWrite(), size_t(16));
    QVERIFY(buf.isEmpty());

    for (int i = 0; i < 5; ++i) {
        QCOMPARE(readData[i], writeData[i]);
    }
}

void SpscRingBufferTests::testWrapAroundWriteRead()
{
    // Capacity 8
    SpscRingBuffer<int> buf(8);
    QCOMPARE(buf.capacity(), size_t(8));

    // Cycle through 100 iterations, writing 5 items and reading 5 items each time
    for (int iter = 0; iter < 100; ++iter) {
        int in[5];
        for (int i = 0; i < 5; ++i) {
            in[i] = iter * 10 + i;
        }

        size_t written = buf.write(in, 5);
        QCOMPARE(written, size_t(5));

        int out[5] = {0};
        size_t read = buf.read(out, 5);
        QCOMPARE(read, size_t(5));

        for (int i = 0; i < 5; ++i) {
            QCOMPARE(out[i], in[i]);
        }
        QVERIFY(buf.isEmpty());
    }
}

void SpscRingBufferTests::testWriteFullRejection()
{
    SpscRingBuffer<int> buf(8);
    int in[8] = {10, 20, 30, 40, 50, 60, 70, 80};

    size_t written = buf.write(in, 8);
    QCOMPARE(written, size_t(8));
    QVERIFY(buf.isFull());
    QCOMPARE(buf.availableWrite(), size_t(0));

    // Attempting to write more should write 0 items
    int extra[2] = {90, 100};
    size_t extraWritten = buf.write(extra, 2);
    QCOMPARE(extraWritten, size_t(0));

    // Read 3 items
    int out[3] = {0};
    size_t read = buf.read(out, 3);
    QCOMPARE(read, size_t(3));
    QCOMPARE(buf.availableWrite(), size_t(3));

    // Now write 2 items
    extraWritten = buf.write(extra, 2);
    QCOMPARE(extraWritten, size_t(2));
    QCOMPARE(buf.availableRead(), size_t(7));
}

void SpscRingBufferTests::testWriteDropOldest()
{
    // Capacity 16
    SpscRingBuffer<int> buf(16);
    QCOMPARE(buf.capacity(), size_t(16));

    // Write 16 values: 0 to 15
    int initial[16];
    std::iota(std::begin(initial), std::end(initial), 0);
    size_t written = buf.write(initial, 16);
    QCOMPARE(written, size_t(16));
    QVERIFY(buf.isFull());

    // Write 4 more values with writeDropOldest: 16, 17, 18, 19
    int extra[4] = {16, 17, 18, 19};
    size_t extraWritten = buf.writeDropOldest(extra, 4);
    QCOMPARE(extraWritten, size_t(4));
    QCOMPARE(buf.availableRead(), size_t(16));

    // The oldest 4 (0, 1, 2, 3) must have been dropped.
    QCOMPARE(buf.dropCount(), uint64_t(4));
    buf.resetDropCount();
    QCOMPARE(buf.dropCount(), uint64_t(0));

    // We should read values 4 through 19.
    int out[16] = {0};
    size_t read = buf.read(out, 16);
    QCOMPARE(read, size_t(16));

    for (int i = 0; i < 16; ++i) {
        QCOMPARE(out[i], 4 + i);
    }
    QVERIFY(buf.isEmpty());
}

void SpscRingBufferTests::testWriteDropOldestPeekWithoutConsumerHeal()
{
    SpscRingBuffer<int> buf(16);
    int initial[16];
    std::iota(std::begin(initial), std::end(initial), 0);
    QCOMPARE(buf.write(initial, 16), size_t(16));

    int extra[4] = {16, 17, 18, 19};
    QCOMPARE(buf.writeDropOldest(extra, 4), size_t(4));
    QCOMPARE(buf.availableRead(), size_t(16));

    size_t contiguous = 0;
    const int* peek = buf.peekContiguous(contiguous);
    QVERIFY(peek != nullptr);
    QCOMPARE(contiguous, size_t(12));
    QCOMPARE(peek[0], 4);

    int out[16] = {0};
    QCOMPARE(buf.read(out, 16), size_t(16));
    for (int i = 0; i < 16; ++i)
        QCOMPARE(out[i], 4 + i);
}

void SpscRingBufferTests::testSpscReadBlock()
{
    SpscRingBuffer<float> ring(32);
    std::vector<float> residual;
    float block[8];

    float first[5] = {1.f, 2.f, 3.f, 4.f, 5.f};
    QCOMPARE(ring.write(first, 5), size_t(5));
    QVERIFY(!spscReadBlock(ring, residual, block, 8));
    QCOMPARE(residual.size(), size_t(5));

    float second[3] = {6.f, 7.f, 8.f};
    QCOMPARE(ring.write(second, 3), size_t(3));
    QVERIFY(spscReadBlock(ring, residual, block, 8));
    QCOMPARE(residual.size(), size_t(0));
    for (int i = 0; i < 8; ++i)
        QCOMPARE(block[i], static_cast<float>(i + 1));

    QVERIFY(!spscReadBlock(ring, residual, block, 8));
}

void SpscRingBufferTests::testPeekContiguousAndAdvance()
{
    // Capacity 8
    SpscRingBuffer<int> buf(8);

    // Push 6 items: [0, 1, 2, 3, 4, 5]
    int in1[6] = {0, 1, 2, 3, 4, 5};
    buf.write(in1, 6);

    // Read 4 items: consumer index is now 4
    int out[4];
    buf.read(out, 4);
    QCOMPARE(buf.availableRead(), size_t(2));

    // Push 5 items: [6, 7, 8, 9, 10]
    // Indices in ring buffer wrap around:
    // Slot 4: 4, Slot 5: 5, Slot 6: 6, Slot 7: 7, Slot 0: 8, Slot 1: 9, Slot 2: 10
    int in2[5] = {6, 7, 8, 9, 10};
    buf.write(in2, 5);
    QCOMPARE(buf.availableRead(), size_t(7));

    // First contiguous peek should return items from slot 4 to slot 7 (4 items: 4, 5, 6, 7)
    size_t contiguous1 = 0;
    const int* p1 = buf.peekContiguous(contiguous1);
    QVERIFY(p1 != nullptr);
    QCOMPARE(contiguous1, size_t(4));
    QCOMPARE(p1[0], 4);
    QCOMPARE(p1[1], 5);
    QCOMPARE(p1[2], 6);
    QCOMPARE(p1[3], 7);

    // Advance read by 4
    buf.advanceRead(contiguous1);
    QCOMPARE(buf.availableRead(), size_t(3));

    // Second contiguous peek should return items from slot 0 to 2 (3 items: 8, 9, 10)
    size_t contiguous2 = 0;
    const int* p2 = buf.peekContiguous(contiguous2);
    QVERIFY(p2 != nullptr);
    QCOMPARE(contiguous2, size_t(3));
    QCOMPARE(p2[0], 8);
    QCOMPARE(p2[1], 9);
    QCOMPARE(p2[2], 10);

    // Advance read by 3 -> buffer should be empty
    buf.advanceRead(contiguous2);
    QVERIFY(buf.isEmpty());

    // Next peek returns nullptr
    size_t contiguous3 = 99;
    const int* p3 = buf.peekContiguous(contiguous3);
    QCOMPARE(p3, nullptr);
    QCOMPARE(contiguous3, size_t(0));
}

void SpscRingBufferTests::testClear()
{
    SpscRingBuffer<int> buf(16);
    int in[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    buf.write(in, 8);
    QCOMPARE(buf.availableRead(), size_t(8));

    buf.clear();
    QCOMPARE(buf.availableRead(), size_t(0));
    QVERIFY(buf.isEmpty());
    QCOMPARE(buf.availableWrite(), size_t(16));
}

void SpscRingBufferTests::testConcurrentProducerConsumerStress()
{
    constexpr size_t totalItems = 500000;
    SpscRingBuffer<uint32_t> buf(4096);

    std::atomic<bool> producerDone{false};
    std::vector<uint32_t> received;
    received.reserve(totalItems);

    // Producer thread pushes sequential integers 0 .. totalItems - 1
    std::thread producer([&]() {
        uint32_t current = 0;
        constexpr size_t batchSize = 64;
        uint32_t batch[batchSize];

        while (current < totalItems) {
            size_t count = std::min(batchSize, size_t(totalItems - current));
            for (size_t i = 0; i < count; ++i) {
                batch[i] = current + static_cast<uint32_t>(i);
            }

            size_t written = 0;
            while (written < count) {
                size_t n = buf.write(batch + written, count - written);
                written += n;
                if (written < count) {
                    std::this_thread::yield();
                }
            }
            current += static_cast<uint32_t>(count);
        }
        producerDone.store(true, std::memory_order_release);
    });

    // Consumer thread reads batches and appends to received
    constexpr size_t readBatchSize = 128;
    uint32_t readBatch[readBatchSize];

    while (!producerDone.load(std::memory_order_acquire) || !buf.isEmpty()) {
        size_t n = buf.read(readBatch, readBatchSize);
        if (n > 0) {
            received.insert(received.end(), readBatch, readBatch + n);
        } else {
            std::this_thread::yield();
        }
    }

    producer.join();

    // Verify all items received in strictly increasing sequence
    QCOMPARE(received.size(), totalItems);
    for (size_t i = 0; i < totalItems; ++i) {
        if (received[i] != static_cast<uint32_t>(i)) {
            QFAIL(qPrintable(QString("Mismatch at index %1: expected %2, got %3")
                .arg(i).arg(i).arg(received[i])));
        }
    }
}

void SpscRingBufferTests::testReceiverAudioOutputIntegration()
{
    ReceiverAudioOutput audioOutput;
    QCOMPARE(audioOutput.ringBufferCapacity(), size_t(131072));
    QCOMPARE(audioOutput.ringBufferAvailableRead(), size_t(0));

    // When stopped, writeAudio is ignored
    QVector<float> testBlock(1024, 0.5f);
    audioOutput.writeAudio(testBlock);
    QCOMPARE(audioOutput.ringBufferAvailableRead(), size_t(0));
}

QTEST_APPLESS_MAIN(SpscRingBufferTests)
#include "spsc_ring_buffer_tests.moc"
