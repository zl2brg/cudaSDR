#include <QtTest/QtTest>

#include "cusdr_hamDatabase.h"

class HamDatabaseUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void bandFromFrequencyMatchesKnownBand();
    void bandFromFrequencyReturnsGenWhenOutOfRange();
    void filterFromDspModeMatchesUsb();
    void filterFromDspModeFallsBackToFirst();
    void resolvedTxPassbandKeepsLiveEdges();
    void resolvedTxPassbandRejectsInvertedLsb();
    void resolvedTxPassbandFlipsStaleLsbOntoUsb();
    void resolvedTxPassbandFlipsStaleUsbOntoLsb();
    void hamBandTextStringShortForm();
    void hamBandTextStringOutOfBand();
};

void HamDatabaseUtilsTests::bandFromFrequencyMatchesKnownBand()
{
    const auto bands = getHamBandFrequencies();
    QCOMPARE(getBandFromFrequency(bands, 7'050'000), static_cast<HamBand>(m40));
    QCOMPARE(getBandFromFrequency(bands, 14'100'000), static_cast<HamBand>(m20));
}

void HamDatabaseUtilsTests::bandFromFrequencyReturnsGenWhenOutOfRange()
{
    const auto bands = getHamBandFrequencies();
    QCOMPARE(getBandFromFrequency(bands, 100), static_cast<HamBand>(gen));
}

void HamDatabaseUtilsTests::filterFromDspModeMatchesUsb()
{
    const auto filters = getDefaultFilterFrequencies();
    const TDefaultFilter usbFilter = getFilterFromDSPMode(filters, USB);
    QCOMPARE(usbFilter.dspMode, USB);
    QCOMPARE(usbFilter.defaultFilterMode, filterUSB);
    QCOMPARE(usbFilter.filterLo, 150.0f);
    QCOMPARE(usbFilter.filterHi, 3050.0f);
}

void HamDatabaseUtilsTests::filterFromDspModeFallsBackToFirst()
{
    const auto filters = getDefaultFilterFrequencies();
    const TDefaultFilter fallback = getFilterFromDSPMode(filters, static_cast<DSPMode>(99));
    QCOMPARE(fallback.dspMode, filters.first().dspMode);
}

void HamDatabaseUtilsTests::resolvedTxPassbandKeepsLiveEdges()
{
    const auto filters = getDefaultFilterFrequencies();
    const TDefaultFilter live = resolvedTxPassband(filters, USB, 200.0, 2400.0);
    QCOMPARE(live.filterLo, 200.0);
    QCOMPARE(live.filterHi, 2400.0);
}

void HamDatabaseUtilsTests::resolvedTxPassbandRejectsInvertedLsb()
{
    const auto filters = getDefaultFilterFrequencies();
    // Widget bug: lo=-150, hi=-2700 mutes SSB TX.
    const TDefaultFilter fixed = resolvedTxPassband(filters, LSB, -150.0, -2700.0);
    QCOMPARE(fixed.filterLo, -3050.0);
    QCOMPARE(fixed.filterHi, -150.0);
}

void HamDatabaseUtilsTests::resolvedTxPassbandFlipsStaleLsbOntoUsb()
{
    const auto filters = getDefaultFilterFrequencies();
    const TDefaultFilter flipped = resolvedTxPassband(filters, USB, -3050.0, -150.0);
    QCOMPARE(flipped.filterLo, 150.0);
    QCOMPARE(flipped.filterHi, 3050.0);
}

void HamDatabaseUtilsTests::resolvedTxPassbandFlipsStaleUsbOntoLsb()
{
    const auto filters = getDefaultFilterFrequencies();
    const TDefaultFilter flipped = resolvedTxPassband(filters, DIGL, 150.0, 2700.0);
    QCOMPARE(flipped.filterLo, -2700.0);
    QCOMPARE(flipped.filterHi, -150.0);
}

void HamDatabaseUtilsTests::hamBandTextStringShortForm()
{
    const auto textList = getHamBandText();
    const QString bandText = getHamBandTextString(textList, true, 14'100'000);
    QVERIFY(!bandText.isEmpty());
    QVERIFY(bandText != QStringLiteral("Out of Band"));
}

void HamDatabaseUtilsTests::hamBandTextStringOutOfBand()
{
    const auto textList = getHamBandText();
    QCOMPARE(getHamBandTextString(textList, true, 100), QStringLiteral("Out of Band"));
}

QTEST_APPLESS_MAIN(HamDatabaseUtilsTests)
#include "ham_database_utils_tests.moc"
