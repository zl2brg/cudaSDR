#include <QtTest/QtTest>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QVector>

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
    void testClampEqDegAndBandDb();
    void testClampDbAndFreq();
    void testClampTciGain();
    void testIniOnOff();
    void testJsonArrayViaSetters();
    void testCalculateTxFilterBoundsManual();
    void testCalculateTxFilterBoundsUseRxFilter();
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

void SettingsUtilsTests::testClampEqDegAndBandDb()
{
    QCOMPARE(clampEqDeg(0), 0);
    QCOMPARE(clampEqDeg(8), 8);
    QCOMPARE(clampEqDeg(-1), 0);
    QCOMPARE(clampEqDeg(9), 8);
    QCOMPARE(clampEqBandDb(0), 0);
    QCOMPARE(clampEqBandDb(-12), -12);
    QCOMPARE(clampEqBandDb(12), 12);
    QCOMPARE(clampEqBandDb(-20), -12);
    QCOMPARE(clampEqBandDb(20), 12);
}

void SettingsUtilsTests::testClampDbAndFreq()
{
    QCOMPARE(clampDb(0.0, -20.0, 20.0), 0.0);
    QCOMPARE(clampDb(-25.0, -20.0, 20.0), -20.0);
    QCOMPARE(clampDb(25.0, -20.0, 20.0), 20.0);
    QCOMPARE(clampFreq(1500000.0, 0.0, 2000000.0, 1500000.0), 1500000L);
    QCOMPARE(clampFreq(-1.0, 0.0, 2000000.0, 1500000.0), 1500000L);
    QCOMPARE(clampFreq(3000000.0, 0.0, 2000000.0, 1500000.0), 1500000L);
}

void SettingsUtilsTests::testClampTciGain()
{
    QCOMPARE(clampTciGain(1.0f), 1.0f);
    QCOMPARE(clampTciGain(-0.5f), 0.0f);
    QCOMPARE(clampTciGain(3.0f), 2.0f);
}

void SettingsUtilsTests::testIniOnOff()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral("cudaSDR"), QStringLiteral("settings_utils_tests"));
    settings.clear();
    QVERIFY(!iniOn(&settings, QStringLiteral("flag")));
    QVERIFY(iniOn(&settings, QStringLiteral("flag"), QStringLiteral("on")));
    setIniOn(&settings, QStringLiteral("flag"), true);
    QVERIFY(iniOn(&settings, QStringLiteral("flag")));
    setIniOn(&settings, QStringLiteral("flag"), false);
    QVERIFY(!iniOn(&settings, QStringLiteral("flag")));
    settings.setValue(QStringLiteral("legacy"), QStringLiteral("ON"));
    QVERIFY(iniOn(&settings, QStringLiteral("legacy")));
    settings.clear();
}

void SettingsUtilsTests::testJsonArrayViaSetters()
{
    QJsonObject json;
    json[QStringLiteral("bands")] = QJsonArray{1, 2, 3, 4};
    json[QStringLiteral("freqs")] = QJsonArray{1500000.0, 7000000.0};

    QVector<int> bands(3, 0);
    applyJsonArray<int>(json, QStringLiteral("bands"), bands.size(),
                        [&](int i, int v) { bands[i] = v; });
    QCOMPARE(bands, QVector<int>({1, 2, 3}));

    const QVector<int> fromVec = jsonArrayToVector<int>(json, QStringLiteral("bands"), 5, 0);
    QCOMPARE(fromVec, QVector<int>({1, 2, 3, 4, 0}));

    QList<long> freqs(2, 0);
    applyJsonArray<long>(json, QStringLiteral("freqs"), freqs.size(),
                         [&](int i, long v) { freqs[i] = v; });
    QCOMPARE(freqs.at(0), 1500000L);
    QCOMPARE(freqs.at(1), 7000000L);

    applyJsonArray<int>(json, QStringLiteral("missing"), 3, [&](int, int) {
        QFAIL("missing JSON array should not invoke setter");
    });

    const QJsonArray out = toJsonArray(QVector<int>({4, 5, 6}));
    QCOMPARE(out.size(), 3);
    QCOMPARE(out.at(0).toInt(), 4);
    QCOMPARE(out.at(2).toInt(), 6);
}

void SettingsUtilsTests::testCalculateTxFilterBoundsManual()
{
    // Defaults matching deskHPSDR: txFilterLow=100, txFilterHigh=2900, useRxFilter=false
    const auto usb = calculateTxFilterBounds(DSPMode::USB, 100, 2900, false, 0, 0);
    QCOMPARE(usb.low, 100.0);
    QCOMPARE(usb.high, 2900.0);

    const auto lsb = calculateTxFilterBounds(DSPMode::LSB, 100, 2900, false, 0, 0);
    QCOMPARE(lsb.low, -2900.0);
    QCOMPARE(lsb.high, -100.0);

    const auto fmn = calculateTxFilterBounds(DSPMode::FMN, 100, 2900, false, 0, 0);
    QCOMPARE(fmn.low, -3000.0);
    QCOMPARE(fmn.high, 3000.0);

    const auto am = calculateTxFilterBounds(DSPMode::AM, 100, 2900, false, 0, 0);
    QCOMPARE(am.low, -2900.0);
    QCOMPARE(am.high, 2900.0);

    const auto cwu = calculateTxFilterBounds(DSPMode::CWU, 100, 2900, false, 0, 0);
    QCOMPARE(cwu.low, -150.0);
    QCOMPARE(cwu.high, 150.0);

    const auto cwl = calculateTxFilterBounds(DSPMode::CWL, 100, 2900, false, 0, 0);
    QCOMPARE(cwl.low, -150.0);
    QCOMPARE(cwl.high, 150.0);

    const auto fdv = calculateTxFilterBounds(DSPMode::FDV, 100, 2900, false, 0, 0);
    QCOMPARE(fdv.low, 700.0);
    QCOMPARE(fdv.high, 2300.0);
}

void SettingsUtilsTests::testCalculateTxFilterBoundsUseRxFilter()
{
    // With useRxFilter=true, TX filter tracks RX filter edges
    // USB with RX 2.4k (150 to 2550 Hz)
    const auto usb = calculateTxFilterBounds(DSPMode::USB, 100, 2900, true, 150.0, 2550.0);
    QCOMPARE(usb.low, 150.0);
    QCOMPARE(usb.high, 2550.0);

    // USB with stale LSB-signed RX edges must still yield a USB passband
    const auto usbStale = calculateTxFilterBounds(DSPMode::USB, 100, 2900, true, -2550.0, -150.0);
    QCOMPARE(usbStale.low, 150.0);
    QCOMPARE(usbStale.high, 2550.0);

    // TReceiver defaults (-3050/-150) while the band mode is already USB
    const auto usbDefaults = calculateTxFilterBounds(DSPMode::USB, 100, 2900, true, -3050.0, -150.0);
    QCOMPARE(usbDefaults.low, 150.0);
    QCOMPARE(usbDefaults.high, 3050.0);

    QVERIFY(rxFilterSignsMatchMode(DSPMode::USB, 150.0, 3050.0));
    QVERIFY(!rxFilterSignsMatchMode(DSPMode::USB, -3050.0, -150.0));
    QVERIFY(rxFilterSignsMatchMode(DSPMode::LSB, -3050.0, -150.0));
    QVERIFY(!rxFilterSignsMatchMode(DSPMode::LSB, 150.0, 3050.0));

    // LSB with RX 2.1k (-2250 to -150 Hz)
    const auto lsb = calculateTxFilterBounds(DSPMode::LSB, 100, 2900, true, -2250.0, -150.0);
    QCOMPARE(lsb.low, -2250.0);
    QCOMPARE(lsb.high, -150.0);

    // AM with RX 8k (-4000 to 4000 Hz)
    const auto am = calculateTxFilterBounds(DSPMode::AM, 100, 2900, true, -4000.0, 4000.0);
    QCOMPARE(am.low, -4000.0);
    QCOMPARE(am.high, 4000.0);

    // FMN still retains 3 kHz pre-modulator audio filter
    const auto fmn = calculateTxFilterBounds(DSPMode::FMN, 100, 2900, true, -5500.0, 5500.0);
    QCOMPARE(fmn.low, -3000.0);
    QCOMPARE(fmn.high, 3000.0);
}

QTEST_APPLESS_MAIN(SettingsUtilsTests)
#include "settings_utils_tests.moc"
