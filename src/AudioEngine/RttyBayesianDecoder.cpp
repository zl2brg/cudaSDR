#include "AudioEngine/RttyBayesianDecoder.h"

#include <algorithm>
#include <cmath>

// Standard ITA2 tables
static const char LTRS_TABLE[32] = {
    '\0', 'E',  '\n', 'A',  ' ',  'S',  'I',  'U',
    '\r', 'D',  'R',  'J',  'N',  'F',  'C',  'K',
    'T',  'Z',  'L',  'W',  'H',  'Y',  'P',  'Q',
    'O',  'B',  'G',  '\0', 'M',  'X',  'V',  '\0'
};

static const char FIGS_TABLE[32] = {
    '\0', '3',  '\n', '-',  ' ',  '\'', '8',  '7',
    '\r', '$',  '4',  '\'', ',',  '!',  ':',  '(',
    '5',  '+',  ')',  '2',  '#',  '6',  '0',  '1',
    '9',  '?',  '&',  '\0', '.',  '/',  ';',  '\0'
};

// Empirical English / Amateur RTTY unigram character priors
// Indexed by 5-bit Baudot code (0x00 to 0x1F)
static const float DEFAULT_RTTY_PRIORS[32] = {
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

void RttyBayesianDecoder::reset() {
    m_recentText.clear();
    m_figsBelief = 0.0f; // Start in LTRS mode
    m_framingLocked = false;
    m_lastFramingConfidence = 0.0f;
    m_lockCounter = 0;
    m_activeHypotheses.clear();
    m_totalSymbols = 0;
    m_lastAcceptedEndIdx = -100;
}

void RttyBayesianDecoder::processSymbol(const RttySoftSymbol &sym) {
    if (!m_enabled) return;
    processSoftSymbol(sym.llr);
}

void RttyBayesianDecoder::processSymbols(const QVector<RttySoftSymbol> &symbols) {
    if (!m_enabled) return;
    for (const auto &sym : symbols) {
        processSoftSymbol(sym.llr);
    }
}

void RttyBayesianDecoder::processSoftSymbol(float llr) {
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

    // A candidate Start bit is Space (LLR < 0.0f)
    // Only launch if we haven't just accepted a frame ending at the immediate prior symbol
    if (llr < 0.0f && (m_totalSymbols - m_lastAcceptedEndIdx >= 1)) {
        FrameHypothesis hyp;
        hyp.startSampleIdx = m_totalSymbols;
        hyp.bitCount = 0;
        hyp.startLlr = llr;
        m_activeHypotheses.append(hyp);
    }

    // Evaluate completed hypotheses (bitCount == 6)
    int bestIdx = -1;
    float bestScore = -1e9f;
    BayesianCharResult bestResult;

    for (int i = 0; i < m_activeHypotheses.size(); ++i) {
        const auto &hyp = m_activeHypotheses[i];
        if (hyp.bitCount == 6) {
            // Stop bit must not be strongly space
            if (hyp.stopLlr > -1.5f) {
                const bool isFigs = (m_figsBelief > 0.5f);
                BayesianCharResult res = evaluateBaudotPosterior(hyp.dataLlrs, isFigs);
                const float framingMetric = (-hyp.startLlr) + hyp.stopLlr;
                res.framingConfidence = framingMetric;

                if (res.confidence >= m_squelchThreshold && framingMetric >= m_framingThreshold) {
                    const float compositeScore = framingMetric + 5.0f * res.confidence;
                    if (compositeScore > bestScore) {
                        bestScore = compositeScore;
                        bestIdx = i;
                        bestResult = res;
                    }
                }
            }
        }
    }

    if (bestIdx >= 0) {
        const qint64 acceptedStart = m_activeHypotheses[bestIdx].startSampleIdx;
        m_lastAcceptedEndIdx = m_totalSymbols;
        m_lastFramingConfidence = bestResult.framingConfidence;

        if (!m_framingLocked) {
            ++m_lockCounter;
            if (m_lockCounter >= 2) {
                m_framingLocked = true;
                emit framingStateChanged(m_rxId, true, bestResult.framingConfidence);
            }
        }

        updateHMMShift(bestResult.baudotCode);

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

        // Prune hypotheses that overlap with this accepted frame
        QVector<FrameHypothesis> remaining;
        for (const auto &hyp : m_activeHypotheses) {
            if (hyp.startSampleIdx > acceptedStart + 5) {
                remaining.append(hyp);
            }
        }
        m_activeHypotheses = remaining;
    } else {
        // Prune expired hypotheses (bitCount >= 6) that were not accepted
        QVector<FrameHypothesis> remaining;
        for (const auto &hyp : m_activeHypotheses) {
            if (hyp.bitCount < 6) {
                remaining.append(hyp);
            }
        }
        m_activeHypotheses = remaining;
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
    const float *priors = (customPriors != nullptr) ? customPriors : DEFAULT_RTTY_PRIORS;

    for (quint8 c = 0; c < 32; ++c) {
        // Dot product with binary sign vector s_i in {-1, +1}
        float dot = 0.0f;
        for (int bit = 0; bit < 5; ++bit) {
            const float s = ((c >> bit) & 1) ? 1.0f : -1.0f;
            dot += 0.5f * s * dataLlrs[bit];
        }

        const float p = std::max(1e-6f, priors[c]);
        const float priorLog = std::log(p);

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

    // Character lookup from ITA2 tables
    const char ch = figs ? FIGS_TABLE[topCode] : LTRS_TABLE[topCode];
    result.character = (ch != '\0') ? QChar(QLatin1Char(ch)) : QChar('\0');

    return result;
}
