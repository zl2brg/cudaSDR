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
    void region1BandLimits();
    void region2BandLimits();
    void region3BandLimits();
    void microwaveFrequencies();
    void iaruRegionStrings();
    void hamBandTextRegionDifferences();
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

void HamDatabaseUtilsTests::region1BandLimits()
{
    const auto bands = getHamBandFrequencies(region1);
    // 40m in Region 1: 7.0 - 7.2 MHz
    QCOMPARE(getBandFromFrequency(bands, 7'150'000), static_cast<HamBand>(m40));
    QCOMPARE(getBandFromFrequency(bands, 7'250'000), static_cast<HamBand>(gen));

    // 80m in Region 1: 3.5 - 3.8 MHz
    QCOMPARE(getBandFromFrequency(bands, 3'750'000), static_cast<HamBand>(m80));
    QCOMPARE(getBandFromFrequency(bands, 3'850'000), static_cast<HamBand>(gen));

    // 6m in Region 1: 50 - 52 MHz
    QCOMPARE(getBandFromFrequency(bands, 51'500'000), static_cast<HamBand>(m6));
    QCOMPARE(getBandFromFrequency(bands, 53'000'000), static_cast<HamBand>(gen));

    // 2m in Region 1: 144 - 146 MHz
    QCOMPARE(getBandFromFrequency(bands, 145'000'000), static_cast<HamBand>(m2));
    QCOMPARE(getBandFromFrequency(bands, 147'000'000), static_cast<HamBand>(gen));

    // 1.25m (222-225 MHz) is unallocated in Region 1
    QCOMPARE(getBandFromFrequency(bands, 223'000'000), static_cast<HamBand>(gen));

    // 70cm in Region 1: 430 - 440 MHz
    QCOMPARE(getBandFromFrequency(bands, 425'000'000), static_cast<HamBand>(gen));
    QCOMPARE(getBandFromFrequency(bands, 435'000'000), static_cast<HamBand>(cm70));
    QCOMPARE(getBandFromFrequency(bands, 445'000'000), static_cast<HamBand>(gen));

    // 33cm (902-928 MHz) is unallocated in Region 1
    QCOMPARE(getBandFromFrequency(bands, 915'000'000), static_cast<HamBand>(gen));
}

void HamDatabaseUtilsTests::region2BandLimits()
{
    const auto bands = getHamBandFrequencies(region2);
    // 40m in Region 2: 7.0 - 7.3 MHz
    QCOMPARE(getBandFromFrequency(bands, 7'150'000), static_cast<HamBand>(m40));
    QCOMPARE(getBandFromFrequency(bands, 7'250'000), static_cast<HamBand>(m40));
    QCOMPARE(getBandFromFrequency(bands, 7'350'000), static_cast<HamBand>(gen));

    // 80m in Region 2: 3.5 - 4.0 MHz
    QCOMPARE(getBandFromFrequency(bands, 3'750'000), static_cast<HamBand>(m80));
    QCOMPARE(getBandFromFrequency(bands, 3'950'000), static_cast<HamBand>(m80));
    QCOMPARE(getBandFromFrequency(bands, 4'050'000), static_cast<HamBand>(gen));

    // 6m in Region 2: 50 - 54 MHz
    QCOMPARE(getBandFromFrequency(bands, 51'500'000), static_cast<HamBand>(m6));
    QCOMPARE(getBandFromFrequency(bands, 53'000'000), static_cast<HamBand>(m6));

    // 2m in Region 2: 144 - 148 MHz
    QCOMPARE(getBandFromFrequency(bands, 145'000'000), static_cast<HamBand>(m2));
    QCOMPARE(getBandFromFrequency(bands, 147'000'000), static_cast<HamBand>(m2));

    // 1.25m in Region 2: 222 - 225 MHz
    QCOMPARE(getBandFromFrequency(bands, 223'000'000), static_cast<HamBand>(cm125));

    // 70cm in Region 2: 420 - 450 MHz
    QCOMPARE(getBandFromFrequency(bands, 425'000'000), static_cast<HamBand>(cm70));
    QCOMPARE(getBandFromFrequency(bands, 435'000'000), static_cast<HamBand>(cm70));
    QCOMPARE(getBandFromFrequency(bands, 445'000'000), static_cast<HamBand>(cm70));

    // 33cm in Region 2: 902 - 928 MHz
    QCOMPARE(getBandFromFrequency(bands, 915'000'000), static_cast<HamBand>(cm33));
}

void HamDatabaseUtilsTests::region3BandLimits()
{
    const auto bands = getHamBandFrequencies(region3);
    // 40m in Region 3: 7.0 - 7.3 MHz
    QCOMPARE(getBandFromFrequency(bands, 7'150'000), static_cast<HamBand>(m40));
    QCOMPARE(getBandFromFrequency(bands, 7'250'000), static_cast<HamBand>(m40));

    // 80m in Region 3: 3.5 - 3.9 MHz
    QCOMPARE(getBandFromFrequency(bands, 3'750'000), static_cast<HamBand>(m80));
    QCOMPARE(getBandFromFrequency(bands, 3'850'000), static_cast<HamBand>(m80));
    QCOMPARE(getBandFromFrequency(bands, 3'950'000), static_cast<HamBand>(gen));

    // 6m in Region 3: 50 - 54 MHz
    QCOMPARE(getBandFromFrequency(bands, 51'500'000), static_cast<HamBand>(m6));
    QCOMPARE(getBandFromFrequency(bands, 53'000'000), static_cast<HamBand>(m6));

    // 2m in Region 3: 144 - 148 MHz
    QCOMPARE(getBandFromFrequency(bands, 147'000'000), static_cast<HamBand>(m2));

    // 1.25m and 33cm unallocated in Region 3
    QCOMPARE(getBandFromFrequency(bands, 223'000'000), static_cast<HamBand>(gen));
    QCOMPARE(getBandFromFrequency(bands, 915'000'000), static_cast<HamBand>(gen));

    // 70cm in Region 3: 430 - 450 MHz
    QCOMPARE(getBandFromFrequency(bands, 425'000'000), static_cast<HamBand>(gen));
    QCOMPARE(getBandFromFrequency(bands, 435'000'000), static_cast<HamBand>(cm70));
    QCOMPARE(getBandFromFrequency(bands, 445'000'000), static_cast<HamBand>(cm70));
}

void HamDatabaseUtilsTests::microwaveFrequencies()
{
    const auto bandsR1 = getHamBandFrequencies(region1);
    const auto bandsR2 = getHamBandFrequencies(region2);

    // 23cm: 1240 - 1300 MHz
    QCOMPARE(getBandFromFrequency(bandsR1, 1'296'000'000LL), static_cast<HamBand>(cm23));
    // 13cm: 2300 - 2450 MHz
    QCOMPARE(getBandFromFrequency(bandsR1, 2'400'000'000LL), static_cast<HamBand>(cm13));
    // 10cm: 3400 - 3475 MHz (R1) vs 3300 - 3500 MHz (R2)
    QCOMPARE(getBandFromFrequency(bandsR1, 3'456'000'000LL), static_cast<HamBand>(cm10));
    QCOMPARE(getBandFromFrequency(bandsR1, 3'350'000'000LL), static_cast<HamBand>(gen));
    QCOMPARE(getBandFromFrequency(bandsR2, 3'350'000'000LL), static_cast<HamBand>(cm10));
    // 5cm: 5650 - 5925 MHz
    QCOMPARE(getBandFromFrequency(bandsR1, 5'760'000'000LL), static_cast<HamBand>(cm5));
}

void HamDatabaseUtilsTests::iaruRegionStrings()
{
    QCOMPARE(getIARURegionString(region1), QStringLiteral("Region 1"));
    QCOMPARE(getIARURegionString(region2), QStringLiteral("Region 2"));
    QCOMPARE(getIARURegionString(region3), QStringLiteral("Region 3"));
}

void HamDatabaseUtilsTests::hamBandTextRegionDifferences()
{
    const auto textR1 = getHamBandText(region1);
    const auto textR2 = getHamBandText(region2);
    QCOMPARE(getHamBandTextString(textR1, true, 7'250'000), QStringLiteral("Out of Band"));
    QVERIFY(getHamBandTextString(textR2, true, 7'250'000) != QStringLiteral("Out of Band"));
}

QTEST_APPLESS_MAIN(HamDatabaseUtilsTests)
#include "ham_database_utils_tests.moc"
