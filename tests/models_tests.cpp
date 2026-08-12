#include <QtTest/QtTest>
#include <QSignalSpy>

#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "Models/RadioTelemetry.h"
#include "Models/BandPlanManager.h"
#include "cusdr_settings.h"

class ModelsTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // RadioModel tests
    void testRadioModelProperties();
    void testRadioModelSlices();

    // SliceModel tests
    void testSliceModelProperties();
    void testSliceModelVfoAb();

    // RadioTelemetry tests
    void testRadioTelemetrySignals();
    void testRadioTelemetrySMeter();
    void testTelemetryFromSettings();

    void testBandPlanManagerLoadsInternational();
};

void ModelsTests::initTestCase() {
    // Initialize Settings instance if needed, but normally QTEST_MAIN handles it.
}

void ModelsTests::cleanupTestCase() {
    Settings::delete_instance();
}

void ModelsTests::testRadioModelProperties() {
    RadioModel radio;
    QCOMPARE(radio.connected(), false);
    QCOMPARE(radio.sampleRate(), 48000);
    QCOMPARE(radio.hardwareType(), QStringLiteral("Unknown"));

    QSignalSpy spyConnected(&radio, &RadioModel::connectedChanged);
    QSignalSpy spySampleRate(&radio, &RadioModel::sampleRateChanged);
    QSignalSpy spyHardwareType(&radio, &RadioModel::hardwareTypeChanged);
    QSignalSpy spyColors(&radio, &RadioModel::colorsChanged);

    radio.setConnected(true);
    QCOMPARE(radio.connected(), true);
    QCOMPARE(spyConnected.count(), 1);
    QCOMPARE(spyConnected.takeFirst().at(0).toBool(), true);

    // Setting same value shouldn't emit signal
    radio.setConnected(true);
    QCOMPARE(spyConnected.count(), 0);

    radio.setSampleRate(192000);
    QCOMPARE(radio.sampleRate(), 192000);
    QCOMPARE(spySampleRate.count(), 1);
    QCOMPARE(spySampleRate.takeFirst().at(0).toInt(), 192000);

    radio.setSampleRate(192000);
    QCOMPARE(spySampleRate.count(), 0);

    radio.setHardwareType(QStringLiteral("Hermes"));
    QCOMPARE(radio.hardwareType(), QStringLiteral("Hermes"));
    QCOMPARE(spyHardwareType.count(), 1);
    QCOMPARE(spyHardwareType.takeFirst().at(0).toString(), QStringLiteral("Hermes"));

    radio.setHardwareType(QStringLiteral("Hermes"));
    QCOMPARE(spyHardwareType.count(), 0);

    TPanadapterColors colors;
    colors.panBackgroundColor = Qt::blue;
    radio.setPanadapterColors(colors);
    QCOMPARE(radio.panadapterColors().panBackgroundColor, QColor(Qt::blue));
    QCOMPARE(spyColors.count(), 1);
}

void ModelsTests::testRadioModelSlices() {
    RadioModel radio;
    QCOMPARE(radio.slices().size(), 0);

    SliceModel* slice1 = new SliceModel(0, &radio);
    SliceModel* slice2 = new SliceModel(1, &radio);

    radio.addSlice(slice1);
    QCOMPARE(radio.slices().size(), 1);
    QCOMPARE(radio.slices().at(0), slice1);

    // Duplicate add should be ignored
    radio.addSlice(slice1);
    QCOMPARE(radio.slices().size(), 1);

    radio.addSlice(slice2);
    QCOMPARE(radio.slices().size(), 2);
    QCOMPARE(radio.slices().at(1), slice2);

    radio.removeSlice(slice1);
    QCOMPARE(radio.slices().size(), 1);
    QCOMPARE(radio.slices().at(0), slice2);

    // removeSlice deletes the slice object, so slice1 is no longer valid.
    // slice2 is still in the list and owned by radio (will be cleaned up in destructor).
}

void ModelsTests::testSliceModelProperties() {
    SliceModel slice(5);
    QCOMPARE(slice.id(), 5);
    QCOMPARE(slice.frequency(), static_cast<qint64>(7000000));
    QCOMPARE(slice.centerFrequency(), static_cast<qint64>(7000000));
    QCOMPARE(slice.dspMode(), LSB);
    QCOMPARE(slice.filterLow(), -3050.0f);
    QCOMPARE(slice.filterHigh(), -150.0f);
    QCOMPARE(slice.volume(), 0.5f);
    QCOMPARE(slice.mute(), false);
    QCOMPARE(slice.pan(), 0.0f);
    QCOMPARE(slice.agcMode(), agcMED);
    QCOMPARE(slice.nbMode(), 0);
    QCOMPARE(slice.nrMode(), 0);
    QCOMPARE(slice.anf(), false);
    QCOMPARE(slice.snb(), false);
    QCOMPARE(slice.sMeterValue(), -140.0);
    QCOMPARE(slice.active(), false);

    QSignalSpy spyFreq(&slice, &SliceModel::frequencyChanged);
    QSignalSpy spyCenterFreq(&slice, &SliceModel::centerFrequencyChanged);
    QSignalSpy spyDsp(&slice, &SliceModel::dspModeChanged);
    QSignalSpy spyFilter(&slice, &SliceModel::filterChanged);
    QSignalSpy spyVol(&slice, &SliceModel::volumeChanged);
    QSignalSpy spyMute(&slice, &SliceModel::muteChanged);
    QSignalSpy spyPan(&slice, &SliceModel::panChanged);
    QSignalSpy spyAgc(&slice, &SliceModel::agcModeChanged);
    QSignalSpy spyNb(&slice, &SliceModel::nbModeChanged);
    QSignalSpy spyNr(&slice, &SliceModel::nrModeChanged);
    QSignalSpy spyAnf(&slice, &SliceModel::anfChanged);
    QSignalSpy spySnb(&slice, &SliceModel::snbChanged);
    QSignalSpy spySMeter(&slice, &SliceModel::sMeterValueChanged);
    QSignalSpy spyActive(&slice, &SliceModel::activeChanged);

    slice.setFrequency(14000000);
    QCOMPARE(slice.frequency(), static_cast<qint64>(14000000));
    QCOMPARE(spyFreq.count(), 1);

    slice.setCenterFrequency(14050000);
    QCOMPARE(slice.centerFrequency(), static_cast<qint64>(14050000));
    QCOMPARE(spyCenterFreq.count(), 1);

    slice.setDspMode(USB);
    QCOMPARE(slice.dspMode(), USB);
    QCOMPARE(spyDsp.count(), 1);

    slice.setFilterLow(-2700.0f);
    QCOMPARE(slice.filterLow(), -2700.0f);
    QCOMPARE(spyFilter.count(), 1);

    slice.setFilterHigh(-300.0f);
    QCOMPARE(slice.filterHigh(), -300.0f);
    QCOMPARE(spyFilter.count(), 2); // filterChanged emitted for high too

    slice.setVolume(0.8f);
    QCOMPARE(slice.volume(), 0.8f);
    QCOMPARE(spyVol.count(), 1);

    slice.setMute(true);
    QCOMPARE(slice.mute(), true);
    QCOMPARE(spyMute.count(), 1);

    slice.setPan(-0.5f);
    QCOMPARE(slice.pan(), -0.5f);
    QCOMPARE(spyPan.count(), 1);

    slice.setAgcMode(agcFAST);
    QCOMPARE(slice.agcMode(), agcFAST);
    QCOMPARE(spyAgc.count(), 1);

    slice.setNbMode(1);
    QCOMPARE(slice.nbMode(), 1);
    QCOMPARE(spyNb.count(), 1);

    slice.setNrMode(2);
    QCOMPARE(slice.nrMode(), 2);
    QCOMPARE(spyNr.count(), 1);

    slice.setAnf(true);
    QCOMPARE(slice.anf(), true);
    QCOMPARE(spyAnf.count(), 1);

    slice.setSnb(true);
    QCOMPARE(slice.snb(), true);
    QCOMPARE(spySnb.count(), 1);

    slice.setSMeterValue(-73.0);
    QCOMPARE(slice.sMeterValue(), -73.0);
    QCOMPARE(spySMeter.count(), 1);

    slice.setActive(true);
    QCOMPARE(slice.active(), true);
    QCOMPARE(spyActive.count(), 1);

    QSignalSpy spyFilterPreset(&slice, &SliceModel::filterPresetChanged);
    QSignalSpy spyAgcGain(&slice, &SliceModel::agcGainChanged);
    QSignalSpy spyAgcMaxGain(&slice, &SliceModel::agcMaxGainChanged);
    QSignalSpy spyAgcFixedGain(&slice, &SliceModel::agcFixedGainChanged);
    QSignalSpy spyAgcHangThreshold(&slice, &SliceModel::agcHangThresholdChanged);
    QSignalSpy spyAgcSlope(&slice, &SliceModel::agcSlopeChanged);
    QSignalSpy spyNr2GainMethod(&slice, &SliceModel::nr2GainMethodChanged);
    QSignalSpy spyNr2NpeMethod(&slice, &SliceModel::nr2NpeMethodChanged);
    QSignalSpy spyNr2Ae(&slice, &SliceModel::nr2AeChanged);
    QSignalSpy spyNrAgc(&slice, &SliceModel::nrAgcChanged);
    QSignalSpy spySMeterHoldTime(&slice, &SliceModel::sMeterHoldTimeChanged);
    QSignalSpy spyFftSize(&slice, &SliceModel::fftSizeChanged);
    QSignalSpy spySpectrumAveraging(&slice, &SliceModel::spectrumAveragingChanged);
    QSignalSpy spySpectrumAveragingCnt(&slice, &SliceModel::spectrumAveragingCntChanged);
    QSignalSpy spyPanAveragingMode(&slice, &SliceModel::panAveragingModeChanged);
    QSignalSpy spyPanMode(&slice, &SliceModel::panModeChanged);
    QSignalSpy spyPanDetectorMode(&slice, &SliceModel::panDetectorModeChanged);
    QSignalSpy spyWaterfallMode(&slice, &SliceModel::waterfallModeChanged);
    QSignalSpy spyWaterfallOffset(&slice, &SliceModel::waterfallOffsetChanged);
    QSignalSpy spyPanGrid(&slice, &SliceModel::panGridChanged);
    QSignalSpy spyPeakHold(&slice, &SliceModel::peakHoldChanged);
    QSignalSpy spyPanScale(&slice, &SliceModel::panScaleChanged);

    QCOMPARE(slice.filterPreset(), 3);
    slice.setFilterPreset(2);
    QCOMPARE(slice.filterPreset(), 2);
    QCOMPARE(spyFilterPreset.count(), 1);

    QCOMPARE(slice.agcGain(), 30);
    slice.setAgcGain(40);
    QCOMPARE(slice.agcGain(), 40);
    QCOMPARE(spyAgcGain.count(), 1);

    QCOMPARE(slice.agcMaxGain(), 100);
    slice.setAgcMaxGain(90);
    QCOMPARE(slice.agcMaxGain(), 90);
    QCOMPARE(spyAgcMaxGain.count(), 1);

    QCOMPARE(slice.agcFixedGain(), 0);
    slice.setAgcFixedGain(10);
    QCOMPARE(slice.agcFixedGain(), 10);
    QCOMPARE(spyAgcFixedGain.count(), 1);

    QCOMPARE(slice.agcHangThreshold(), -100);
    slice.setAgcHangThreshold(-90);
    QCOMPARE(slice.agcHangThreshold(), -90);
    QCOMPARE(spyAgcHangThreshold.count(), 1);

    QCOMPARE(slice.agcSlope(), 0);
    slice.setAgcSlope(5);
    QCOMPARE(slice.agcSlope(), 5);
    QCOMPARE(spyAgcSlope.count(), 1);

    QCOMPARE(slice.nr2GainMethod(), 0);
    slice.setNr2GainMethod(1);
    QCOMPARE(slice.nr2GainMethod(), 1);
    QCOMPARE(spyNr2GainMethod.count(), 1);

    QCOMPARE(slice.nr2NpeMethod(), 0);
    slice.setNr2NpeMethod(1);
    QCOMPARE(slice.nr2NpeMethod(), 1);
    QCOMPARE(spyNr2NpeMethod.count(), 1);

    QCOMPARE(slice.nr2Ae(), false);
    slice.setNr2Ae(true);
    QCOMPARE(slice.nr2Ae(), true);
    QCOMPARE(spyNr2Ae.count(), 1);

    QCOMPARE(slice.nrAgc(), 0);
    slice.setNrAgc(1);
    QCOMPARE(slice.nrAgc(), 1);
    QCOMPARE(spyNrAgc.count(), 1);

    QCOMPARE(slice.sMeterHoldTime(), 1000);
    slice.setSMeterHoldTime(2000);
    QCOMPARE(slice.sMeterHoldTime(), 2000);
    QCOMPARE(spySMeterHoldTime.count(), 1);

    QCOMPARE(slice.fftSize(), 1);
    slice.setFftSize(2);
    QCOMPARE(slice.fftSize(), 2);
    QCOMPARE(spyFftSize.count(), 1);

    QCOMPARE(slice.spectrumAveraging(), false);
    slice.setSpectrumAveraging(true);
    QCOMPARE(slice.spectrumAveraging(), true);
    QCOMPARE(spySpectrumAveraging.count(), 1);

    QCOMPARE(slice.spectrumAveragingCnt(), 5);
    slice.setSpectrumAveragingCnt(10);
    QCOMPARE(slice.spectrumAveragingCnt(), 10);
    QCOMPARE(spySpectrumAveragingCnt.count(), 1);

    QCOMPARE(slice.panAveragingMode(), AV_MODE_NONE);
    slice.setPanAveragingMode(AV_MODE_RECURSIVE);
    QCOMPARE(slice.panAveragingMode(), AV_MODE_RECURSIVE);
    QCOMPARE(spyPanAveragingMode.count(), 1);

    QCOMPARE(slice.panMode(), FilledLine);
    slice.setPanMode(Solid);
    QCOMPARE(slice.panMode(), Solid);
    QCOMPARE(spyPanMode.count(), 1);

    QCOMPARE(slice.panDetectorMode(), DET_MODE_PEAK);
    slice.setPanDetectorMode(DET_MODE_AVERAGE);
    QCOMPARE(slice.panDetectorMode(), DET_MODE_AVERAGE);
    QCOMPARE(spyPanDetectorMode.count(), 1);

    QCOMPARE(slice.waterfallMode(), Simple);
    slice.setWaterfallMode(Enhanced);
    QCOMPARE(slice.waterfallMode(), Enhanced);
    QCOMPARE(spyWaterfallMode.count(), 1);

    QCOMPARE(slice.waterfallOffsetLo(), -120);
    slice.setWaterfallOffsetLo(-110);
    QCOMPARE(slice.waterfallOffsetLo(), -110);
    QCOMPARE(spyWaterfallOffset.count(), 1);

    QCOMPARE(slice.waterfallOffsetHi(), -60);
    slice.setWaterfallOffsetHi(-50);
    QCOMPARE(slice.waterfallOffsetHi(), -50);
    QCOMPARE(spyWaterfallOffset.count(), 2);

    QCOMPARE(slice.panGrid(), true);
    slice.setPanGrid(false);
    QCOMPARE(slice.panGrid(), false);
    QCOMPARE(spyPanGrid.count(), 1);

    QCOMPARE(slice.peakHold(), false);
    slice.setPeakHold(true);
    QCOMPARE(slice.peakHold(), true);
    QCOMPARE(spyPeakHold.count(), 1);

    QCOMPARE(slice.dBmPanScaleMin(), -140.0);
    slice.setDBmPanScaleMin(-130.0);
    QCOMPARE(slice.dBmPanScaleMin(), -130.0);
    QCOMPARE(spyPanScale.count(), 1);

    QCOMPARE(slice.dBmPanScaleMax(), -20.0);
    slice.setDBmPanScaleMax(-30.0);
    QCOMPARE(slice.dBmPanScaleMax(), -30.0);
    QCOMPARE(spyPanScale.count(), 2);
}

void ModelsTests::testSliceModelVfoAb() {
    SliceModel slice(0);
    QCOMPARE(slice.activeVfo(), SliceModel::VfoA);
    QCOMPARE(slice.vfoAFrequency(), slice.frequency());
    QCOMPARE(slice.vfoBFrequency(), slice.frequency());

    QSignalSpy spyFreq(&slice, &SliceModel::frequencyChanged);
    QSignalSpy spyA(&slice, &SliceModel::vfoAFrequencyChanged);
    QSignalSpy spyB(&slice, &SliceModel::vfoBFrequencyChanged);
    QSignalSpy spyActive(&slice, &SliceModel::activeVfoChanged);

    // Dial write-through updates the active slot (A).
    slice.setFrequency(14'070'000);
    QCOMPARE(slice.frequency(), static_cast<qint64>(14'070'000));
    QCOMPARE(slice.vfoAFrequency(), static_cast<qint64>(14'070'000));
    QCOMPARE(slice.vfoBFrequency(), static_cast<qint64>(7'000'000));
    QCOMPARE(spyFreq.count(), 1);
    QCOMPARE(spyA.count(), 1);

    // Store B without retuning while A is active.
    slice.setVfoBFrequency(7'074'000);
    QCOMPARE(slice.vfoBFrequency(), static_cast<qint64>(7'074'000));
    QCOMPARE(slice.frequency(), static_cast<qint64>(14'070'000));
    QCOMPARE(spyFreq.count(), 1);
    QCOMPARE(spyB.count(), 1);

    // Switch to B retunes live dial.
    slice.setActiveVfo(SliceModel::VfoB);
    QCOMPARE(slice.activeVfo(), SliceModel::VfoB);
    QCOMPARE(slice.frequency(), static_cast<qint64>(7'074'000));
    QCOMPARE(spyActive.count(), 1);
    QCOMPARE(spyFreq.count(), 2);

    // Dial while B is active updates B only.
    slice.setFrequency(10'136'000);
    QCOMPARE(slice.vfoBFrequency(), static_cast<qint64>(10'136'000));
    QCOMPARE(slice.vfoAFrequency(), static_cast<qint64>(14'070'000));

    slice.copyAtoB();
    QCOMPARE(slice.vfoBFrequency(), static_cast<qint64>(14'070'000));
    QCOMPARE(slice.frequency(), static_cast<qint64>(14'070'000));

    slice.setVfoBFrequency(21'074'000);
    QCOMPARE(slice.frequency(), static_cast<qint64>(21'074'000));
    slice.copyBtoA();
    QCOMPARE(slice.vfoAFrequency(), static_cast<qint64>(21'074'000));
    // Active remains B; live dial already matches B.
    QCOMPARE(slice.activeVfo(), SliceModel::VfoB);
    QCOMPARE(slice.frequency(), static_cast<qint64>(21'074'000));

    slice.setVfoMemories(14'070'000, 7'074'000, SliceModel::VfoA);
    QCOMPARE(slice.frequency(), static_cast<qint64>(14'070'000));
    QCOMPARE(slice.activeVfo(), SliceModel::VfoA);

    slice.swapVfos();
    QCOMPARE(slice.vfoAFrequency(), static_cast<qint64>(7'074'000));
    QCOMPARE(slice.vfoBFrequency(), static_cast<qint64>(14'070'000));
    QCOMPARE(slice.frequency(), static_cast<qint64>(7'074'000));
}

void ModelsTests::testRadioTelemetrySignals() {
    RadioModel radio;
    RadioTelemetry telemetry(&radio);

    QSignalSpy spyProtocolSync(&telemetry, &RadioTelemetry::protocolSyncChanged);
    QSignalSpy spyAdcOverflow(&telemetry, &RadioTelemetry::adcOverflowChanged);
    QSignalSpy spyPacketLoss(&telemetry, &RadioTelemetry::packetLossChanged);
    QSignalSpy spyFwdPower(&telemetry, &RadioTelemetry::forwardPowerChanged);
    QSignalSpy spyRevPower(&telemetry, &RadioTelemetry::reversePowerChanged);
    QSignalSpy spySwr(&telemetry, &RadioTelemetry::swrChanged);
    QSignalSpy spySupply(&telemetry, &RadioTelemetry::supplyVoltageChanged);
    QSignalSpy spyTemp(&telemetry, &RadioTelemetry::temperatureChanged);
    QSignalSpy spySendIQ(&telemetry, &RadioTelemetry::sendIQSignalChanged);
    QSignalSpy spyRcveIQ(&telemetry, &RadioTelemetry::rcveIQSignalChanged);
    QSignalSpy spySpectrum(&telemetry, &RadioTelemetry::spectrumBufferChanged);
    QSignalSpy spyPostSpectrum(&telemetry, &RadioTelemetry::postSpectrumBufferChanged);
    QSignalSpy spyWbSpectrum(&telemetry, &RadioTelemetry::widebandSpectrumBufferChanged);
    QSignalSpy spyWbReset(&telemetry, &RadioTelemetry::widebandSpectrumBufferReset);
    QSignalSpy spyWbFreqRange(&telemetry, &RadioTelemetry::widebandFrequencyRangeChanged);

    telemetry.setProtocolSync(5);
    QCOMPARE(spyProtocolSync.count(), 1);
    QCOMPARE(spyProtocolSync.first().at(0).toInt(), 5);

    telemetry.setADCOverflow(1);
    QCOMPARE(spyAdcOverflow.count(), 1);
    QCOMPARE(spyAdcOverflow.first().at(0).toInt(), 1);

    telemetry.setPacketLoss(10);
    QCOMPARE(spyPacketLoss.count(), 1);
    QCOMPARE(spyPacketLoss.first().at(0).toInt(), 10);

    telemetry.setForwardPower(100.5);
    QCOMPARE(spyFwdPower.count(), 1);
    QCOMPARE(spyFwdPower.first().at(0).toDouble(), 100.5);

    telemetry.setReversePower(2.3);
    QCOMPARE(spyRevPower.count(), 1);
    QCOMPARE(spyRevPower.first().at(0).toDouble(), 2.3);

    telemetry.setSWR(1.5);
    QCOMPARE(spySwr.count(), 1);
    QCOMPARE(spySwr.first().at(0).toDouble(), 1.5);

    telemetry.setSupplyVoltage(13.8);
    QCOMPARE(spySupply.count(), 1);
    QCOMPARE(spySupply.first().at(0).toDouble(), 13.8);

    telemetry.setTemperature(45.2);
    QCOMPARE(spyTemp.count(), 1);
    QCOMPARE(spyTemp.first().at(0).toDouble(), 45.2);

    telemetry.setSendIQ(1);
    QCOMPARE(spySendIQ.count(), 1);
    QCOMPARE(spySendIQ.first().at(0).toInt(), 1);

    telemetry.setRcveIQ(0);
    QCOMPARE(spyRcveIQ.count(), 1);
    QCOMPARE(spyRcveIQ.first().at(0).toInt(), 0);

    qVectorFloat spectrum = { 1.0f, 2.0f, 3.0f };
    telemetry.setSpectrumBuffer(2, spectrum);
    QCOMPARE(spySpectrum.count(), 1);
    QCOMPARE(spySpectrum.first().at(0).toInt(), 2);
    QCOMPARE(spySpectrum.first().at(1).value<qVectorFloat>(), spectrum);

    float postSpectrum[] = { 4.0f, 5.0f };
    telemetry.setPostSpectrumBuffer(1, postSpectrum);
    QCOMPARE(spyPostSpectrum.count(), 1);
    QCOMPARE(spyPostSpectrum.first().at(0).toInt(), 1);
    QCOMPARE(spyPostSpectrum.first().at(1).value<const float*>(), postSpectrum);

    telemetry.setWidebandSpectrumBuffer(spectrum);
    QCOMPARE(spyWbSpectrum.count(), 1);
    QCOMPARE(spyWbSpectrum.first().at(0).value<qVectorFloat>(), spectrum);

    telemetry.resetWidebandSpectrumBuffer();
    QCOMPARE(spyWbReset.count(), 1);

    telemetry.setWidebandFrequencyRange(1.8e6, 30e6);
    QCOMPARE(spyWbFreqRange.count(), 1);
    QCOMPARE(spyWbFreqRange.first().at(0).toDouble(), 1.8e6);
    QCOMPARE(spyWbFreqRange.first().at(1).toDouble(), 30e6);
}

void ModelsTests::testRadioTelemetrySMeter() {
    RadioModel radio;
    RadioTelemetry telemetry(&radio);

    // If slice doesn't exist, setSMeterValue is a no-op and doesn't crash
    telemetry.setSMeterValue(0, -50.0);

    SliceModel* slice = new SliceModel(0, &radio);
    radio.addSlice(slice);

    QCOMPARE(slice->sMeterValue(), -140.0);
    telemetry.setSMeterValue(0, -60.0);
    QCOMPARE(slice->sMeterValue(), -60.0);

    // Bounds checking: rx index out of range
    telemetry.setSMeterValue(1, -50.0); // should not crash
    telemetry.setSMeterValue(-1, -50.0); // should not crash
}

void ModelsTests::testTelemetryFromSettings() {
    // Before radioModel is set, telemetryFromSettings should be nullptr
    QVERIFY(telemetryFromSettings() == nullptr);

    Settings* settings = Settings::instance();
    RadioModel* radio = new RadioModel(settings);
    settings->setRadioModel(radio);

    QVERIFY(telemetryFromSettings() != nullptr);
    QCOMPARE(telemetryFromSettings(), radio->telemetry());

    // Clean up settings to avoid leakage
    settings->setRadioModel(nullptr);
    delete radio;
}

void ModelsTests::testBandPlanManagerLoadsInternational() {
    BandPlanManager mgr;
    const QByteArray xml =
        "<?xml version=\"1.0\"?>"
        "<ArrayOfRangeEntry>"
        "<RangeEntry minFrequency=\"7000000\" maxFrequency=\"7040000\" mode=\"CW\" step=\"10\" color=\"50FF0000\">40m CW</RangeEntry>"
        "<RangeEntry minFrequency=\"7040000\" maxFrequency=\"7200000\" mode=\"LSB\" step=\"10\" color=\"50FF8000\">40m Phone</RangeEntry>"
        "</ArrayOfRangeEntry>";
    QVERIFY(mgr.loadFromData(xml));
    QCOMPARE(mgr.ranges().size(), 2);
    QCOMPARE(mgr.labelAt(7010000), QStringLiteral("40m CW"));
    QCOMPARE(mgr.labelAt(7100000), QStringLiteral("40m Phone"));
    QVERIFY(mgr.labelAt(6000000).isEmpty());

    const QVector<BandRange> mid = mgr.rangesInSpan(7030000, 7050000);
    QCOMPARE(mid.size(), 2);

    const QByteArray kiwi =
        "{\"dx\":["
        "[14095.6,\"USB\",\"WSPR\",\"\",{\"SB\":1}],"
        "[10000.0,\"AM\",\"WWV%20/%20WWVH\",\"time%20signals\",{\"WL\":1}]"
        "]}";
    QVERIFY(mgr.loadKiwiDxFromData(kiwi));
    QCOMPARE(mgr.spots().size(), 2);
    QCOMPARE(mgr.spots().at(0).freqHz, 10000000LL);
    QCOMPARE(mgr.spots().at(0).label, QStringLiteral("WWV / WWVH"));
    QCOMPARE(mgr.spots().at(1).label, QStringLiteral("WSPR"));

    const QByteArray spotsJson =
        "{\"spots\":["
        "{\"freq\":14.074,\"label\":\"FT8\"},"
        "{\"freq\":14.0956,\"label\":\"WSPR\"}"
        "]}";
    BandPlanManager digi;
    QVERIFY(digi.loadSpotsFromData(spotsJson));
    mgr.mergeSpots(digi.spots());
    // WSPR near-dupe skipped; FT8 added.
    QCOMPARE(mgr.spots().size(), 3);
    QCOMPARE(mgr.spotsInSpan(14070000, 14100000).size(), 2);
}

QTEST_MAIN(ModelsTests)
#include "models_tests.moc"
