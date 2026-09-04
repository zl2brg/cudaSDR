#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QJsonArray>
#include <QColor>

#include <QTemporaryFile>
#include <QTemporaryDir>

#include "cusdr_settings.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"
#include "Models/TransmitModel.h"
#include "Settings/DisplayConfig.h"
#include "Settings/ReceiverConfig.h"
#include "Settings/NetworkConfig.h"
#include "Settings/AudioConfig.h"
#include "Settings/CWConfig.h"
#include "Settings/HardwareConfig.h"
#include "Settings/AlexConfig.h"
#include "Settings/TransmitConfig.h"
#include "Settings/FreeDVConfig.h"
#include "Settings/WindowConfig.h"
#include "Settings/TciConfig.h"
#include "Settings/SoapyConfig.h"
#include "Settings/WidebandConfig.h"
#include "Settings/PennyConfig.h"

class ConfigJsonTests : public QObject {
    Q_OBJECT

private slots:
    void testDisplayConfigJson();
    void testReceiverConfigJson();
    void testNetworkConfigJson();
    void testAudioConfigJson();
    void testCWConfigJson();
    void testHardwareConfigJson();
    void testAlexConfigJson();
    void testTransmitConfigJson();
    void testFreeDVConfigJson();
    void testWindowConfigJson();
    void testTciConfigJson();
    void testSoapyConfigJson();
    void testWidebandConfigJson();
    void testPennyConfigJson();
    void testSettingsFullJsonRoundtrip();
    void testFromJsonGettersMatchConfigObjects();
    void testSettingsSaveAndLoadJsonFile();
    void testSettingsJsonSchemaValidation();
    void testSettingsJsonModelHydration();
};

void ConfigJsonTests::testDisplayConfigJson() {
    DisplayConfig config;
    QCOMPARE(config.spectrumSize(), 4096);
    QCOMPARE(config.dBmDistScaleMin(), -20.0);

    QSignalSpy spySize(&config, &DisplayConfig::spectrumSizeChanged);
    QSignalSpy spyMin(&config, &DisplayConfig::dBmDistScaleMinChanged);
    QSignalSpy spyMax(&config, &DisplayConfig::dBmDistScaleMaxChanged);
    QSignalSpy spyHold(&config, &DisplayConfig::sMeterHoldTimeChanged);
    QSignalSpy spySMeter(&config, &DisplayConfig::showPanadapterSMeterChanged);
    QSignalSpy spyFreq(&config, &DisplayConfig::showPanadapterFreqChanged);
    QSignalSpy spyColors(&config, &DisplayConfig::panadapterColorsChanged);

    config.setSpectrumSize(2048);
    config.setdBmDistScaleMin(-40.0);
    config.setdBmDistScaleMax(80.0);
    config.setSMeterHoldTime(1000);
    config.setShowPanadapterSMeter(false);
    config.setShowPanadapterFreq(false);

    TPanadapterColors colors;
    colors.panBackgroundColor = Qt::red;
    colors.waterfallColor = Qt::green;
    colors.panLineColor = Qt::blue;
    config.setPanadapterColors(colors);

    QCOMPARE(spySize.count(), 1);
    QCOMPARE(spyMin.count(), 1);
    QCOMPARE(spyMax.count(), 1);
    QCOMPARE(spyHold.count(), 1);
    QCOMPARE(spySMeter.count(), 1);
    QCOMPARE(spyFreq.count(), 1);
    QCOMPARE(spyColors.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["spectrumSize"].toInt(), 2048);
    QCOMPARE(json["dBmDistScaleMin"].toDouble(), -40.0);
    QCOMPARE(json["dBmDistScaleMax"].toDouble(), 80.0);
    QCOMPARE(json["sMeterHoldTime"].toInt(), 1000);
    QCOMPARE(json["showPanadapterSMeter"].toBool(), false);
    QCOMPARE(json["showPanadapterFreq"].toBool(), false);

    QJsonObject colorsObj = json["colors"].toObject();
    QCOMPARE(colorsObj["panBackground"].toString(), DisplayConfig::colorToString(Qt::red));
    QCOMPARE(colorsObj["waterfall"].toString(), DisplayConfig::colorToString(Qt::green));
    QCOMPARE(colorsObj["panLine"].toString(), DisplayConfig::colorToString(Qt::blue));

    // Load into a new config
    DisplayConfig config2;
    config2.load(json);

    QCOMPARE(config2.spectrumSize(), 2048);
    QCOMPARE(config2.dBmDistScaleMin(), -40.0);
    QCOMPARE(config2.dBmDistScaleMax(), 80.0);
    QCOMPARE(config2.sMeterHoldTime(), 1000);
    QCOMPARE(config2.showPanadapterSMeter(), false);
    QCOMPARE(config2.showPanadapterFreq(), false);
    QCOMPARE(config2.panadapterColors().panBackgroundColor, QColor(Qt::red));
    QCOMPARE(config2.panadapterColors().waterfallColor, QColor(Qt::green));
    QCOMPARE(config2.panadapterColors().panLineColor, QColor(Qt::blue));
}

void ConfigJsonTests::testReceiverConfigJson() {
    ReceiverConfig config(1);
    QCOMPARE(config.id(), 1);
    QCOMPARE(config.ctrFrequency(), static_cast<qint64>(7050000));

    QSignalSpy spyDspCore(&config, &ReceiverConfig::dspCoreChanged);
    QSignalSpy spyHamBand(&config, &ReceiverConfig::hamBandChanged);
    QSignalSpy spyDspMode(&config, &ReceiverConfig::dspModeChanged);
    QSignalSpy spyAdcMode(&config, &ReceiverConfig::adcModeChanged);
    QSignalSpy spyAgcMode(&config, &ReceiverConfig::agcModeChanged);
    QSignalSpy spyCtrFreq(&config, &ReceiverConfig::ctrFrequencyChanged);
    QSignalSpy spyVfoFreq(&config, &ReceiverConfig::vfoFrequencyChanged);

    config.setDspCore(QSDR::CudaDSP);
    config.setHamBand(m80);
    config.setDspMode(LSB);
    config.setAdcMode(adc2);
    config.setAgcMode(agcSLOW);
    config.setCtrFrequency(3500000);
    config.setVfoFrequency(3600000);

    QCOMPARE(spyDspCore.count(), 1);
    QCOMPARE(spyHamBand.count(), 1);
    QCOMPARE(spyDspMode.count(), 1);
    QCOMPARE(spyAdcMode.count(), 1);
    QCOMPARE(spyAgcMode.count(), 1);
    QCOMPARE(spyCtrFreq.count(), 1);
    QCOMPARE(spyVfoFreq.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["dspCore"].toInt(), static_cast<int>(QSDR::CudaDSP));
    QCOMPARE(json["hamBand"].toInt(), static_cast<int>(m80));
    QCOMPARE(json["dspMode"].toInt(), static_cast<int>(LSB));
    QCOMPARE(json["adcMode"].toInt(), static_cast<int>(adc2));
    QCOMPARE(json["agcMode"].toInt(), static_cast<int>(agcSLOW));
    QCOMPARE(json["ctrFrequency"].toDouble(), 3500000.0);
    QCOMPARE(json["vfoFrequency"].toDouble(), 3600000.0);

    ReceiverConfig config2(2);
    config2.load(json);

    QCOMPARE(config2.dspCore(), QSDR::CudaDSP);
    QCOMPARE(config2.hamBand(), m80);
    QCOMPARE(config2.dspMode(), LSB);
    QCOMPARE(config2.adcMode(), adc2);
    QCOMPARE(config2.agcMode(), agcSLOW);
    QCOMPARE(config2.ctrFrequency(), static_cast<qint64>(3500000));
    QCOMPARE(config2.vfoFrequency(), static_cast<qint64>(3600000));

    config2.setFilterSlope(2);
    TReceiver rx{};
    rx.dspModeList = QList<DSPMode>() << LSB << USB << LSB; // enough slots for m80
    while (rx.dspModeList.size() < MAX_BANDS)
        rx.dspModeList << USB;
    config2.applyTo(rx);
    QCOMPARE(rx.ctrFrequency, static_cast<qint64>(3500000));
    QCOMPARE(rx.vfoFrequency, static_cast<qint64>(3600000));
    QCOMPARE(rx.hamBand, m80);
    QCOMPARE(rx.dspMode, LSB);
    QCOMPARE(rx.filterSlope, 2);
    QCOMPARE(rx.dspModeList.value(m80), LSB);

    rx.vfoFrequency = 3700000;
    rx.filterSlope = 0;
    ReceiverConfig config3(3);
    config3.fromReceiver(rx);
    QCOMPARE(config3.vfoFrequency(), static_cast<qint64>(3700000));
    QCOMPARE(config3.filterSlope(), 0);

    ReceiverConfig full(0);
    full.setNr(2);
    full.setAnf(true);
    full.setSnb(true);
    full.setNbMode(1);
    full.setFftSize(4);
    full.setPanAvMode(AV_MODE_TIME_WINDOW);
    full.setPanDetMode(DET_MODE_AVERAGE);
    full.setAudioVolume(0.42f);
    full.setFilterLo(-2400);
    full.setFilterHi(-200);
    full.setAgcGain(80);
    full.setFramesPerSecond(30);
    full.setCwDecode(true);
    QList<qint64> centers = full.lastCenterFrequencyList();
    centers[m40] = 7150000;
    full.setLastCenterFrequencyList(centers);
    QList<int> att = full.mercuryAttenuators();
    att[m40] = 2;
    full.setMercuryAttenuators(att);
    QList<DSPMode> modes = full.dspModeList();
    modes[m40] = USB;
    full.setDspModeList(modes);

    QJsonObject fullJson;
    full.save(fullJson);
    QCOMPARE(fullJson["nr"].toInt(), 2);
    QCOMPARE(fullJson["anf"].toBool(), true);
    QCOMPARE(fullJson["filterLo"].toDouble(), -2400.0);
    QCOMPARE(fullJson["lastCenterFrequencyList"].toArray().size(), ReceiverConfig::kBandCount);

    ReceiverConfig loaded(4);
    loaded.load(fullJson);
    QCOMPARE(loaded.nr(), 2);
    QCOMPARE(loaded.anf(), true);
    QCOMPARE(loaded.snb(), true);
    QCOMPARE(loaded.nbMode(), 1);
    QCOMPARE(loaded.fftSize(), 4);
    QCOMPARE(loaded.panAvMode(), AV_MODE_TIME_WINDOW);
    QCOMPARE(loaded.panDetMode(), DET_MODE_AVERAGE);
    QCOMPARE(loaded.audioVolume(), 0.42f);
    QCOMPARE(loaded.filterLo(), -2400.0);
    QCOMPARE(loaded.filterHi(), -200.0);
    QCOMPARE(loaded.agcGain(), 80.0);
    QCOMPARE(loaded.framesPerSecond(), 30);
    QCOMPARE(loaded.cwDecode(), true);
    QCOMPARE(loaded.lastCenterFrequencyList().at(m40), static_cast<qint64>(7150000));
    QCOMPARE(loaded.mercuryAttenuators().at(m40), 2);
    QCOMPARE(loaded.dspModeList().at(m40), USB);

    TReceiver rxFull{};
    loaded.applyTo(rxFull);
    QCOMPARE(rxFull.nr, 2);
    QCOMPARE(rxFull.anf, true);
    QCOMPARE(rxFull.filterLo, -2400.0);
    QCOMPARE(rxFull.lastCenterFrequencyList.at(m40), static_cast<qint64>(7150000));
    QCOMPARE(rxFull.mercuryAttenuators.at(m40), 2);
    QCOMPARE(rxFull.dspModeList.at(m40), USB);
}

void ConfigJsonTests::testNetworkConfigJson() {
    NetworkConfig config;
    QCOMPARE(config.serverAddress(), QStringLiteral("127.0.0.1"));
    QCOMPARE(config.serverPort(), static_cast<quint16>(52685));

    QSignalSpy spyServAddr(&config, &NetworkConfig::serverAddressChanged);
    QSignalSpy spyLocAddr(&config, &NetworkConfig::localAddressChanged);
    QSignalSpy spyServPort(&config, &NetworkConfig::serverPortChanged);
    QSignalSpy spyListPort(&config, &NetworkConfig::listenPortChanged);
    QSignalSpy spyAudPort(&config, &NetworkConfig::audioPortChanged);
    QSignalSpy spyMetPort(&config, &NetworkConfig::metisPortChanged);
    QSignalSpy spySockBuf(&config, &NetworkConfig::socketBufferSizeChanged);

    config.setServerAddress(QStringLiteral("192.168.1.50"));
    config.setLocalAddress(QStringLiteral("192.168.1.100"));
    config.setServerPort(60000);
    config.setListenPort(12000);
    config.setAudioPort(16000);
    config.setMetisPort(2048);
    config.setSocketBufferSize(64);

    QCOMPARE(spyServAddr.count(), 1);
    QCOMPARE(spyLocAddr.count(), 1);
    QCOMPARE(spyServPort.count(), 1);
    QCOMPARE(spyListPort.count(), 1);
    QCOMPARE(spyAudPort.count(), 1);
    QCOMPARE(spyMetPort.count(), 1);
    QCOMPARE(spySockBuf.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["serverAddress"].toString(), QStringLiteral("192.168.1.50"));
    QCOMPARE(json["localAddress"].toString(), QStringLiteral("192.168.1.100"));
    QCOMPARE(json["serverPort"].toInt(), 60000);
    QCOMPARE(json["listenPort"].toInt(), 12000);
    QCOMPARE(json["audioPort"].toInt(), 16000);
    QCOMPARE(json["metisPort"].toInt(), 2048);
    QCOMPARE(json["socketBufferSize"].toInt(), 64);

    NetworkConfig config2;
    config2.load(json);

    QCOMPARE(config2.serverAddress(), QStringLiteral("192.168.1.50"));
    QCOMPARE(config2.localAddress(), QStringLiteral("192.168.1.100"));
    QCOMPARE(config2.serverPort(), static_cast<quint16>(60000));
    QCOMPARE(config2.listenPort(), static_cast<quint16>(12000));
    QCOMPARE(config2.audioPort(), static_cast<quint16>(16000));
    QCOMPARE(config2.metisPort(), static_cast<quint16>(2048));
    QCOMPARE(config2.socketBufferSize(), 64);

    TSDRDevice last;
    last.deviceClass = DeviceClass_HPSDR;
    last.deviceType = QStringLiteral("Hermes");
    last.serialNumber = QStringLiteral("aa:bb:cc");
    last.label = QStringLiteral("Hermes-1");
    config.setLastDevice(last);
    QJsonObject withDevice;
    config.save(withDevice);
    QCOMPARE(withDevice["lastDevice"].toObject()["type"].toString(), QStringLiteral("Hermes"));
    NetworkConfig config3;
    config3.load(withDevice);
    QCOMPARE(config3.lastDevice().deviceClass, DeviceClass_HPSDR);
    QCOMPARE(config3.lastDevice().deviceType, QStringLiteral("Hermes"));
    QCOMPARE(config3.lastDevice().serialNumber, QStringLiteral("aa:bb:cc"));
    QCOMPARE(config3.lastDevice().label, QStringLiteral("Hermes-1"));
}

void ConfigJsonTests::testAudioConfigJson() {
    AudioConfig config;
    QCOMPARE(config.mainVolume(), 0.1f);
    QCOMPARE(config.rxEqEnabled(), false);

    QSignalSpy spyVol(&config, &AudioConfig::mainVolumeChanged);
    QSignalSpy spyRxEq(&config, &AudioConfig::rxEqEnabledChanged);
    QSignalSpy spyEmnr(&config, &AudioConfig::emnrPost2Changed);

    config.setMainVolume(0.5f);
    config.setRxEqEnabled(true);
    config.setRxEqCurveDeg(2);
    config.setEmnrPost2Enabled(true);
    config.setEmnrPost2Factor(20.0);
    config.setEmnrPost2Nlevel(18.0);
    config.setEmnrPost2Taper(10.0);
    config.setEmnrPost2Rate(3.5);

    QCOMPARE(spyVol.count(), 1);
    QCOMPARE(spyRxEq.count(), 1);
    QCOMPARE(spyEmnr.count(), 5);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["mainVolume"].toDouble(), 0.5);
    QCOMPARE(json["rxEqEnabled"].toBool(), true);
    QCOMPARE(json["rxEqCurveDeg"].toInt(), 2);
    QCOMPARE(json["emnrPost2Enabled"].toBool(), true);
    QCOMPARE(json["emnrPost2Factor"].toDouble(), 20.0);
    QVERIFY(!json.contains("driveLevel"));
    QVERIFY(!json.contains("micSource"));

    AudioConfig config2;
    config2.load(json);

    QCOMPARE(config2.mainVolume(), 0.5f);
    QCOMPARE(config2.rxEqEnabled(), true);
    QCOMPARE(config2.rxEqCurveDeg(), 2);
    QCOMPARE(config2.emnrPost2Enabled(), true);
    QCOMPARE(config2.emnrPost2Factor(), 20.0);
    QCOMPARE(config2.emnrPost2Nlevel(), 18.0);
    QCOMPARE(config2.emnrPost2Taper(), 10.0);
    QCOMPARE(config2.emnrPost2Rate(), 3.5);
}

void ConfigJsonTests::testCWConfigJson() {
    CWConfig config;
    QCOMPARE(config.internalCw(), 1);
    QCOMPARE(config.keyerSpeed(), 20);

    QSignalSpy spyInternalCw(&config, &CWConfig::internalCwChanged);
    QSignalSpy spyKeyReversed(&config, &CWConfig::keyReversedChanged);
    QSignalSpy spyKeyerSpacing(&config, &CWConfig::keyerSpacingChanged);
    QSignalSpy spyKeyerSpeed(&config, &CWConfig::keyerSpeedChanged);
    QSignalSpy spyKeyerMode(&config, &CWConfig::keyerModeChanged);
    QSignalSpy spySidetoneVolume(&config, &CWConfig::sidetoneVolumeChanged);
    QSignalSpy spySidetoneFreq(&config, &CWConfig::sidetoneFreqChanged);
    QSignalSpy spyPttDelay(&config, &CWConfig::pttDelayChanged);
    QSignalSpy spyHangTime(&config, &CWConfig::hangTimeChanged);
    QSignalSpy spyKeyerWeight(&config, &CWConfig::keyerWeightChanged);

    config.setInternalCw(0);
    config.setKeyReversed(1);
    config.setKeyerSpacing(1);
    config.setKeyerSpeed(18);
    config.setKeyerMode(1);
    config.setSidetoneVolume(80);
    config.setSidetoneFreq(800);
    config.setPttDelay(50);
    config.setHangTime(100);
    config.setKeyerWeight(30);

    QCOMPARE(spyInternalCw.count(), 1);
    QCOMPARE(spyKeyReversed.count(), 1);
    QCOMPARE(spyKeyerSpacing.count(), 1);
    QCOMPARE(spyKeyerSpeed.count(), 1);
    QCOMPARE(spyKeyerMode.count(), 1);
    QCOMPARE(spySidetoneVolume.count(), 1);
    QCOMPARE(spySidetoneFreq.count(), 1);
    QCOMPARE(spyPttDelay.count(), 1);
    QCOMPARE(spyHangTime.count(), 1);
    QCOMPARE(spyKeyerWeight.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["internalCw"].toInt(), 0);
    QCOMPARE(json["keyReversed"].toInt(), 1);
    QCOMPARE(json["keyerSpacing"].toInt(), 1);
    QCOMPARE(json["keyerSpeed"].toInt(), 18);
    QCOMPARE(json["keyerMode"].toInt(), 1);
    QCOMPARE(json["sidetoneVolume"].toInt(), 80);
    QCOMPARE(json["sidetoneFreq"].toInt(), 800);
    QCOMPARE(json["pttDelay"].toInt(), 50);
    QCOMPARE(json["hangTime"].toInt(), 100);
    QCOMPARE(json["keyerWeight"].toInt(), 30);

    CWConfig config2;
    config2.load(json);

    QCOMPARE(config2.internalCw(), 0);
    QCOMPARE(config2.keyReversed(), 1);
    QCOMPARE(config2.keyerSpacing(), 1);
    QCOMPARE(config2.keyerSpeed(), 18);
    QCOMPARE(config2.keyerMode(), 1);
    QCOMPARE(config2.sidetoneVolume(), 80);
    QCOMPARE(config2.sidetoneFreq(), 800);
    QCOMPARE(config2.pttDelay(), 50);
    QCOMPARE(config2.hangTime(), 100);
    QCOMPARE(config2.keyerWeight(), 30);
}

void ConfigJsonTests::testHardwareConfigJson() {
    HardwareConfig config;
    QCOMPARE(config.checkFirmwareVersions(), true);
    QCOMPARE(config.devices().mercuryPresence, true);

    QSignalSpy spyHpsdrHardware(&config, &HardwareConfig::hpsdrHardwareChanged);
    QSignalSpy spyCheckFW(&config, &HardwareConfig::checkFirmwareVersionsChanged);
    QSignalSpy spySrc10(&config, &HardwareConfig::source10MhzChanged);
    QSignalSpy spySrc122(&config, &HardwareConfig::source122_88MhzChanged);
    QSignalSpy spyRxClass(&config, &HardwareConfig::rxClassChanged);
    QSignalSpy spyRxTiming(&config, &HardwareConfig::rxTimingChanged);
    QSignalSpy spyDevices(&config, &HardwareConfig::devicesChanged);

    config.setHpsdrHardware(1);
    config.setCheckFirmwareVersions(false);
    config.setSource10Mhz(1);
    config.setSource122_88Mhz(2);
    config.setRxClass(2);
    config.setRxTiming(1);

    THPSDRDevices d;
    d.mercuryPresence = false;
    d.penelopePresence = true;
    d.pennylanePresence = true;
    d.excaliburPresence = true;
    d.alexPresence = true;
    d.hermesPresence = true;
    d.metisPresence = true;
    config.setDevices(d);

    QCOMPARE(spyHpsdrHardware.count(), 1);
    QCOMPARE(spyCheckFW.count(), 1);
    QCOMPARE(spySrc10.count(), 1);
    QCOMPARE(spySrc122.count(), 1);
    QCOMPARE(spyRxClass.count(), 1);
    QCOMPARE(spyRxTiming.count(), 1);
    QCOMPARE(spyDevices.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["hpsdrHardware"].toInt(), 1);
    QCOMPARE(json["checkFirmwareVersions"].toBool(), false);
    QCOMPARE(json["source10Mhz"].toInt(), 1);
    QCOMPARE(json["source122_88Mhz"].toInt(), 2);
    QCOMPARE(json["rxClass"].toInt(), 2);
    QCOMPARE(json["rxTiming"].toInt(), 1);

    QJsonObject devObj = json["devices"].toObject();
    QCOMPARE(devObj["mercuryPresence"].toBool(), false);
    QCOMPARE(devObj["penelopePresence"].toBool(), true);
    QCOMPARE(devObj["pennylanePresence"].toBool(), true);
    QCOMPARE(devObj["excaliburPresence"].toBool(), true);
    QCOMPARE(devObj["alexPresence"].toBool(), true);
    QCOMPARE(devObj["hermesPresence"].toBool(), true);
    QCOMPARE(devObj["metisPresence"].toBool(), true);

    HardwareConfig config2;
    config2.load(json);

    QCOMPARE(config2.hpsdrHardware(), 1);
    QCOMPARE(config2.checkFirmwareVersions(), false);
    QCOMPARE(config2.source10Mhz(), 1);
    QCOMPARE(config2.source122_88Mhz(), 2);
    QCOMPARE(config2.rxClass(), 2);
    QCOMPARE(config2.rxTiming(), 1);
    QCOMPARE(config2.devices().mercuryPresence, false);
    QCOMPARE(config2.devices().penelopePresence, true);
    QCOMPARE(config2.devices().pennylanePresence, true);
    QCOMPARE(config2.devices().excaliburPresence, true);
    QCOMPARE(config2.devices().alexPresence, true);
    QCOMPARE(config2.devices().hermesPresence, true);
    QCOMPARE(config2.devices().metisPresence, true);

    config.setReceiverCount(4);
    config.setHwInterface(2);
    config.setDither(true);
    config.setRandom(true);
    QJsonObject json2;
    config.save(json2);
    QCOMPARE(json2["receiverCount"].toInt(), 4);
    QCOMPARE(json2["interface"].toInt(), 2);
    QCOMPARE(json2["dither"].toBool(), true);
    QCOMPARE(json2["random"].toBool(), true);
    HardwareConfig config3;
    config3.load(json2);
    QCOMPARE(config3.receiverCount(), 4);
    QCOMPARE(config3.hwInterface(), 2);
    QCOMPARE(config3.dither(), true);
    QCOMPARE(config3.random(), true);
}

void ConfigJsonTests::testAlexConfigJson() {
    AlexConfig config;
    QCOMPARE(config.manualFilterSelect(), false);
    QCOMPARE(config.attenuation(), 0);

    QSignalSpy spyManual(&config, &AlexConfig::manualFilterSelectChanged);
    QSignalSpy spyBypass(&config, &AlexConfig::bypassAllChanged);
    QSignalSpy spyAmp6m(&config, &AlexConfig::amp6mChanged);
    QSignalSpy spyHpf15(&config, &AlexConfig::hpf1_5MHzChanged);
    QSignalSpy spyAttn(&config, &AlexConfig::attenuationChanged);

    config.setManualFilterSelect(true);
    config.setBypassAll(true);
    config.setAmp6m(true);
    config.setHpf1_5MHz(true);
    config.setAttenuation(2);
    config.setHpfLoFrequency(0, 1600000L);
    config.setHpfHiFrequency(0, 5600000L);
    config.setLpfLoFrequency(0, 1850000L);
    const quint16 lpfBits = 0x4100; // LPF 160m + LPF 6m
    config.setAlexConfig(static_cast<quint16>(config.alexConfig() | lpfBits));
    QList<int> states;
    states << 33 << 34;
    config.setAlexStates(states);

    QCOMPARE(spyManual.count(), 1);
    QCOMPARE(spyBypass.count(), 1);
    QCOMPARE(spyAmp6m.count(), 1);
    QCOMPARE(spyHpf15.count(), 1);
    QCOMPARE(spyAttn.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["manualFilterSelect"].toBool(), true);
    QCOMPARE(json["bypassAll"].toBool(), true);
    QCOMPARE(json["amp6m"].toBool(), true);
    QCOMPARE(json["hpf1_5MHz"].toBool(), true);
    QCOMPARE(json["attenuation"].toInt(), 2);
    QCOMPARE(json["alexConfig"].toInt(), 0x410F);
    QCOMPARE(json["lpf160m"].toBool(), true);
    QCOMPARE(json["lpf6m"].toBool(), true);
    QVERIFY(json.contains("alexStates"));

    AlexConfig config2;
    config2.load(json);

    QCOMPARE(config2.manualFilterSelect(), true);
    QCOMPARE(config2.bypassAll(), true);
    QCOMPARE(config2.amp6m(), true);
    QCOMPARE(config2.hpf1_5MHz(), true);
    QCOMPARE(config2.attenuation(), 2);
    QCOMPARE(config2.hpfLoFrequencies().value(0), 1600000L);
    QCOMPARE(config2.hpfHiFrequencies().value(0), 5600000L);
    QCOMPARE(config2.lpfLoFrequencies().value(0), 1850000L);
    QCOMPARE(config2.alexConfig(), static_cast<quint16>(0x410F));
    QVERIFY((config2.alexConfig() & 0x4100) == 0x4100);
    QCOMPARE(config2.lpf160m(), true);
    QCOMPARE(config2.lpf6m(), true);
    QCOMPARE(config2.alexStates().size(), AlexConfig::kAlexStateCount);
    QCOMPARE(config2.alexStates().value(0), 33);
    QCOMPARE(config2.alexStates().value(1), 34);
}

void ConfigJsonTests::testTransmitConfigJson() {
    TransmitConfig config;
    QCOMPARE(config.micSource(), 1);
    QCOMPARE(config.driveLevel(), 100);
    QCOMPARE(config.fmDeviation(), 5000.0);
    QCOMPARE(config.cfcFreqs().size(), TransmitConfig::kCfcBands);
    QCOMPARE(config.cfcFreqs().value(0), 50.0);
    QCOMPARE(config.cfcFreqs().value(9), 3100.0);

    QSignalSpy spyMicSrc(&config, &TransmitConfig::micSourceChanged);
    QSignalSpy spyDrive(&config, &TransmitConfig::driveLevelChanged);
    QSignalSpy spyTune(&config, &TransmitConfig::tunePowerChanged);
    QSignalSpy spyPa(&config, &TransmitConfig::paEnabledChanged);
    QSignalSpy spyCarrier(&config, &TransmitConfig::amCarrierLevelChanged);
    QSignalSpy spyCompress(&config, &TransmitConfig::audioCompressionChanged);
    QSignalSpy spyDevia(&config, &TransmitConfig::fmDeviationChanged);
    QSignalSpy spyCtcss(&config, &TransmitConfig::ctcssToneHzChanged);

    config.setMicSource(2);
    config.setDriveLevel(80);
    config.setTunePower(15);
    config.setPaEnabled(false);
    config.setAmCarrierLevel(0.75);
    config.setAudioCompression(1);
    config.setFmDeviation(3000.0);
    config.setCtcssToneHz(100);
    config.setTxEqEnabled(true);
    config.setTxEqBand(0, 3);
    config.setCfcEnabled(true);
    config.setCfcLevel(1, 4.0);

    QCOMPARE(spyMicSrc.count(), 1);
    QCOMPARE(spyDrive.count(), 1);
    QCOMPARE(spyTune.count(), 1);
    QCOMPARE(spyPa.count(), 1);
    QCOMPARE(spyCarrier.count(), 1);
    QCOMPARE(spyCompress.count(), 1);
    QCOMPARE(spyDevia.count(), 1);
    QCOMPARE(spyCtcss.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["micSource"].toInt(), 2);
    QCOMPARE(json["driveLevel"].toInt(), 80);
    QCOMPARE(json["tunePower"].toInt(), 15);
    QCOMPARE(json["paEnabled"].toBool(), false);
    QCOMPARE(json["amCarrierLevel"].toDouble(), 0.75);
    QCOMPARE(json["audioCompression"].toInt(), 1);
    QCOMPARE(json["fmDeviation"].toDouble(), 3000.0);
    QCOMPARE(json["ctcssToneHz"].toInt(), 100);
    QCOMPARE(json["txEqEnabled"].toBool(), true);
    QCOMPARE(json["cfcEnabled"].toBool(), true);

    TransmitConfig config2;
    config2.load(json);

    QCOMPARE(config2.micSource(), 2);
    QCOMPARE(config2.driveLevel(), 80);
    QCOMPARE(config2.tunePower(), 15);
    QCOMPARE(config2.paEnabled(), false);
    QCOMPARE(config2.amCarrierLevel(), 0.75);
    QCOMPARE(config2.audioCompression(), 1);
    QCOMPARE(config2.fmDeviation(), 3000.0);
    QCOMPARE(config2.ctcssToneHz(), 100);
    QCOMPARE(config2.txEqEnabled(), true);
    QCOMPARE(config2.txEqBands().value(0), 3);
    QCOMPARE(config2.cfcEnabled(), true);
    QCOMPARE(config2.cfcLevels().value(1), 4.0);
}

void ConfigJsonTests::testFreeDVConfigJson() {
    FreeDVConfig config;
    QCOMPARE(config.defaultMode(), 0);
    QCOMPARE(config.sqThreshold(), 0.0f);

    QSignalSpy spyMode(&config, &FreeDVConfig::defaultModeChanged);
    QSignalSpy spySq(&config, &FreeDVConfig::sqThresholdChanged);
    QSignalSpy spyAuto(&config, &FreeDVConfig::autoSyncChanged);

    config.setDefaultMode(3); // 700C / 2400
    config.setSqThreshold(2.5f);
    config.setAutoSync(false);
    config.setRxMode(1, 4);

    QCOMPARE(spyMode.count(), 1);
    QCOMPARE(spySq.count(), 1);
    QCOMPARE(spyAuto.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["defaultMode"].toInt(), 3);
    QCOMPARE(json["sqThreshold"].toDouble(), 2.5);
    QCOMPARE(json["autoSync"].toBool(), false);

    FreeDVConfig config2;
    config2.load(json);

    QCOMPARE(config2.defaultMode(), 3);
    QCOMPARE(config2.sqThreshold(), 2.5f);
    QCOMPARE(config2.autoSync(), false);
    QCOMPARE(config2.rxMode(1), 4);
}

void ConfigJsonTests::testWindowConfigJson() {
    WindowConfig config;
    QCOMPARE(config.minimumWidgetWidth(), 300);
    QCOMPARE(config.minimumGroupBoxWidth(), 250);
    QCOMPARE(config.multiRxView(), 0);

    config.setMinimumWidgetWidth(280);
    config.setMinimumGroupBoxWidth(240);
    config.setMultiRxView(2);

    QJsonObject json;
    config.save(json);
    QCOMPARE(json["minimumWidgetWidth"].toInt(), 280);
    QCOMPARE(json["minimumGroupBoxWidth"].toInt(), 240);
    QCOMPARE(json["multiRxView"].toInt(), 2);

    WindowConfig loaded;
    loaded.load(json);
    QCOMPARE(loaded.minimumWidgetWidth(), 280);
    QCOMPARE(loaded.minimumGroupBoxWidth(), 240);
    QCOMPARE(loaded.multiRxView(), 2);

    loaded.setMinimumWidgetWidth(100);
    loaded.setMultiRxView(9);
    QCOMPARE(loaded.minimumWidgetWidth(), 300);
    QCOMPARE(loaded.multiRxView(), 0);
}

void ConfigJsonTests::testTciConfigJson() {
    TciConfig config;
    QCOMPARE(config.serverEnabled(), true);
    QCOMPARE(config.rxGain(), 1.0f);
    QCOMPARE(config.txGain(), 1.0f);

    config.setServerEnabled(false);
    config.setRxGain(1.5f);
    config.setTxGain(0.25f);

    QJsonObject json;
    config.save(json);
    QCOMPARE(json["enabled"].toBool(), false);
    QCOMPARE(json["rxGain"].toDouble(), 1.5);
    QCOMPARE(json["txGain"].toDouble(), 0.25);

    TciConfig loaded;
    loaded.load(json);
    QCOMPARE(loaded.serverEnabled(), false);
    QCOMPARE(loaded.rxGain(), 1.5f);
    QCOMPARE(loaded.txGain(), 0.25f);

    loaded.setRxGain(5.0f);
    QCOMPARE(loaded.rxGain(), 2.0f);
}

void ConfigJsonTests::testSoapyConfigJson() {
    SoapyConfig config;
    config.setRxAntenna(QStringLiteral("LNAW"));
    config.setTxAntenna(QStringLiteral("BAND1"));
    config.setLnaGain(18);
    config.setTiaGain(9);
    config.setPgaGain(6);
    config.setOverallGain(42);
    config.setAutoCalibrate(true);
    config.setIqBalance(false);

    QJsonObject json;
    config.save(json);
    QCOMPARE(json["rxAntenna"].toString(), QStringLiteral("LNAW"));
    QCOMPARE(json["txAntenna"].toString(), QStringLiteral("BAND1"));
    QCOMPARE(json["lnaGain"].toInt(), 18);
    QCOMPARE(json["overallGain"].toInt(), 42);
    QCOMPARE(json["autoCalibrate"].toBool(), true);
    QCOMPARE(json["iqBalance"].toBool(), false);

    SoapyConfig loaded;
    loaded.load(json);
    QCOMPARE(loaded.rxAntenna(), QStringLiteral("LNAW"));
    QCOMPARE(loaded.txAntenna(), QStringLiteral("BAND1"));
    QCOMPARE(loaded.lnaGain(), 18);
    QCOMPARE(loaded.tiaGain(), 9);
    QCOMPARE(loaded.pgaGain(), 6);
    QCOMPARE(loaded.overallGain(), 42);
    QCOMPARE(loaded.autoCalibrate(), true);
    QCOMPARE(loaded.iqBalance(), false);
}

void ConfigJsonTests::testWidebandConfigJson() {
    WidebandConfig config;
    QCOMPARE(config.dataEnabled(), true);
    QCOMPARE(config.displayEnabled(), false);

    config.setDataEnabled(true);
    config.setDisplayEnabled(true);
    config.setAveraging(false);
    config.setAveragingCnt(12);
    config.setdBmScaleMin(-130);
    config.setdBmScaleMax(-20);
    config.setPanMode(FilledLine);

    QJsonObject json;
    config.save(json);
    QCOMPARE(json["data"].toBool(), true);
    QCOMPARE(json["display"].toBool(), true);
    QCOMPARE(json["averaging"].toBool(), false);
    QCOMPARE(json["averagingCnt"].toInt(), 12);
    QCOMPARE(json["dBmScaleMin"].toDouble(), -130.0);
    QCOMPARE(json["dBmScaleMax"].toDouble(), -20.0);
    QCOMPARE(json["panMode"].toInt(), static_cast<int>(FilledLine));

    WidebandConfig loaded;
    loaded.load(json);
    QCOMPARE(loaded.dataEnabled(), true);
    QCOMPARE(loaded.displayEnabled(), true);
    QCOMPARE(loaded.averaging(), false);
    QCOMPARE(loaded.averagingCnt(), 12);
    QCOMPARE(loaded.dBmScaleMin(), -130.0);
    QCOMPARE(loaded.dBmScaleMax(), -20.0);
    QCOMPARE(loaded.panMode(), FilledLine);
}

void ConfigJsonTests::testPennyConfigJson() {
    PennyConfig config;
    QCOMPARE(config.ocEnabled(), false);
    QCOMPARE(config.rxJ6().size(), PennyConfig::kPinCount);

    QList<int> rx = config.rxJ6();
    QList<int> tx = config.txJ6();
    rx[m40] = 5;
    tx[m20] = 7;
    config.setOcEnabled(true);
    config.setRxJ6(rx);
    config.setTxJ6(tx);

    QJsonObject json;
    config.save(json);
    QCOMPARE(json["ocEnabled"].toBool(), true);
    QCOMPARE(json["rxJ6"].toArray().size(), PennyConfig::kPinCount);
    QCOMPARE(json["rxJ6"].toArray().at(m40).toInt(), 5);
    QCOMPARE(json["txJ6"].toArray().at(m20).toInt(), 7);

    PennyConfig loaded;
    loaded.load(json);
    QCOMPARE(loaded.ocEnabled(), true);
    QCOMPARE(loaded.rxJ6().at(m40), 5);
    QCOMPARE(loaded.txJ6().at(m20), 7);
}

void ConfigJsonTests::testSettingsFullJsonRoundtrip() {
    Settings::delete_instance();
    Settings* settings = Settings::instance();

    settings->setCallsign(QStringLiteral("ZL2BRG"));
    settings->setServerAddr(QStringLiteral("192.168.1.100"));
    settings->setServerPort(50000);
    settings->setDriveLevel(75);
    settings->setAlexConfiguration(0x410F);
    settings->setAlexState(0, 34);
    settings->setFreeDVMode(0, 2);
    settings->cwConfig()->setKeyerSpeed(26);
    settings->transmitConfig()->setAudioCompression(4);
    settings->freeDVConfig()->setDefaultMode(2);
    settings->setMultiRxView(1);
    settings->setTciServerEnabled(false);
    settings->setTciRxGain(1.25f);
    settings->setSoapyRxAntenna(QStringLiteral("RX2"));
    settings->setSoapyLnaGain(22);
    settings->setSampleRate(96000);
    settings->setDither(1);
    settings->setRandom(1);
    settings->setReceivers(2);
    settings->setPennyOCEnabled(true);
    settings->setRxJ6Pin(m40, 5);
    settings->setWidebandData(true);
    settings->setWidebandStatus(true);
    settings->setWidebanddBmScaleMin(-130.0);
    settings->setAnf(0, true);
    settings->setdBmPanScaleMin(0, -95.5);

    const QJsonObject json = settings->toJson();
    QCOMPARE(json["schemaVersion"].toInt(), kSettingsJsonSchemaVersion);
    QCOMPARE(json["callsign"].toString(), QStringLiteral("ZL2BRG"));
    QCOMPARE(json["network"].toObject()["serverAddress"].toString(), QStringLiteral("192.168.1.100"));
    QCOMPARE(json["network"].toObject()["serverPort"].toInt(), 50000);
    QCOMPARE(json["transmit"].toObject()["driveLevel"].toInt(), 75);
    QCOMPARE(json["alex"].toObject()["alexConfig"].toInt(), 0x410F);
    QCOMPARE(json["cw"].toObject()["keyerSpeed"].toInt(), 26);
    QCOMPARE(json["transmit"].toObject()["audioCompression"].toInt(), 4);
    QCOMPARE(json["freedv"].toObject()["defaultMode"].toInt(), 2);
    QCOMPARE(json["server"].toObject()["sampleRate"].toInt(), 96000);
    QCOMPARE(json["hardware"].toObject()["dither"].toBool(), true);
    QCOMPARE(json["hardware"].toObject()["receiverCount"].toInt(), 2);
    QCOMPARE(json["penny"].toObject()["ocEnabled"].toBool(), true);
    QCOMPARE(json["penny"].toObject()["rxJ6"].toArray().at(m40).toInt(), 5);
    QCOMPARE(json["wideband"].toObject()["dBmScaleMin"].toDouble(), -130.0);
    QCOMPARE(json["receivers"].toArray().at(0).toObject()["anf"].toBool(), true);

    // Reset settings and reload from JSON
    Settings::delete_instance();
    Settings* settings2 = Settings::instance();
    QVERIFY(settings2->fromJson(json));

    QCOMPARE(settings2->getCallsign(), QStringLiteral("ZL2BRG"));
    QCOMPARE(settings2->networkConfig()->serverAddress(), QStringLiteral("192.168.1.100"));
    QCOMPARE(settings2->networkConfig()->serverPort(), 50000);
    QCOMPARE(settings2->getServerPort(), static_cast<quint16>(50000));
    QCOMPARE(settings2->getServerPort(), settings2->networkConfig()->serverPort());
    QCOMPARE(settings2->getDriveLevel(), 75);
    QCOMPARE(settings2->getDriveLevel(), settings2->transmitConfig()->driveLevel());
    QCOMPARE(settings2->getAlexConfig(), static_cast<quint16>(0x410F));
    QCOMPARE(settings2->getAlexConfig(), settings2->alexConfig()->alexConfig());
    QCOMPARE(settings2->getAlexStates().value(0), 34);
    QCOMPARE(settings2->getAlexStates(), settings2->alexConfig()->alexStates());
    QCOMPARE(settings2->cwConfig()->keyerSpeed(), 26);
    QCOMPARE(settings2->transmitConfig()->audioCompression(), 4);
    QCOMPARE(settings2->freeDVConfig()->defaultMode(), 2);
    QCOMPARE(settings2->getFreeDVMode(0), 2);
    QCOMPARE(settings2->getFreeDVMode(0), settings2->freeDVConfig()->rxMode(0));
    QCOMPARE(settings2->getMinimumWidgetWidth(), settings2->windowConfig()->minimumWidgetWidth());
    QCOMPARE(settings2->getMultiRxView(), 1);
    QCOMPARE(settings2->getMultiRxView(), settings2->windowConfig()->multiRxView());
    QCOMPARE(settings2->getTciServerEnabled(), false);
    QCOMPARE(settings2->getTciServerEnabled(), settings2->tciConfig()->serverEnabled());
    QCOMPARE(settings2->getTciRxGain(), 1.25f);
    QCOMPARE(settings2->getTciRxGain(), settings2->tciConfig()->rxGain());
    QCOMPARE(settings2->getSoapyRxAntenna(), QStringLiteral("RX2"));
    QCOMPARE(settings2->getSoapyRxAntenna(), settings2->soapyConfig()->rxAntenna());
    QCOMPARE(settings2->getSoapyLnaGain(), 22);
    QCOMPARE(settings2->getSoapyLnaGain(), settings2->soapyConfig()->lnaGain());
    QCOMPARE(settings2->getSampleRate(), 96000);
    QCOMPARE(settings2->getMercuryDither(), 1);
    QCOMPARE(settings2->getMercuryRandom(), 1);
    QCOMPARE(settings2->getNumberOfReceivers(), 2);
    QCOMPARE(settings2->getPennyOCEnabled(), true);
    QCOMPARE(settings2->getRxJ6Pins().at(m40), 5);
    QCOMPARE(settings2->getWidebandData(), true);
    QCOMPARE(settings2->getWidebanddBmScaleMin(), -130.0);
    QCOMPARE(settings2->getReceiverDataList().at(0).anf, true);

    Settings::delete_instance();
}

void ConfigJsonTests::testFromJsonGettersMatchConfigObjects() {
    Settings::delete_instance();
    Settings *settings = Settings::instance();

    settings->setAlexConfiguration(0x410F);
    settings->setDriveLevel(75);
    settings->setServerPort(50000);
    settings->setFreeDVMode(0, 2);
    settings->setMultiRxView(2);
    settings->setTciTxGain(0.5f);
    settings->setSoapyOverallGain(55);
    settings->setVfoFrequency(0, 14150000);
    settings->setCtrFrequency(0, 14100000);

    const QJsonObject json = settings->toJson();
    Settings::delete_instance();

    Settings *loaded = Settings::instance();
    QVERIFY(loaded->fromJson(json));

    QCOMPARE(loaded->getAlexConfig(), loaded->alexConfig()->alexConfig());
    QCOMPARE(loaded->getAlexConfig(), static_cast<quint16>(0x410F));
    QCOMPARE(loaded->getDriveLevel(), loaded->transmitConfig()->driveLevel());
    QCOMPARE(loaded->getDriveLevel(), 75);
    QCOMPARE(loaded->get_tx_drivelevel(), loaded->transmitConfig()->driveLevel());
    QCOMPARE(loaded->getServerPort(), loaded->networkConfig()->serverPort());
    QCOMPARE(loaded->getServerPort(), static_cast<quint16>(50000));
    QCOMPARE(loaded->getFreeDVMode(0), loaded->freeDVConfig()->rxMode(0));
    QCOMPARE(loaded->getFreeDVMode(0), 2);
    QCOMPARE(loaded->getMinimumWidgetWidth(), loaded->windowConfig()->minimumWidgetWidth());
    QCOMPARE(loaded->getMultiRxView(), loaded->windowConfig()->multiRxView());
    QCOMPARE(loaded->getMultiRxView(), 2);
    QCOMPARE(loaded->getTciTxGain(), loaded->tciConfig()->txGain());
    QCOMPARE(loaded->getTciTxGain(), 0.5f);
    QCOMPARE(loaded->getSoapyOverallGain(), loaded->soapyConfig()->overallGain());
    QCOMPARE(loaded->getSoapyOverallGain(), 55);
    QCOMPARE(loaded->getVfoFrequency(0), static_cast<qint64>(14150000));
    QCOMPARE(loaded->getCtrFrequency(0), static_cast<qint64>(14100000));
    QCOMPARE(loaded->getVfoFrequency(0), loaded->getReceiverDataList().at(0).vfoFrequency);
    QCOMPARE(loaded->getCtrFrequency(0), loaded->getReceiverDataList().at(0).ctrFrequency);
    loaded->receiverConfigs().at(0)->fromReceiver(loaded->getReceiverDataList().at(0));
    QCOMPARE(loaded->receiverConfigs().at(0)->vfoFrequency(), loaded->getVfoFrequency(0));
    QCOMPARE(loaded->receiverConfigs().at(0)->ctrFrequency(), loaded->getCtrFrequency(0));

    Settings::delete_instance();
}

void ConfigJsonTests::testSettingsSaveAndLoadJsonFile() {
    Settings::delete_instance();
    Settings* settings = Settings::instance();

    settings->setCallsign(QStringLiteral("W1AW"));
    settings->transmitConfig()->setFmDeviation(3500);

    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString jsonPath = tempDir.filePath("test_config.json");

    QVERIFY(settings->saveJson(jsonPath));
    QVERIFY(QFile::exists(jsonPath));

    Settings::delete_instance();
    Settings* settings2 = Settings::instance();
    QVERIFY(settings2->loadJson(jsonPath));

    QCOMPARE(settings2->getCallsign(), QStringLiteral("W1AW"));
    QCOMPARE(settings2->transmitConfig()->fmDeviation(), 3500);

    Settings::delete_instance();
}

void ConfigJsonTests::testSettingsJsonSchemaValidation() {
    Settings::delete_instance();
    Settings* settings = Settings::instance();

    // Empty JSON should fail gracefully
    QJsonObject emptyObj;
    QVERIFY(!settings->fromJson(emptyObj));

    // Invalid schemaVersion should fail
    QJsonObject invalidVersionObj;
    invalidVersionObj["schemaVersion"] = 0;
    QVERIFY(!settings->fromJson(invalidVersionObj));

    // Partial JSON with valid version should succeed using defaults
    QJsonObject partialObj;
    partialObj["schemaVersion"] = 1;
    partialObj["callsign"] = QStringLiteral("DX1TEST");
    QVERIFY(settings->fromJson(partialObj));
    QCOMPARE(settings->getCallsign(), QStringLiteral("DX1TEST"));

    Settings::delete_instance();
}

void ConfigJsonTests::testSettingsJsonModelHydration() {
    Settings::delete_instance();
    Settings* settings = Settings::instance();

    RadioModel radio;
    auto *slice0 = new SliceModel(0, &radio);
    radio.addSlice(slice0);
    settings->setRadioModel(&radio);

    QJsonObject json = settings->toJson();
    QJsonObject netObj = json["network"].toObject();
    netObj["serverPort"] = 45000;
    json["network"] = netObj;

    QJsonObject txObj = json["transmit"].toObject();
    txObj["audioCompression"] = 8;
    json["transmit"] = txObj;

    QJsonArray rxArray = json["receivers"].toArray();
    if (!rxArray.isEmpty()) {
        QJsonObject rx0 = rxArray[0].toObject();
        rx0["vfoFrequency"] = 14200000;
        rx0["vfoAFrequency"] = 14200000;
        rx0["ctrFrequency"] = 14180000;
        rx0["filterSlope"] = 2;
        rx0["agcMode"] = static_cast<int>(agcSLOW);
        rxArray[0] = rx0;
    }
    json["receivers"] = rxArray;

    QVERIFY(settings->fromJson(json));

    QCOMPARE(radio.transmit()->audioCompression(), 8);
    QCOMPARE(slice0->vfoAFrequency(), static_cast<qint64>(14200000));
    QCOMPARE(slice0->frequency(), static_cast<qint64>(14200000));
    QCOMPARE(slice0->centerFrequency(), static_cast<qint64>(14180000));
    QCOMPARE(slice0->filterSlope(), 2);
    QCOMPARE(slice0->agcMode(), agcSLOW);
    QCOMPARE(settings->getVfoFrequency(0), slice0->frequency());
    QCOMPARE(settings->getCtrFrequency(0), slice0->centerFrequency());

    settings->setRadioModel(nullptr);
    Settings::delete_instance();
}

QTEST_MAIN(ConfigJsonTests)
#include "config_json_tests.moc"
