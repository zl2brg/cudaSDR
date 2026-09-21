#include "AudioEngine/RttyBayesianDecoder.h"
#include "AudioEngine/RttyAutoClassifier.h"
#include "AudioEngine/RttyBaudot.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

const char *baudotDebugLabel(quint8 code, QChar ch)
{
    code &= 0x1F;
    if (code == 0x1B)
        return "FIGS";
    if (code == 0x1F)
        return "LTRS";
    if (code == 0x00)
        return "NUL";
    if (code == 0x02)
        return "LF";
    if (code == 0x08)
        return "CR";
    if (ch == QLatin1Char(' '))
        return "SP";
    if (ch != QChar('\0') && ch.isPrint()) {
        static char buf[2];
        buf[0] = ch.toLatin1();
        buf[1] = '\0';
        return buf;
    }
    return "?";
}

} // namespace

// Empirical English / Amateur RTTY unigram character priors
// Indexed by 5-bit Baudot code (0x00 to 0x1F)
[[maybe_unused]] static const float DEFAULT_RTTY_PRIORS[32] = {
    0.002f, // 0x00: Null / Blank
    0.085f, // 0x01: E / 3
    0.020f, // 0x02: LF
    0.055f, // 0x03: A / -
    0.140f, // 0x04: SPACE
    0.045f, // 0x05: S / '
    0.050f, // 0x06: I / 8
    0.025f, // 0x07: U / 7
    0.020f, // 0x08: CR
    0.030f, // 0x09: D / $
    0.045f, // 0x0A: R / 4
    0.010f, // 0x0B: J / '
    0.045f, // 0x0C: N / ,
    0.015f, // 0x0D: F / !
    0.020f, // 0x0E: C / :
    0.012f, // 0x0F: K / (
    0.065f, // 0x10: T / 5
    0.005f, // 0x11: Z / +
    0.030f, // 0x12: L / )
    0.015f, // 0x13: W / 2
    0.035f, // 0x14: H / #
    0.015f, // 0x15: Y / 6
    0.018f, // 0x16: P / 0
    0.008f, // 0x17: Q / 1
    0.050f, // 0x18: O / 9
    0.012f, // 0x19: B / ?
    0.015f, // 0x1A: G / &
    0.025f, // 0x1B: FIGS shift
    0.020f, // 0x1C: M / .
    0.005f, // 0x1D: X / /
    0.008f, // 0x1E: V / ;
    0.030f  // 0x1F: LTRS shift
};

RttyBayesianDecoder::RttyBayesianDecoder(int rxId, QObject *parent)
    : QObject(parent)
    , m_rxId(rxId)
{
    reset();
}

void RttyBayesianDecoder::clearText() {
    m_recentText.clear();
    emit textUpdated(m_rxId, m_recentText);
}

void RttyBayesianDecoder::resetFraming() {
    m_figsBelief = 0.0f; // Start in LTRS mode
    m_framingLocked = false;
    m_lastFramingConfidence = 0.0f;
    m_lockCounter = 0;
    m_consecutiveMissCount = 0;
    m_activeHypotheses.clear();
    m_totalSymbols = 0;
    m_lastAcceptedEndIdx = -100;
}

void RttyBayesianDecoder::reset() {
    m_recentText.clear();
    resetFraming();
}

void RttyBayesianDecoder::processSymbol(const RttySoftSymbol &sym) {
    if (!m_enabled) return;
    processSoftSymbol(sym.llr, sym.snrDb);
}

void RttyBayesianDecoder::processSymbols(const QVector<RttySoftSymbol> &symbols) {
    if (!m_enabled) return;
    for (const auto &sym : symbols) {
        processSoftSymbol(sym.llr, sym.snrDb);
    }
}

void RttyBayesianDecoder::processSoftSymbol(float llr, float snrDb) {
    ++m_totalSymbols;

    // Advance existing active hypotheses
    for (auto &hyp : m_activeHypotheses) {
        if (hyp.bitCount < 5) {
            hyp.dataLlrs[hyp.bitCount] = llr;
            ++hyp.bitCount;
        } else if (hyp.bitCount == 5) {
            hyp.stopLlr = llr;
            ++hyp.bitCount; // now 6: completed frame (Start + 5 data + Stop)
        }
    }

    bool completedThisSymbol = false;
    for (const auto &hyp : m_activeHypotheses) {
        if (hyp.bitCount == 6)
            completedThisSymbol = true;
    }

    // Evaluate completed hypotheses (bitCount == 6)
    int bestIdx = -1;
    float bestScore = -1e9f;
    BayesianCharResult bestResult;

    for (int i = 0; i < m_activeHypotheses.size(); ++i) {
        const auto &hyp = m_activeHypotheses[i];
        if (hyp.bitCount == 6) {
            const bool isFigs = (m_figsBelief > 0.5f);
            BayesianCharResult res = evaluateBaudotPosterior(hyp.dataLlrs, isFigs);
            const float framingMetric = (-hyp.startLlr) + hyp.stopLlr;
            res.framingConfidence = framingMetric;

            const quint8 code = res.baudotCode & 0x1F;
            const bool isShift = (code == 0x1B || code == 0x1F);
            // Real starts in on-air logs are << -8. A −1 dip into idle Mark
            // plus five Mark bits is a fake LTRS/M/CR.
            const bool startValid = hyp.startLlr < -3.0f;
            // Stop must be Mark. A slightly-negative stop is the next start bit.
            const bool stopValid = hyp.stopLlr > 0.0f;
            // FIGS/LTRS need a Mark stop long enough to be idle, not a −1 dip.
            // Start can be moderate: on-air LTRS often has start≈−5 and stop≫+20.
            const bool shiftOk = !isShift
                || (hyp.stopLlr >= 2.0f && framingMetric >= 10.0f);

            if (startValid && stopValid && shiftOk
                && res.confidence >= m_squelchThreshold
                && framingMetric >= m_framingThreshold) {
                const float compositeScore = framingMetric + 5.0f * res.confidence;
                if (compositeScore > bestScore) {
                    bestScore = compositeScore;
                    bestIdx = i;
                    bestResult = res;
                }
            }
        }
    }

    if (bestIdx >= 0) {
        if (m_autoClassifier) {
            m_autoClassifier->feedFramingResult(m_activeHypotheses[bestIdx].stopLlr, bestResult.confidence);
        }
        m_lastAcceptedEndIdx = m_totalSymbols;
        m_lastFramingConfidence = bestResult.framingConfidence;
        m_consecutiveMissCount = 0;

        if (!m_framingLocked) {
            ++m_lockCounter;
            if (m_lockCounter >= 2) {
                m_framingLocked = true;
                emit framingStateChanged(m_rxId, true, bestResult.framingConfidence);
            }
        }

        if (bestResult.confidence >= 0.70f
            || (bestResult.baudotCode != 0x1B && bestResult.baudotCode != 0x1F)) {
            updateHMMShift(bestResult.baudotCode);
        }

        if (bestResult.character != '\0') {
            const QString charStr(bestResult.character);
            m_recentText.append(charStr);
            if (m_recentText.size() > 500) {
                m_recentText = m_recentText.right(400);
            }
            emit characterDecoded(m_rxId, charStr, bestResult.confidence, bestResult.errorProbability);
            emit characterResultDecoded(m_rxId, bestResult);
            emit textUpdated(m_rxId, m_recentText);
        }

        if (qEnvironmentVariableIsSet("CUDASDR_RTTY_DEBUG")) {
            qDebug("[RTTY rx%d DECODER] ACCEPTED %s code=0x%02X conf=%.2f start=%+.1f stop=%+.1f metric=%.1f figs=%.0f lock=%d",
                   m_rxId, baudotDebugLabel(bestResult.baudotCode, bestResult.character),
                   bestResult.baudotCode, bestResult.confidence,
                   m_activeHypotheses[bestIdx].startLlr, m_activeHypotheses[bestIdx].stopLlr,
                   bestResult.framingConfidence, m_figsBelief, m_framingLocked);
        }

        // The accepted frame is finished; clear hypotheses so the next symbol starts fresh
        m_activeHypotheses.clear();
    } else {
        // Prune completed hypotheses (bitCount >= 6) that were not accepted
        bool hadValidCandidate = false;
        QVector<FrameHypothesis> remaining;
        for (const auto &hyp : m_activeHypotheses) {
            if (hyp.bitCount < 6) {
                remaining.append(hyp);
            } else {
                // Only count as a genuine missed frame if the channel SNR was sufficient
                // for reception (> 0.8 dB) AND the hypothesis began with a strong Space
                // start bit (< -2.5). Atmospheric noise during deep fades (SNR <= 0.8 dB)
                // must NOT drop framing lock (Flywheel effect).
                if (snrDb > 0.8f && hyp.startLlr < -2.5f) {
                    hadValidCandidate = true;
                }
            }
        }
        if (hadValidCandidate && m_framingLocked) {
            // A locked frame with sufficient SNR failed to pass thresholds
            ++m_consecutiveMissCount;
            if (qEnvironmentVariableIsSet("CUDASDR_RTTY_DEBUG")) {
                for (const auto &hyp : m_activeHypotheses) {
                    if (hyp.bitCount >= 6) {
                        const bool isFigs = (m_figsBelief > 0.5f);
                        BayesianCharResult res = evaluateBaudotPosterior(hyp.dataLlrs, isFigs);
                        const float framingMetric = (-hyp.startLlr) + hyp.stopLlr;
                        qDebug("[RTTY rx%d DECODER] Frame MISSED (%d/8) cand=%s code=0x%02X conf=%.2f start=%+.1f stop=%+.1f metric=%.1f",
                               m_rxId, m_consecutiveMissCount,
                               baudotDebugLabel(res.baudotCode, res.character),
                               res.baudotCode, res.confidence, hyp.startLlr, hyp.stopLlr, framingMetric);
                    }
                }
            }
            if (m_consecutiveMissCount >= 8) {
                m_framingLocked = false;
                m_lockCounter = 0;
                remaining.clear();
                emit framingStateChanged(m_rxId, false, 0.0f);
            }
        }
        m_activeHypotheses = remaining;
    }

    // The demod emits one start+5+stop packet per hunted edge. Never start a
    // new hypothesis on a completed stop — that one-bit "resync" walked the
    // decoder through Space runs (8 misses → unlock) on strong weather signals.
    bool collectingData = false;
    for (const auto &hyp : m_activeHypotheses) {
        if (hyp.bitCount < 6) {
            collectingData = true;
            break;
        }
    }
    const bool canLaunch = !collectingData
        && !completedThisSymbol
        && (m_totalSymbols - m_lastAcceptedEndIdx >= 1)
        && (llr < -3.0f);
    if (canLaunch) {
        FrameHypothesis hyp;
        hyp.startSampleIdx = m_totalSymbols;
        hyp.bitCount = 0;
        hyp.startLlr = llr;
        m_activeHypotheses.append(hyp);
    }

    // Bound memory / active hypotheses
    while (m_activeHypotheses.size() > 12) {
        m_activeHypotheses.removeFirst();
    }
}

void RttyBayesianDecoder::updateHMMShift(quint8 code) {
    code &= 0x1F;

    if (code == 0x1B) {
        // FIGS shift code (11011)
        m_figsBelief = 1.0f;
    } else if (code == 0x1F) {
        // LTRS shift code (11111)
        m_figsBelief = 0.0f;
    } else if (code == 0x04 && m_usosEnabled) {
        // Unshift On Space (USOS)
        m_figsBelief = 0.0f;
    }
}

BayesianCharResult RttyBayesianDecoder::evaluateBaudotPosterior(
    const float dataLlrs[5],
    bool figs,
    const float *customPriors)
{
    BayesianCharResult result;
    for (int k = 0; k < 5; ++k) {
        result.bitLlrs[k] = dataLlrs[k];
    }
    result.isFigs = figs;

    float scores[32];
    float maxScore = -1e9f;

    for (quint8 c = 0; c < 32; ++c) {
        // Dot product with binary sign vector s_i in {-1, +1}
        float dot = 0.0f;
        for (int bit = 0; bit < 5; ++bit) {
            const float s = ((c >> bit) & 1) ? 1.0f : -1.0f;
            dot += 0.5f * s * dataLlrs[bit];
        }

        float priorLog = 0.0f;
        if (customPriors != nullptr) {
            const float p = std::max(1e-6f, customPriors[c]);
            priorLog = std::log(p);
        } else {
            // Unbiased prior across all 31 valid ITA2 symbols (letters/figures/callsigns).
            // Mildly penalize null/blank code (0x00) which is not a valid text character.
            priorLog = (c == 0x00) ? -2.5f : 0.0f;
        }

        scores[c] = dot + priorLog;
        if (scores[c] > maxScore) {
            maxScore = scores[c];
        }
    }

    // Softmax over 32 codes
    float sumExp = 0.0f;
    float probs[32];
    for (int c = 0; c < 32; ++c) {
        probs[c] = std::exp(scores[c] - maxScore);
        sumExp += probs[c];
    }

    quint8 topCode = 0;
    float topProb = -1.0f;
    float secondScore = -1e9f;

    for (int c = 0; c < 32; ++c) {
        probs[c] /= (sumExp > 1e-12f) ? sumExp : 1.0f;
        if (probs[c] > topProb) {
            topProb = probs[c];
            topCode = static_cast<quint8>(c);
        }
    }

    // Find 2nd best score for margin calculation
    for (int c = 0; c < 32; ++c) {
        if (c != topCode && scores[c] > secondScore) {
            secondScore = scores[c];
        }
    }

    result.baudotCode = topCode;
    result.confidence = std::clamp(topProb, 0.0f, 1.0f);
    result.errorProbability = 1.0f - result.confidence;
    result.logLikelihoodMargin = maxScore - secondScore;

    result.character = RttyBaudot::decode(topCode, figs);

    return result;
}
