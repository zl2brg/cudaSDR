/* Copyright (C)
 *
 * Simon Eatough ZL2BRG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <QtTest/QtTest>
#include <memory>
#include <vector>
#include <cmath>

#include "AudioEngine/IDigitalVoiceDemodulator.h"
#include "AudioEngine/cusdr_freedvprocessor.h"
#ifdef HAVE_RADE
#include "AudioEngine/cusdr_radeprocessor.h"
#endif

namespace {

// Test mock implementing IDigitalVoiceDemodulator for contract verification
class MockDigitalVoiceDemodulator : public IDigitalVoiceDemodulator {
public:
    explicit MockDigitalVoiceDemodulator(int modeId = 1)
        : m_modeId(modeId)
        , m_sync(false)
        , m_snr(0.0f)
        , m_totalSamplesProcessed(0)
    {
    }

    ~MockDigitalVoiceDemodulator() override {
        s_destructorCallCount++;
    }

    QVector<float> processSamples(const float* audio48k, int n) override {
        if (!audio48k || n <= 0) return {};

        m_totalSamplesProcessed += n;
        m_callCount++;

        // Simulate sync after receiving at least 960 samples
        if (m_totalSamplesProcessed >= 960) {
            m_sync = true;
            m_snr = 12.5f;
        }

        // Return stereo float output (2x input size)
        QVector<float> output(n * 2, 0.0f);
        for (int i = 0; i < n; ++i) {
            output[i * 2] = audio48k[i] * 0.5f;
            output[i * 2 + 1] = audio48k[i] * 0.5f;
        }
        return output;
    }

    bool isSync() const override { return m_sync; }
    float getSNR() const override { return m_snr; }

    int modeId() const { return m_modeId; }
    int callCount() const { return m_callCount; }
    int totalSamplesProcessed() const { return m_totalSamplesProcessed; }

    static int s_destructorCallCount;

private:
    int m_modeId;
    bool m_sync;
    float m_snr;
    int m_totalSamplesProcessed;
    int m_callCount = 0;
};

int MockDigitalVoiceDemodulator::s_destructorCallCount = 0;

} // anonymous namespace

class DigitalVoiceTests : public QObject {
    Q_OBJECT

private slots:
    void init();
    void testMockInterfaceContract();
    void testPolymorphicOwnershipAndReset();
    void testBoundaryAndNullInputs();
    void testModeSwitchingLifecycle();
    void testFreeDVProcessorLifecycle();
    void testFreeDVProcessorSilenceProcessing();
#ifdef HAVE_RADE
    void testRadeProcessorLifecycle();
    void testRadeProcessorSilenceProcessing();
    void testHeterogeneousDemodulatorSwitching();
#endif
};

void DigitalVoiceTests::init()
{
    MockDigitalVoiceDemodulator::s_destructorCallCount = 0;
}

void DigitalVoiceTests::testMockInterfaceContract()
{
    std::unique_ptr<IDigitalVoiceDemodulator> demod = std::make_unique<MockDigitalVoiceDemodulator>(42);

    QVERIFY(demod != nullptr);
    QCOMPARE(demod->isSync(), false);
    QCOMPARE(demod->getSNR(), 0.0f);

    // Feed a block of 480 samples
    std::vector<float> audioIn(480, 0.25f);
    QVector<float> speech = demod->processSamples(audioIn.data(), static_cast<int>(audioIn.size()));

    QCOMPARE(speech.size(), 960); // 480 * 2 stereo
    QCOMPARE(demod->isSync(), false); // Not reached 960 yet

    // Feed another 480 samples
    speech = demod->processSamples(audioIn.data(), static_cast<int>(audioIn.size()));
    QCOMPARE(speech.size(), 960);
    QCOMPARE(demod->isSync(), true); // Reached 960 total
    QCOMPARE(demod->getSNR(), 12.5f);
}

void DigitalVoiceTests::testPolymorphicOwnershipAndReset()
{
    QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 0);

    {
        std::unique_ptr<IDigitalVoiceDemodulator> demod = std::make_unique<MockDigitalVoiceDemodulator>(1);
        QVERIFY(demod != nullptr);

        // Reset replaces with a new instance
        demod = std::make_unique<MockDigitalVoiceDemodulator>(2);
        QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 1);

        // Reset to null
        demod.reset();
        QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 2);
    }

    // Scope exit with null does not increment again
    QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 2);
}

void DigitalVoiceTests::testBoundaryAndNullInputs()
{
    std::unique_ptr<IDigitalVoiceDemodulator> demod = std::make_unique<MockDigitalVoiceDemodulator>(1);

    // Null pointer
    QVector<float> out = demod->processSamples(nullptr, 100);
    QVERIFY(out.isEmpty());

    // Zero count
    float dummy = 1.0f;
    out = demod->processSamples(&dummy, 0);
    QVERIFY(out.isEmpty());

    // Negative count
    out = demod->processSamples(&dummy, -10);
    QVERIFY(out.isEmpty());
}

void DigitalVoiceTests::testModeSwitchingLifecycle()
{
    // Emulates SliceProcessor::setFreeDVMode switching
    std::unique_ptr<IDigitalVoiceDemodulator> demod;
    int currentMode = -1;

    auto setMode = [&](int newMode) {
        if (currentMode == newMode && demod) return;
        currentMode = newMode;
        demod.reset();
        demod = std::make_unique<MockDigitalVoiceDemodulator>(newMode);
    };

    setMode(0);
    QVERIFY(demod != nullptr);
    QCOMPARE(currentMode, 0);
    QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 0);

    // Duplicate call is a no-op
    setMode(0);
    QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 0);

    // Switch to mode 1
    setMode(1);
    QCOMPARE(currentMode, 1);
    QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 1);

    // Switch to mode 2
    setMode(2);
    QCOMPARE(currentMode, 2);
    QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 2);

    demod.reset();
    QCOMPARE(MockDigitalVoiceDemodulator::s_destructorCallCount, 3);
}

void DigitalVoiceTests::testFreeDVProcessorLifecycle()
{
#ifdef HAVE_CODEC2
    // Create FreeDV demodulator in default mode (0 = 1600)
    std::unique_ptr<IDigitalVoiceDemodulator> demod = std::make_unique<FreeDVProcessor>(0);
    QVERIFY(demod != nullptr);
    QCOMPARE(demod->isSync(), false);
    QCOMPARE(demod->getSNR(), 0.0f);

    // Destruction is verified on scope exit
#else
    QSKIP("Codec2 not enabled; skipping FreeDVProcessorLifecycle test");
#endif
}

void DigitalVoiceTests::testFreeDVProcessorSilenceProcessing()
{
#ifdef HAVE_CODEC2
    std::unique_ptr<IDigitalVoiceDemodulator> demod = std::make_unique<FreeDVProcessor>(0);
    QVERIFY(demod != nullptr);

    // Feed 1920 samples (40ms @ 48kHz) of silence
    std::vector<float> silence(1920, 0.0f);
    QVector<float> speech = demod->processSamples(silence.data(), static_cast<int>(silence.size()));

    // In silent noise, demodulator will not sync
    QCOMPARE(demod->isSync(), false);

    // Even if no frames decoded yet, it should not crash and output must be valid stereo (even length)
    QVERIFY(speech.size() % 2 == 0);

    // Feed another 1920 samples
    speech = demod->processSamples(silence.data(), static_cast<int>(silence.size()));
    QVERIFY(speech.size() % 2 == 0);
#else
    QSKIP("Codec2 not enabled; skipping FreeDVProcessorSilenceProcessing test");
#endif
}

#ifdef HAVE_RADE
void DigitalVoiceTests::testRadeProcessorLifecycle()
{
    std::unique_ptr<IDigitalVoiceDemodulator> demod = std::make_unique<RadeProcessor>();
    QVERIFY(demod != nullptr);
    QCOMPARE(demod->isSync(), false);
    QCOMPARE(demod->getSNR(), 0.0f);
}

void DigitalVoiceTests::testRadeProcessorSilenceProcessing()
{
    std::unique_ptr<IDigitalVoiceDemodulator> demod = std::make_unique<RadeProcessor>();
    QVERIFY(demod != nullptr);

    // Feed 1920 samples of silence
    std::vector<float> silence(1920, 0.0f);
    QVector<float> speech = demod->processSamples(silence.data(), static_cast<int>(silence.size()));

    QCOMPARE(demod->isSync(), false);
    QVERIFY(speech.size() % 2 == 0);
}

void DigitalVoiceTests::testHeterogeneousDemodulatorSwitching()
{
    // Emulates SliceProcessor dynamic switching between FreeDV and RADE via IDigitalVoiceDemodulator
    std::unique_ptr<IDigitalVoiceDemodulator> demod;
    int mode = -1;

    auto setSliceDVMode = [&](int newMode) {
        if (mode == newMode && demod) return;
        mode = newMode;
        demod.reset();
        if (mode == 100) {
            demod = std::make_unique<RadeProcessor>();
        } else {
            demod = std::make_unique<FreeDVProcessor>(mode);
        }
    };

    // Switch to FreeDV 1600 (mode 0)
    setSliceDVMode(0);
    QVERIFY(demod != nullptr);
    QCOMPARE(demod->isSync(), false);

    // Switch to RADE (mode 100)
    setSliceDVMode(100);
    QVERIFY(demod != nullptr);
    QCOMPARE(demod->isSync(), false);

    // Feed silence to RADE through base interface
    std::vector<float> silence(960, 0.0f);
    QVector<float> speech = demod->processSamples(silence.data(), static_cast<int>(silence.size()));
    QVERIFY(speech.size() % 2 == 0);

    // Switch back to FreeDV
    setSliceDVMode(0);
    QVERIFY(demod != nullptr);
    speech = demod->processSamples(silence.data(), static_cast<int>(silence.size()));
    QVERIFY(speech.size() % 2 == 0);

    demod.reset();
    QVERIFY(demod == nullptr);
}
#endif

QTEST_MAIN(DigitalVoiceTests)
#include "digital_voice_tests.moc"
