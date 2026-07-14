#include <QtTest/QtTest>

#include "Util/settings_utils.h"

using namespace SettingsUtils;

class SettingsUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void agcHangDisabledForOffMedFast();
    void agcHangEnabledForOtherModes();
    void sampleRateToParamsKnownRates();
    void sampleRateToParamsRejectsUnknown();
    void testClampIntRangeBounds();
    void testClampNetworkPortRejectsOutOfRange();
    void testClampMinimumWidgetWidth();
    void testClampMinimumGroupBoxWidth();
    void testClampMultiRxView();
    void testClampDriveLevel();
    void testClampSocketBufferSizeKb();
    void testStripSurroundingQuotes();
};

void SettingsUtilsTests::agcHangDisabledForOffMedFast()
{
    QVERIFY(!agcHangEnabledForMode(static_cast<AGCMode>(agcOFF)));
    QVERIFY(!agcHangEnabledForMode(static_cast<AGCMode>(agcMED)));
    QVERIFY(!agcHangEnabledForMode(static_cast<AGCMode>(agcFAST)));
}

void SettingsUtilsTests::agcHangEnabledForOtherModes()
{
    QVERIFY(agcHangEnabledForMode(static_cast<AGCMode>(agcSLOW)));
    QVERIFY(agcHangEnabledForMode(static_cast<AGCMode>(agcLONG)));
}

void SettingsUtilsTests::sampleRateToParamsKnownRates()
{
    int speed = -1;
    int outputIncrement = -1;

    QVERIFY(sampleRateToParams(48000, speed, outputIncrement));
    QCOMPARE(speed, 0);
    QCOMPARE(outputIncrement, 1);

    QVERIFY(sampleRateToParams(1536000, speed, outputIncrement));
    QCOMPARE(speed, 5);
    QCOMPARE(outputIncrement, 32);
}

void SettingsUtilsTests::sampleRateToParamsRejectsUnknown()
{
    int speed = 0;
    int outputIncrement = 0;
    QVERIFY(!sampleRateToParams(44100, speed, outputIncrement));
}

void SettingsUtilsTests::testClampIntRangeBounds()
{
    QCOMPARE(clampIntRange(5, 1, 10, 99), 5);
    QCOMPARE(clampIntRange(0, 1, 10, 99), 99);
    QCOMPARE(clampIntRange(11, 1, 10, 99), 99);
}

void SettingsUtilsTests::testClampNetworkPortRejectsOutOfRange()
{
    QCOMPARE(clampNetworkPort(11000, 52685), 11000);
    QCOMPARE(clampNetworkPort(70000, 52685), 52685);
    QCOMPARE(clampNetworkPort(-1, 52685), 52685);
}

void SettingsUtilsTests::testClampMinimumWidgetWidth()
{
    QCOMPARE(clampMinimumWidgetWidth(300), 300);
    QCOMPARE(clampMinimumWidgetWidth(100), 300);
    QCOMPARE(clampMinimumWidgetWidth(400), 300);
}

void SettingsUtilsTests::testClampMinimumGroupBoxWidth()
{
    QCOMPARE(clampMinimumGroupBoxWidth(250, 300), 250);
    QCOMPARE(clampMinimumGroupBoxWidth(200, 300), 250);
    QCOMPARE(clampMinimumGroupBoxWidth(290, 290), 250);
}

void SettingsUtilsTests::testClampMultiRxView()
{
    QCOMPARE(clampMultiRxView(0), 0);
    QCOMPARE(clampMultiRxView(2), 2);
    QCOMPARE(clampMultiRxView(9), 0);
}

void SettingsUtilsTests::testClampDriveLevel()
{
    QCOMPARE(clampDriveLevel(50), 50);
    QCOMPARE(clampDriveLevel(-5), 0);
    QCOMPARE(clampDriveLevel(150), 0);
}

void SettingsUtilsTests::testClampSocketBufferSizeKb()
{
    QCOMPARE(clampSocketBufferSizeKb(16), 16);
    QCOMPARE(clampSocketBufferSizeKb(256), 256);
    QCOMPARE(clampSocketBufferSizeKb(48), 32);
}

void SettingsUtilsTests::testStripSurroundingQuotes()
{
    QCOMPARE(stripSurroundingQuotes(QStringLiteral("\"127.0.0.1\"")), QStringLiteral("127.0.0.1"));
    QCOMPARE(stripSurroundingQuotes(QStringLiteral("127.0.0.1")), QStringLiteral("127.0.0.1"));
    QCOMPARE(stripSurroundingQuotes(QStringLiteral("\"quoted\"")), QStringLiteral("quoted"));
}

QTEST_APPLESS_MAIN(SettingsUtilsTests)
#include "settings_utils_tests.moc"
