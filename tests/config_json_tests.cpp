#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonObject>
#include <QColor>

#include "Settings/DisplayConfig.h"
#include "Settings/ReceiverConfig.h"
#include "Settings/NetworkConfig.h"
#include "Settings/AudioConfig.h"
#include "Settings/CWConfig.h"
#include "Settings/HardwareConfig.h"

class ConfigJsonTests : public QObject {
    Q_OBJECT

private slots:
    void testDisplayConfigJson();
    void testReceiverConfigJson();
    void testNetworkConfigJson();
    void testAudioConfigJson();
    void testCWConfigJson();
    void testHardwareConfigJson();
};

void ConfigJsonTests::testDisplayConfigJson() {
    DisplayConfig config;
    QCOMPARE(config.spectrumSize(), 4096);
    QCOMPARE(config.dBmDistScaleMin(), -20.0);

    QSignalSpy spySize(&config, &DisplayConfig::spectrumSizeChanged);
    QSignalSpy spyMin(&config, &DisplayConfig::dBmDistScaleMinChanged);
    QSignalSpy spyMax(&config, &DisplayConfig::dBmDistScaleMaxChanged);
    QSignalSpy spyHold(&config, &DisplayConfig::sMeterHoldTimeChanged);
    QSignalSpy spyColors(&config, &DisplayConfig::panadapterColorsChanged);

    config.setSpectrumSize(2048);
    config.setdBmDistScaleMin(-40.0);
    config.setdBmDistScaleMax(80.0);
    config.setSMeterHoldTime(1000);

    TPanadapterColors colors;
    colors.panBackgroundColor = Qt::red;
    colors.waterfallColor = Qt::green;
    colors.panLineColor = Qt::blue;
    config.setPanadapterColors(colors);

    QCOMPARE(spySize.count(), 1);
    QCOMPARE(spyMin.count(), 1);
    QCOMPARE(spyMax.count(), 1);
    QCOMPARE(spyHold.count(), 1);
    QCOMPARE(spyColors.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["spectrumSize"].toInt(), 2048);
    QCOMPARE(json["dBmDistScaleMin"].toDouble(), -40.0);
    QCOMPARE(json["dBmDistScaleMax"].toDouble(), 80.0);
    QCOMPARE(json["sMeterHoldTime"].toInt(), 1000);

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
}

void ConfigJsonTests::testAudioConfigJson() {
    AudioConfig config;
    QCOMPARE(config.micSource(), 1);
    QCOMPARE(config.micGain(), 10.0);

    QSignalSpy spyMicSrc(&config, &AudioConfig::micSourceChanged);
    QSignalSpy spyMicDev(&config, &AudioConfig::micInputDevChanged);
    QSignalSpy spyDigDev(&config, &AudioConfig::digitalAudioInputDevChanged);
    QSignalSpy spyMicName(&config, &AudioConfig::micInputSourceNameChanged);
    QSignalSpy spyDigName(&config, &AudioConfig::digitalInputSourceNameChanged);
    QSignalSpy spyMicGain(&config, &AudioConfig::micGainChanged);
    QSignalSpy spyDrive(&config, &AudioConfig::driveLevelChanged);
    QSignalSpy spyPreemp(&config, &AudioConfig::fmPreemphasisChanged);
    QSignalSpy spyCarrier(&config, &AudioConfig::amCarrierLevelChanged);
    QSignalSpy spyCompress(&config, &AudioConfig::audioCompressionChanged);
    QSignalSpy spyDevia(&config, &AudioConfig::fmDeviationChanged);
    QSignalSpy spyVol(&config, &AudioConfig::mainVolumeChanged);

    config.setMicSource(2);
    config.setMicInputDev(1);
    config.setDigitalAudioInputDev(2);
    config.setMicInputSourceName(QStringLiteral("InputMic"));
    config.setDigitalInputSourceName(QStringLiteral("InputDig"));
    config.setMicGain(15.5);
    config.setDriveLevel(50);
    config.setFmPreemphasis(2);
    config.setPhaseRotator(0);
    config.setAmCarrierLevel(0.8);
    config.setAudioCompression(1);
    config.setFmDeviation(4500.0);
    config.setMainVolume(0.5f);
    config.setRxEqCurveDeg(2);
    config.setTxEqCurveDeg(3);
    config.setCfcEnabled(true);
    config.setCfcPeqEnabled(true);
    config.setCfcPrecomp(4.5);
    config.setCfcPrePeq(-6.0);
    config.setCfcCurveDeg(2);
    config.setCfcLevel(2, 5.0);
    config.setCfcPost(3, -2.0);
    config.setEmnrPost2Enabled(true);
    config.setEmnrPost2Factor(20.0);
    config.setEmnrPost2Nlevel(18.0);
    config.setEmnrPost2Taper(10.0);
    config.setEmnrPost2Rate(3.5);

    QCOMPARE(spyMicSrc.count(), 1);
    QCOMPARE(spyMicDev.count(), 1);
    QCOMPARE(spyDigDev.count(), 1);
    QCOMPARE(spyMicName.count(), 1);
    QCOMPARE(spyDigName.count(), 1);
    QCOMPARE(spyMicGain.count(), 1);
    QCOMPARE(spyDrive.count(), 1);
    QCOMPARE(spyPreemp.count(), 1);
    QCOMPARE(spyCarrier.count(), 1);
    QCOMPARE(spyCompress.count(), 1);
    QCOMPARE(spyDevia.count(), 1);
    QCOMPARE(spyVol.count(), 1);

    QJsonObject json;
    config.save(json);

    QCOMPARE(json["micSource"].toInt(), 2);
    QCOMPARE(json["micInputDev"].toInt(), 1);
    QCOMPARE(json["digitalAudioInputDev"].toInt(), 2);
    QCOMPARE(json["micInputSourceName"].toString(), QStringLiteral("InputMic"));
    QCOMPARE(json["digitalInputSourceName"].toString(), QStringLiteral("InputDig"));
    QCOMPARE(json["micGain"].toDouble(), 15.5);
    QCOMPARE(json["driveLevel"].toInt(), 50);
    QCOMPARE(json["fmPreemphasis"].toInt(), 2);
    QCOMPARE(json["amCarrierLevel"].toDouble(), 0.8);
    QCOMPARE(json["audioCompression"].toInt(), 1);
    QCOMPARE(json["fmDeviation"].toDouble(), 4500.0);
    QCOMPARE(json["mainVolume"].toDouble(), 0.5);
    QCOMPARE(json["rxEqCurveDeg"].toInt(), 2);
    QCOMPARE(json["txEqCurveDeg"].toInt(), 3);
    QCOMPARE(json["cfcEnabled"].toBool(), true);
    QCOMPARE(json["cfcPeqEnabled"].toBool(), true);
    QCOMPARE(json["cfcPrecomp"].toDouble(), 4.5);
    QCOMPARE(json["cfcPrePeq"].toDouble(), -6.0);
    QCOMPARE(json["cfcCurveDeg"].toInt(), 2);
    QCOMPARE(json["emnrPost2Enabled"].toBool(), true);
    QCOMPARE(json["emnrPost2Factor"].toDouble(), 20.0);
    QCOMPARE(json["emnrPost2Nlevel"].toDouble(), 18.0);
    QCOMPARE(json["emnrPost2Taper"].toDouble(), 10.0);
    QCOMPARE(json["emnrPost2Rate"].toDouble(), 3.5);

    AudioConfig config2;
    config2.load(json);

    QCOMPARE(config2.micSource(), 2);
    QCOMPARE(config2.micInputDev(), 1);
    QCOMPARE(config2.digitalAudioInputDev(), 2);
    QCOMPARE(config2.micInputSourceName(), QStringLiteral("InputMic"));
    QCOMPARE(config2.digitalInputSourceName(), QStringLiteral("InputDig"));
    QCOMPARE(config2.micGain(), 15.5);
    QCOMPARE(config2.driveLevel(), 50);
    QCOMPARE(config2.fmPreemphasis(), 2);
    QCOMPARE(config2.amCarrierLevel(), 0.8);
    QCOMPARE(config2.audioCompression(), 1);
    QCOMPARE(config2.fmDeviation(), 4500.0);
    QCOMPARE(config2.mainVolume(), 0.5f);
    QCOMPARE(config2.rxEqCurveDeg(), 2);
    QCOMPARE(config2.txEqCurveDeg(), 3);
    QCOMPARE(config2.cfcEnabled(), true);
    QCOMPARE(config2.cfcPeqEnabled(), true);
    QCOMPARE(config2.cfcPrecomp(), 4.5);
    QCOMPARE(config2.cfcPrePeq(), -6.0);
    QCOMPARE(config2.cfcCurveDeg(), 2);
    QCOMPARE(config2.cfcLevels().value(2), 5.0);
    QCOMPARE(config2.cfcPost().value(3), -2.0);
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
}

QTEST_MAIN(ConfigJsonTests)
#include "config_json_tests.moc"
