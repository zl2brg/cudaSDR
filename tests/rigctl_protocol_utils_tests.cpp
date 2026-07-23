#include <QtTest/QtTest>

#include "Util/rigctl_protocol_utils.h"

using namespace RigctlProtocol;

class RigctlProtocolUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void dspModeToRigctlString();
    void rigctlModeToDspRoundTrip();
    void unknownRigctlModeReturnsNegativeOne();
    void parseFrequencyAcceptsOptionalVfo();
    void parseModeAndPttAcceptOptionalVfo();
};

void RigctlProtocolUtilsTests::dspModeToRigctlString()
{
    QCOMPARE(dspModeToRigctlMode(USB), QStringLiteral("USB"));
    QCOMPARE(dspModeToRigctlMode(CWL), QStringLiteral("CWR"));
    QCOMPARE(dspModeToRigctlMode(DIGU), QStringLiteral("PKTUSB"));
    QCOMPARE(dspModeToRigctlMode(DIGL), QStringLiteral("PKTLSB"));
}

void RigctlProtocolUtilsTests::rigctlModeToDspRoundTrip()
{
    QCOMPARE(rigctlModeToDsp(QStringLiteral("USB")), USB);
    QCOMPARE(rigctlModeToDsp(QStringLiteral("CWR")), CWL);
    QCOMPARE(rigctlModeToDsp(QStringLiteral("CW")), CWU);
    QCOMPARE(rigctlModeToDsp(QStringLiteral("PKTUSB")), DIGU);
    QCOMPARE(rigctlModeToDsp(QStringLiteral("pktlsb")), DIGL);
}

void RigctlProtocolUtilsTests::unknownRigctlModeReturnsNegativeOne()
{
    QCOMPARE(rigctlModeToDsp(QStringLiteral("WFM")), -1);
}

void RigctlProtocolUtilsTests::parseFrequencyAcceptsOptionalVfo()
{
    QCOMPARE(parseFrequencyHz(QStringList{QStringLiteral("F"), QStringLiteral("14074000")}),
             std::optional<qint64>(14074000));
    QCOMPARE(parseFrequencyHz(QStringList{QStringLiteral("F"), QStringLiteral("VFOA"), QStringLiteral("14074000")}),
             std::optional<qint64>(14074000));
    QCOMPARE(parseFrequencyHz(QStringList{QStringLiteral("\\set_freq"), QStringLiteral("VFOB"), QStringLiteral("7074000")}),
             std::optional<qint64>(7074000));
    QVERIFY(!parseFrequencyHz(QStringList{QStringLiteral("F"), QStringLiteral("VFOA")}).has_value());
}

void RigctlProtocolUtilsTests::parseModeAndPttAcceptOptionalVfo()
{
    QCOMPARE(parseModeToken(QStringList{QStringLiteral("M"), QStringLiteral("PKTUSB"), QStringLiteral("3000")}),
             QStringLiteral("PKTUSB"));
    QCOMPARE(parseModeToken(QStringList{QStringLiteral("M"), QStringLiteral("VFOA"), QStringLiteral("USB"), QStringLiteral("2700")}),
             QStringLiteral("USB"));
    QCOMPARE(parsePttValue(QStringList{QStringLiteral("T"), QStringLiteral("1")}), std::optional<int>(1));
    QCOMPARE(parsePttValue(QStringList{QStringLiteral("T"), QStringLiteral("VFOA"), QStringLiteral("0")}),
             std::optional<int>(0));
}

QTEST_APPLESS_MAIN(RigctlProtocolUtilsTests)
#include "rigctl_protocol_utils_tests.moc"
