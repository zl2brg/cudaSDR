#include <QtTest/QtTest>

#include "cusdr_hamDatabase.h"

class HamDatabaseUtilsTests : public QObject {
    Q_OBJECT

private slots:
    void bandFromFrequencyMatchesKnownBand();
    void bandFromFrequencyReturnsGenWhenOutOfRange();
    void filterFromDspModeMatchesUsb();
    void filterFromDspModeFallsBackToFirst();
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
