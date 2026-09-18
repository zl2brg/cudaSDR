#ifndef RTTLYEXICON_H
#define RTTLYEXICON_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSet>
#include <QRegularExpression>

/**
 * Stage 3: Lexicon & Callsign Priors for Bayesian RTTY Decoder.
 * 
 * Features:
 * - Amateur radio ITU callsign validation (prefix, digit, suffix, portable modifiers).
 * - Standard RTTY contest keyword detection (CQ, DE, TEST, 599, NR, TU, 73, etc.).
 * - Stream tokenizer: extracts words and detects callsigns from incoming decoded characters.
 * - Contextual character prior calculation: boosts likelihood of characters consistent
 *   with valid callsigns and common RTTY exchanges.
 */
class RttyLexicon : public QObject {
    Q_OBJECT

public:
    explicit RttyLexicon(int rxId = 0, QObject *parent = nullptr);
    ~RttyLexicon() override = default;

    int rxId() const { return m_rxId; }
    void setRxId(int id) { m_rxId = id; }

    /**
     * Checks if a token is a valid amateur radio callsign.
     */
    static bool isCallsign(const QString &token);

    /**
     * Checks if a token is a recognized RTTY keyword.
     */
    static bool isCommonKeyword(const QString &token);

    /**
     * Returns a dynamic Bayesian prior bonus (multiplier >= 1.0) for a candidate character
     * given the current word prefix.
     */
    static float getContextualPriorMultiplier(const QString &prefix, QChar candidate);

    QString lastCallsign() const { return m_lastCallsign; }
    QStringList recentCallsigns() const { return m_recentCallsigns; }
    void clearCallsigns();
    void reset();

public slots:
    /**
     * Ingest a single character decoded from the Bayesian decoder.
     */
    void processCharacter(int rx, const QString &character, float confidence, float errProb);

signals:
    void callsignDetected(int rx, const QString &callsign, float confidence);
    void keywordDetected(int rx, const QString &keyword);

private:
    void completeToken(const QString &token, float avgConfidence);

    int m_rxId = 0;
    QString m_currentWord;
    float m_wordConfidenceSum = 0.0f;
    int m_wordCharCount = 0;

    QString m_lastCallsign;
    QStringList m_recentCallsigns;
};

#endif // RTTLYEXICON_H
