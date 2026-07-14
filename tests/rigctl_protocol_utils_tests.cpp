#include <QtTest/QtTest>

#include "Util/rigctl_protocol_utils.h"

using namespace RigctlProtocol;

class RigctlProtocolUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void dspModeToRigctlString();
    void rigctlModeToDspRoundTrip();
    void unknownRigctlModeReturnsNegativeOne();
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

QTEST_APPLESS_MAIN(RigctlProtocolUtilsTests)
#include "rigctl_protocol_utils_tests.moc"
