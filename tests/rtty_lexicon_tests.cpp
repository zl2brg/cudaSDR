#include <QtTest/QtTest>
#include <QSignalSpy>

#include "AudioEngine/RttyLexicon.h"

class RttyLexiconTests : public QObject {
    Q_OBJECT

private slots:
    void testCallsignValidation();
    void testKeywordValidation();
    void testContextualPriorMultiplier();
    void testStreamTokenExtraction();
};

void RttyLexiconTests::testCallsignValidation() {
    // Valid callsigns
    QVERIFY(RttyLexicon::isCallsign("ZL2BRG"));
    QVERIFY(RttyLexicon::isCallsign("W1AW"));
    QVERIFY(RttyLexicon::isCallsign("DL3HVH"));
    QVERIFY(RttyLexicon::isCallsign("K1JT"));
    QVERIFY(RttyLexicon::isCallsign("JA1ABC"));
    QVERIFY(RttyLexicon::isCallsign("3B8/ZL2BRG"));
    QVERIFY(RttyLexicon::isCallsign("ZL2BRG/P"));
    QVERIFY(RttyLexicon::isCallsign("VE3/W1AW/M"));

    // Invalid callsigns
    QVERIFY(!RttyLexicon::isCallsign(""));
    QVERIFY(!RttyLexicon::isCallsign("AB"));
    QVERIFY(!RttyLexicon::isCallsign("HELLO")); // No digits
    QVERIFY(!RttyLexicon::isCallsign("12345")); // No letters
    QVERIFY(!RttyLexicon::isCallsign("599"));   // RST report
    QVERIFY(!RttyLexicon::isCallsign("73"));    // Number
}

void RttyLexiconTests::testKeywordValidation() {
    QVERIFY(RttyLexicon::isCommonKeyword("CQ"));
    QVERIFY(RttyLexicon::isCommonKeyword("DE"));
    QVERIFY(RttyLexicon::isCommonKeyword("TEST"));
    QVERIFY(RttyLexicon::isCommonKeyword("599"));
    QVERIFY(RttyLexicon::isCommonKeyword("5NN"));
    QVERIFY(RttyLexicon::isCommonKeyword("TU"));
    QVERIFY(RttyLexicon::isCommonKeyword("73"));
    QVERIFY(RttyLexicon::isCommonKeyword("BK"));

    QVERIFY(!RttyLexicon::isCommonKeyword("RANDOM"));
    QVERIFY(!RttyLexicon::isCommonKeyword("ZL2BRG"));
}

void RttyLexiconTests::testContextualPriorMultiplier() {
    // Word bigram rules
    QVERIFY(RttyLexicon::getContextualPriorMultiplier("C", 'Q') > 1.0f);
    QVERIFY(RttyLexicon::getContextualPriorMultiplier("D", 'E') > 1.0f);
    QVERIFY(RttyLexicon::getContextualPriorMultiplier("59", '9') > 1.0f);
    QVERIFY(RttyLexicon::getContextualPriorMultiplier("59", 'N') > 1.0f);
    QVERIFY(RttyLexicon::getContextualPriorMultiplier("7", '3') > 1.0f);

    // Callsign morphology rules
    QVERIFY(RttyLexicon::getContextualPriorMultiplier("ZL", '2') > 1.0f); // Digits after country prefix
    QVERIFY(RttyLexicon::getContextualPriorMultiplier("ZL2", 'B') > 1.0f); // Letters after digit
}

void RttyLexiconTests::testStreamTokenExtraction() {
    RttyLexicon lexicon(0);
    QSignalSpy callSpy(&lexicon, &RttyLexicon::callsignDetected);
    QSignalSpy keySpy(&lexicon, &RttyLexicon::keywordDetected);

    const QString stream = "CQ CQ DE ZL2BRG 599 73 K\n";
    for (const QChar c : stream) {
        lexicon.processCharacter(0, QString(c), 0.98f, 0.02f);
    }

    // Must detect calls
    QCOMPARE(callSpy.count(), 1);
    QCOMPARE(callSpy.at(0).at(1).toString(), QString("ZL2BRG"));
    QCOMPARE(lexicon.lastCallsign(), QString("ZL2BRG"));

    // Must detect keywords (CQ, CQ, DE, 599, 73, K)
    QVERIFY(keySpy.count() >= 5);
}

QTEST_MAIN(RttyLexiconTests)
#include "rtty_lexicon_tests.moc"
