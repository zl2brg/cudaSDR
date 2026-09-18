#include "AudioEngine/RttyLexicon.h"

#include <QRegularExpression>

RttyLexicon::RttyLexicon(int rxId, QObject *parent)
    : QObject(parent)
    , m_rxId(rxId)
{
}

bool RttyLexicon::isCallsign(const QString &rawToken) {
    const QString token = rawToken.trimmed().toUpper();
    if (token.length() < 3 || token.length() > 12) {
        return false;
    }

    // Standard ITU amateur radio callsign regex:
    // Optional prefix (e.g. VE3/ or KH6/) + main call (1-3 alphanumeric prefix, at least 1 digit, 1-4 letters) + optional suffix (/P, /M, /0-9)
    static const QRegularExpression callRegex(
        QStringLiteral(R"(^(?:[A-Z0-9]{1,4}/)?[A-Z0-9]{1,3}[0-9][A-Z]{1,4}(?:/[A-Z0-9]{1,4})?$)")
    );

    if (!callRegex.match(token).hasMatch()) {
        return false;
    }

    // Must contain at least one letter and at least one digit
    bool hasLetter = false;
    bool hasDigit = false;
    for (const QChar c : token) {
        if (c.isLetter()) hasLetter = true;
        if (c.isDigit()) hasDigit = true;
    }

    if (!hasLetter || !hasDigit) {
        return false;
    }

    // Filter out obvious false positives like RST reports (e.g. 599, 5NN) or contest numbers
    if (token == QLatin1String("599") || token == QLatin1String("5NN") || token == QLatin1String("73")) {
        return false;
    }

    return true;
}

bool RttyLexicon::isCommonKeyword(const QString &rawToken) {
    const QString token = rawToken.trimmed().toUpper();

    static const QSet<QString> keywords = {
        QStringLiteral("CQ"), QStringLiteral("DE"), QStringLiteral("TEST"),
        QStringLiteral("599"), QStringLiteral("5NN"), QStringLiteral("NR"),
        QStringLiteral("TU"), QStringLiteral("73"), QStringLiteral("BK"),
        QStringLiteral("RST"), QStringLiteral("QSL"), QStringLiteral("QTH"),
        QStringLiteral("NAME"), QStringLiteral("UR"), QStringLiteral("CFM"),
        QStringLiteral("GL"), QStringLiteral("AGN"), QStringLiteral("QRZ"),
        QStringLiteral("K"), QStringLiteral("R"), QStringLiteral("OP"),
        QStringLiteral("RIG"), QStringLiteral("ANT"), QStringLiteral("WX"),
        QStringLiteral("SN"), QStringLiteral("TNX"), QStringLiteral("GM"),
        QStringLiteral("GA"), QStringLiteral("GE"), QStringLiteral("FB")
    };

    return keywords.contains(token);
}

float RttyLexicon::getContextualPriorMultiplier(const QString &prefix, QChar candidate) {
    if (prefix.isEmpty()) return 1.0f;

    const QString upPrefix = prefix.toUpper();
    const QChar upCand = candidate.toUpper();

    // Word bigram rules
    if (upPrefix.endsWith(QLatin1Char('C')) && upCand == QLatin1Char('Q')) return 6.0f;
    if (upPrefix.endsWith(QLatin1Char('D')) && upCand == QLatin1Char('E')) return 6.0f;
    if (upPrefix.endsWith(QLatin1String("59")) && (upCand == QLatin1Char('9') || upCand == QLatin1Char('N'))) return 6.0f;
    if (upPrefix.endsWith(QLatin1Char('T')) && upCand == QLatin1Char('U')) return 4.0f;
    if (upPrefix.endsWith(QLatin1Char('7')) && upCand == QLatin1Char('3')) return 5.0f;

    // Callsign morphology:
    // If prefix is 1-2 letters without digits (e.g. "Z", "ZL", "W", "K", "DL"):
    // highly probable next character is a digit
    bool hasDigit = false;
    for (const QChar c : upPrefix) {
        if (c.isDigit()) {
            hasDigit = true;
            break;
        }
    }

    if (!hasDigit) {
        if (upPrefix.length() >= 1 && upPrefix.length() <= 3 && upCand.isDigit()) {
            return 3.5f;
        }
    } else {
        // Already has digit: highly probable next character is a letter (suffix)
        if (upCand.isLetter()) {
            return 2.5f;
        }
    }

    return 1.0f;
}

void RttyLexicon::clearCallsigns() {
    m_lastCallsign.clear();
    m_recentCallsigns.clear();
}

void RttyLexicon::reset() {
    clearCallsigns();
    m_currentWord.clear();
    m_wordConfidenceSum = 0.0f;
    m_wordCharCount = 0;
}

void RttyLexicon::processCharacter(int rx, const QString &character, float confidence, float /*errProb*/) {
    Q_UNUSED(rx);

    if (character.isEmpty()) return;

    for (const QChar qc : character) {
        if (qc.isSpace() || qc == QLatin1Char('\r') || qc == QLatin1Char('\n')) {
            if (!m_currentWord.isEmpty()) {
                const float avgConf = (m_wordCharCount > 0) ? (m_wordConfidenceSum / static_cast<float>(m_wordCharCount)) : 0.0f;
                completeToken(m_currentWord, avgConf);
                m_currentWord.clear();
                m_wordConfidenceSum = 0.0f;
                m_wordCharCount = 0;
            }
        } else if (qc.isLetterOrNumber() || qc == QLatin1Char('/')) {
            m_currentWord.append(qc.toUpper());
            m_wordConfidenceSum += confidence;
            ++m_wordCharCount;
        }
    }
}

void RttyLexicon::completeToken(const QString &token, float avgConfidence) {
    if (token.isEmpty()) return;

    if (isCallsign(token)) {
        m_lastCallsign = token;
        if (!m_recentCallsigns.contains(token)) {
            m_recentCallsigns.prepend(token);
            if (m_recentCallsigns.size() > 20) {
                m_recentCallsigns.removeLast();
            }
        }
        emit callsignDetected(m_rxId, token, avgConfidence);
    } else if (isCommonKeyword(token)) {
        emit keywordDetected(m_rxId, token);
    }
}
