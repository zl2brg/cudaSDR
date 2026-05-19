/**
* @file  cusdr_settings.cpp
* @brief settings class for cuSDR
* @author by Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-11-18
*/

/*   
 *   Copyright 2010-2015 Hermann von Hasseln, DL3HVH
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#define LOG_SETTINGS

#include <QStandardPaths>
#include "cusdr_settings.h"

namespace {
bool sampleRateToParams(int rate, int &speed, int &outputIncrement) {
    switch (rate) {
        case 48000:
            speed = 0;
            outputIncrement = 1;
            return true;
        case 96000:
            speed = 1;
            outputIncrement = 2;
            return true;
        case 192000:
            speed = 2;
            outputIncrement = 4;
            return true;
        case 384000:
            speed = 3;
            outputIncrement = 8;
            return true;
        case 768000:
            speed = 4;
            outputIncrement = 16;
            return true;
        case 1536000:
            speed = 5;
            outputIncrement = 32;
            return true;
        default:
            return false;
    }
}
}

Settings *Settings::m_instance = nullptr;        /*!< set m_instance to NULL. */

/*!
	\class Settings
	\brief Settings class implements application specific and user defined variables for the application.
*/

Settings::Settings(QObject *parent)
        : QObject(parent), m_dataEngineState(QSDR::DataEngineDown), setLoaded(false), m_mainPower(false),
          m_manualSocketBufferSize(false), m_peakHold(false), m_packetsToggle(true), m_radioPopupVisible(false),
          m_maxFrequency(MAXFREQUENCY), m_minFrequency(0), m_hpsdrNetworkDevices(0),
          m_mercuryReceivers(1), m_currentReceiver(0) {
    m_devices.mercuryFWVersion = 0;
    m_networkConfig = new NetworkConfig(this);
    m_displayConfig = new DisplayConfig(this);
    m_hardwareConfig = new HardwareConfig(this);
    m_audioConfig = new AudioConfig(this);
    m_cwConfig = new CWConfig(this);

    for (int i = 0; i < MAX_RECEIVERS; ++i) {
        m_receiverConfigs.append(new ReceiverConfig(i, this));
    }

    qRegisterMetaType<QSDR::_Error>();
    qRegisterMetaType<QSDR::_DataEngineState>();
    qRegisterMetaType<QSDR::_ServerMode>();
    qRegisterMetaType<QSDR::_HWInterfaceMode>();
    qRegisterMetaType<HamBand>();
    qRegisterMetaType<DSPMode>();
    qRegisterMetaType<ADCMode>();
    qRegisterMetaType<AGCMode>();
    qRegisterMetaType<TDefaultFilterMode>();
    qRegisterMetaType<TNetworkDevicecard>();
    qRegisterMetaType<QList<TNetworkDevicecard> >();
    qRegisterMetaType<TSoapyDevice>();
    qRegisterMetaType<QList<TSoapyDevice> >();
    qRegisterMetaType<qVectorFloat>("qVectorFloat");

    startTime = QDateTime::currentDateTime();

    SETTINGS_DEBUG << "start at: " << qPrintable(startTime.toString());

    settingsFilename = "settings.ini";
    settings = new QSettings(QCoreApplication::applicationDirPath() + "/" + settingsFilename, QSettings::IniFormat);
    getConfigPath();
    m_titleString = "cudaSDR BETA";
    QFile File(":/cusdr_stylesheet.qss");
    if (File.open(QFile::ReadOnly)) {
        appStyleSheet = QLatin1String(File.readAll());
        File.close();
    } else {
        SETTINGS_DEBUG << "Failed to load stylesheet resource:" << File.errorString();
    }

#ifdef DEBUG
    m_titleString = "cudaSDR Debug BETA ";
#endif

    m_versionString = "v6.1.2 - ZL2BRG";

    qDebug() << qPrintable(m_titleString);

    // receiver data
    for (int i = 0; i < MAX_RECEIVERS; i++) {

        TReceiver receiverData;
        m_receiverDataList.append(receiverData);

        QString str = "receiver";
        QString num;
        num.setNum(i);
        str.append(num);

        m_rxStringList << str;

        m_receiverDataList[i].agcHangThreshold = 0.0;
        m_receiverDataList[i].peakHold = false;
        m_receiverDataList[i].fftFactor = 1;

        for (int j = 0; j < MAX_BANDS; j++) {

            m_receiverDataList[i].mercuryAttenuators << 0;
            m_receiverDataList[i].dBmPanScaleMinList << 0.0;
            m_receiverDataList[i].dBmPanScaleMaxList << 0.0;
            m_receiverDataList[i].dspModeList << (DSPMode) LSB;
        }

                // Initialize FreeDV/Codec2 mode list with default mode 0 (FREEDV_MODE_1600, 1600 bps)
                // Available modes: 0=1600bps, 1=1400bps, 2=1300bps, 3=700C bps, 4=2400bps, 5=3200bps
                m_freeDVModeList << 0; // FREEDV_MODE_1600
                m_freeDVSyncList << false;
                m_freeDVSnrList << 0.0f;
                m_freeDVRxFramesList << 0;
                m_freeDVTxFramesList << 0;

      }

    // Alex parameter configurations
    m_alexConfig = 0;

    for (int i = 0; i < 6; i++) {

        m_HPFLoFrequencyList.append((long) 0);
        m_HPFHiFrequencyList.append((long) 0);
    }

    for (int i = 0; i < 7; i++) {

        m_LPFLoFrequencyList.append((long) 0);
        m_LPFHiFrequencyList.append((long) 0);
    }

    // init alex states
    for (int i = 0; i < MAX_BANDS; i++) {

        m_alexStates << 0;
    }

    // Rx, Tx J6 pins list
    // Bands: 160m, 80m, 60m, 40m, 30m, 20m, 17m, 15m, 12m, 10m, 6m
    // Values: no pin, pins 1 to 7 (0..7)
    //
    // 0 0 0 0 0 0 0 0
    // | | | | | | |
    // | | | | | | |
    // | | | | | | +-------------- pin 1 (0 = deactivated, 1 = activated)
    // | | | | | +---------------- pin 2 (0 = deactivated, 1 = activated)
    // | | | | +------------------ pin 3 (0 = deactivated, 1 = activated)
    // | | | +-------------------- pin 4 (0 = deactivated, 1 = activated)
    // | | +---------------------- pin 5 (0 = deactivated, 1 = activated)
    // | +------------------------ pin 6 (0 = deactivated, 1 = activated)
    // +-------------------------- pin 7 (0 = deactivated, 1 = activated)

    for (int i = 0; i < MAX_BANDS - 1; i++) {

        m_rxJ6pinList << 0;
        m_txJ6pinList << 0;
    }


    m_bandList = getHamBandFrequencies();
    m_bandTextList = getHamBandText();
    m_defaultFilterList = getDefaultFilterFrequencies();

    m_transmitter.txAllowed = false;
}

Settings::~Settings()  {
  //  delete settings;
    //m_clDevices.clear();
}

int Settings::loadSettings() {

    QString str;
    int value;
    long lvalue;
    //QList<QString> bandList = HamBandStrings();

    // user's call sign
    str = settings->value("user/callSign", "Your Call sign").toString();
    //while (str.startsWith('\"')) str = str.right(str.length() - 1).trimmed();
    //while (str.endsWith('\"')) str = str.left(str.length() - 1).trimmed();

    m_callsignString = str;

    // Window settings
    value = settings->value("window/minimumWidgetWidth", 300).toInt();
    if (value < 235 || value > 350) m_minimumWidgetWidth = 300;
    else m_minimumWidgetWidth = value;

    value = settings->value("window/minimumGroupBoxWidth", 250).toInt();
    if (value < 230 || value > 295 || value > m_minimumWidgetWidth - 5) m_minimumGroupBoxWidth = 250;
    else m_minimumGroupBoxWidth = value;

    value = settings->value("window/multiRxView", 0).toInt();
    if (value < 0 || value > 2) m_multiRxView = 0;
    m_multiRxView = value;


    // network settings
    str = settings->value("network/server_ipAddress", "127.0.0.1").toString();
    while (str.startsWith('\"')) str = str.right(str.length() - 1).trimmed();
    while (str.endsWith('\"')) str = str.left(str.length() - 1).trimmed();
    m_networkConfig->setServerAddress(str);
    m_serverAddress = m_networkConfig->serverAddress();

    str = settings->value("network/hpsdr_local_ipAddress", "127.0.0.1").toString();
    while (str.startsWith('\"')) str = str.right(str.length() - 1).trimmed();
    while (str.endsWith('\"')) str = str.left(str.length() - 1).trimmed();
    m_networkConfig->setLocalAddress(str);
    m_hpsdrDeviceLocalAddr = m_networkConfig->localAddress();

    value = settings->value("network/server_port", 52685).toInt();
    if (value < 0 || value > 65535) value = 52685;
    m_networkConfig->setServerPort(static_cast<quint16>(value));
    m_serverPort = m_networkConfig->serverPort();

    value = settings->value("network/listen_port", 11000).toInt();
    if (value < 0 || value > 65535) value = 11000;
    m_networkConfig->setListenPort(static_cast<quint16>(value));
    m_listenerPort = m_networkConfig->listenPort();

    value = settings->value("network/audio_port", 15000).toInt();
    if (value < 0 || value > 65535) value = 15000;
    m_networkConfig->setAudioPort(static_cast<quint16>(value));
    m_audioPort = m_networkConfig->audioPort();

    value = settings->value("network/metis_port", 1024).toInt();
    if (value < 0 || value > 65535) value = 1024;
    m_networkConfig->setMetisPort(static_cast<quint16>(value));
    m_metisPort = m_networkConfig->metisPort();

    value = settings->value("network/socketBufferSize", 32).toInt();
    if (value != 16 && value != 32 && value != 64 && value != 128 && value != 256) value = 32;
    m_networkConfig->setSocketBufferSize(value);
    m_socketBufferSize = m_networkConfig->socketBufferSize();

    value = settings->value("hpsdr/receivers", 1).toInt();
    if (value < 1 || value > MAX_RECEIVERS) value = 1;
    m_mercuryReceivers = value;


    // HPSDR hardware
    value = settings->value("hpsdr/hardware", 0).toInt();
    if (value < 0 || value > 2) value = 0;
    m_hardwareConfig->setHpsdrHardware(value);
    m_hpsdrHardware = m_hardwareConfig->hpsdrHardware();

    THPSDRDevices devices = m_hardwareConfig->devices();

    str = settings->value("hpsdr/mercury", "true").toString();
    devices.mercuryPresence = (str == "true");

    str = settings->value("hpsdr/penelope", "false").toString();
    devices.penelopePresence = (str == "true");

    str = settings->value("hpsdr/pennylane", "false").toString();
    devices.pennylanePresence = (str == "true");

    str = settings->value("hpsdr/excalibur", "false").toString();
    devices.excaliburPresence = (str == "true");

    str = settings->value("hpsdr/alex", "false").toString();
    devices.alexPresence = (str == "true");

    m_hardwareConfig->setDevices(devices);
    m_devices = m_hardwareConfig->devices();


//	str = settings->value("hpsdr/hermes", "false").toString();
//	if (str == "true")
//		m_devices.hermesPresence = true;
//	else
//		m_devices.hermesPresence = false;

    if (m_hpsdrHardware == 0) {

        str = settings->value("hpsdr/interface", "metis").toString();
        if (str == "metis")
            m_hwInterface = QSDR::Metis;
        else
            m_hwInterface = QSDR::NoInterfaceMode;
    } else if (m_hpsdrHardware == 1) {

        m_hwInterface = QSDR::Hermes;
    } else if (m_hpsdrHardware == 2) {

        m_hwInterface = QSDR::SoapySDR;
        m_currentSoapyDevice.label = settings->value("SoapySDR/label", "").toString();
        m_currentSoapyDevice.driver = settings->value("SoapySDR/driver", "").toString();
        m_currentSoapyDevice.serial = settings->value("SoapySDR/serial", "").toString();
    }

    str = settings->value("hpsdr/checkfw", "true").toString();
    m_hardwareConfig->setCheckFirmwareVersions(str == "true");
    m_checkFirmwareVersions = m_hardwareConfig->checkFirmwareVersions();

    //checkHPSDRDevices();

    value = settings->value("server/sample_rate", 48000).toInt();
    if ((value != 48000) & (value != 96000) & (value != 192000) & (value != 384000)) value = 48000;
    setSampleRate(value);

    str = settings->value("server/dither", "off").toString();
    if (str.toLower() == "on")
        m_mercuryDither = 1;
    else
        m_mercuryDither = 0;

    str = settings->value("server/random", "off").toString();
    if (str.toLower() == "on")
        m_mercuryRandom = 1;
    else
        m_mercuryRandom = 0;

    str = settings->value("server/10mhzsource", "mercury").toString();
    if (str == "atlas")
        value = 0;
    else if (str == "penelope")
        value = 1;
    else if (str == "mercury")
        value = 2;
    else if (str == "none")
        value = 3;
    else
        value = 2;
    m_hardwareConfig->setSource10Mhz(value);
    m_10MHzSource = m_hardwareConfig->source10Mhz();

    str = settings->value("server/122_88mhzsource", "mercury").toString();
    if (str == "penelope")
        value = 0;
    else
        value = 1;
    m_hardwareConfig->setSource122_88Mhz(value);
    m_122_8MHzSource = m_hardwareConfig->source122_88Mhz();

    str = settings->value("server/mic_source", "penelope").toString();
    if (str == "janus")
        value = 0;
    else
        value = 1;
    m_audioConfig->setMicSource(value);
    m_micSource = m_audioConfig->micSource();

    m_audioConfig->setMicInputDev(settings->value("mic_InputDevice",0).toInt());
    m_micInputDev = m_audioConfig->micInputDev();

    m_audioConfig->setDigitalAudioInputDev(settings->value("digital_audio_InputDevice",0).toInt());
    m_digitalAudioInputDev = m_audioConfig->digitalAudioInputDev();

    m_audioConfig->setMicInputSourceName(settings->value("mic_input_source",
                                           (m_micInputDev > 0) ? QString("default") : QString()).toString());
    m_micInputSourceName = m_audioConfig->micInputSourceName();

    m_audioConfig->setDigitalInputSourceName(settings->value("digital_input_source",
                                               (m_digitalAudioInputDev > 0) ? QString("default") : QString("none")).toString());
    m_digitalInputSourceName = m_audioConfig->digitalInputSourceName();

    if (m_micInputSourceName.isEmpty()) {
        // Legacy configs only stored an index; prefer a working host mic path.
        m_audioConfig->setMicInputSourceName("default");
        m_micInputSourceName = m_audioConfig->micInputSourceName();
        m_audioConfig->setMicInputDev(1);
        m_micInputDev = m_audioConfig->micInputDev();
    }

    if (m_digitalInputSourceName.isEmpty()) {
        m_audioConfig->setDigitalInputSourceName((m_digitalAudioInputDev > 0) ? QString("default") : QString("none"));
        m_digitalInputSourceName = m_audioConfig->digitalInputSourceName();
    }
    m_audioConfig->setMicGain(settings->value("micGain", 0).toDouble());
    m_micGain = m_audioConfig->micGain();

    m_audioConfig->setDriveLevel(settings->value("driveLevel",0).toInt());
    m_drivelevel = m_audioConfig->driveLevel();

    m_repeaterOffset =  settings->value("repeater_offset",0).toDouble();
    m_audioConfig->setFmPreemphasis(settings->value("fm_preemphesize",0).toInt());
    m_fmPremphasize = m_audioConfig->fmPreemphasis();

    m_audioConfig->setAmCarrierLevel(settings->value("am_carrierlevel",0.5).toDouble());
    m_amCarrierLevel = m_audioConfig->amCarrierLevel();

    m_audioConfig->setAudioCompression(settings->value("audiocompression",0).toInt());
    m_audioCompression = m_audioConfig->audioCompression();

    m_audioConfig->setFmDeviation(settings->value("fmdeveation",5000).toDouble());
    m_fmDeveation = m_audioConfig->fmDeviation();

    str = settings->value("cw/internal", "off").toString();
    qDebug() << "internal" << str;
    m_cwConfig->setInternalCw(str.toLower() == "on" ? 1 : 0);
    m_internal_cw = m_cwConfig->internalCw();

    str = settings->value("cw/key_reversed", "off").toString();
    m_cwConfig->setKeyReversed(str.toLower() == "on" ? 1 : 0);
    m_cw_key_reversed = m_cwConfig->keyReversed();

    str = settings->value("cw/key_spacing", "off").toString();
    m_cwConfig->setKeyerSpacing(str.toLower() == "on" ? 1 : 0);
    m_cw_keyer_spacing = m_cwConfig->keyerSpacing();

    m_cwConfig->setKeyerSpeed(settings->value("cw/keyer_speed",12).toInt());
    m_cw_keyer_speed = m_cwConfig->keyerSpeed();

    m_cwConfig->setKeyerMode(settings->value("cw/keyer_mode",0).toInt());
    m_cw_keyer_mode = m_cwConfig->keyerMode();

    m_cwConfig->setSidetoneVolume(settings->value("cw/sidetone_volume",64).toInt());
    m_cw_sidetone_volume = m_cwConfig->sidetoneVolume();

    m_cwConfig->setSidetoneFreq(settings->value("cw/sidetone_freq",1000).toInt());
    m_cw_sidetone_freq = m_cwConfig->sidetoneFreq();

    m_cwConfig->setPttDelay(settings->value("cw/ptt_delay",32).toInt());
    m_cw_ptt_delay = m_cwConfig->pttDelay();

    m_cwConfig->setHangTime(settings->value("cw/hang_time",32).toInt());
    m_cw_hang_time = m_cwConfig->hangTime();

    m_cwConfig->setKeyerWeight(settings->value("cw/keyer_weight",20).toInt());
    m_cw_keyer_weight = m_cwConfig->keyerWeight();



    m_spectrumSize = 0;


    str = settings->value("server/class", 0).toString();
    m_RxClass = (str.toLower() == "E");
    if (m_RxClass)
        m_RxClass = 1;
    else
        m_RxClass = 0;

    value = settings->value("server/timing", 0).toInt();
    if (value < 0 || value > 1) value = 0;
    m_RxTiming = value;

    value = settings->value("server/mainVolume", 10).toInt();
    if (value < 0) value = 0;
    if (value > 100) value = 100;
    m_audioConfig->setMainVolume(value / 100.0f);
    m_mainVolume = m_audioConfig->mainVolume();

    str = settings->value("server/mode", "sdr").toString();
    if (str == "sdr") {

        m_serverMode = QSDR::SDRMode;
        setSpectrumSize(4096);

    } else {

        m_serverMode = QSDR::SDRMode;
        setSpectrumSize(4096);
    }


//	value = settings->value("server/mouseWheelFreqStep", 1000).toInt();
//	if ((value != 1) & (value != 10) & (value != 100) & (value != 1000) & (value != 10000) & (value != 100000) &
//		(value != 5) & (value != 50) & (value != 500) & (value != 5000) & (value != 50000) & (value != 500000))
//		value = 100;
//	m_mouseWheelFreqStep = (double)value;

    //******************************************************************
    // Alexiares data settings
    // m_alexConfig (qint16):
    //
    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    //   | | | | | | | | | | | | | | |
    //   | | | | | | | | | | | | | | +-----Alex   - manual HPF/LPF filter select (0 = disable, 1 = enable)
    //   | | | | | | | | | | | | | +------ Alex   -	Bypass all HPFs   (0 = disable, 1 = enable)*
    //   | | | | | | | | | | | | +-------- Alex   -	6M low noise amplifier (0 = disable, 1 = enable)*
    //   | | | | | | | | | | | +---------- Alex   -	select 1.5MHz HPF (0 = disable, 1 = enable)*
    //   | | | | | | | | | | +------------ Alex   -	select 6.5MHz HPF (0 = disable, 1 = enable)*
    //   | | | | | | | | | +-------------- Alex   -	select 9.5MHz HPF (0 = disable, 1 = enable)*
    //   | | | | | | | | +---------------- Alex   -	select 13MHz  HPF (0 = disable, 1 = enable)*
    //   | | | | | | | +------------------ Alex   -	select 20MHz  HPF (0 = disable, 1 = enable)*
    //   | | | | | | +-------------------- Alex   - select 160m   LPF (0 = disable, 1 = enable)*
    //   | | | | | +---------------------- Alex   - select 80m    LPF (0 = disable, 1 = enable)*
    //   | | | | +------------------------ Alex   - select 60/40m LPF (0 = disable, 1 = enable)*
    //   | | | +-------------------------- Alex   - select 30/20m LPF (0 = disable, 1 = enable)*
    //   | | +---------------------------- Alex   - select 17/15m LPF (0 = disable, 1 = enable)*
    //   | +------------------------------ Alex   - select 12/10m LPF (0 = disable, 1 = enable)*
    //   +-------------------------------- Alex   - select 6m     LPF (0 = disable, 1 = enable)*

    m_alexConfig = 0;
    double fLo;
    double fHi;

    str = settings->value("alex/manual", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x01;

    str = settings->value("alex/bypassAll", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x02;

    str = settings->value("alex/amp6m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x04;

    fLo = settings->value("alex/amp6mLo", 50000000).toDouble();
    if ((fLo < 49000000) || (fLo > 52500000)) fLo = 50000000;

    fHi = settings->value("alex/amp6mHi", 54000000).toDouble();
    if ((fHi < 52500000) || (fHi > 55000000)) fHi = 54000000;

    m_HPFLoFrequencyList[5] = (long) fLo;
    m_HPFHiFrequencyList[5] = (long) fHi;


    str = settings->value("alex/hpf1_5MHz", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x08;

    fLo = settings->value("alex/hpf1_5MHzLo", 1500000).toDouble();
    if ((fLo < 0) || (fLo > 2000000)) fLo = 1500000;

    fHi = settings->value("alex/hpf1_5MHzHi", 5500000).toDouble();
    if ((fHi < 1600000) || (fHi > 6000000)) fHi = 5500000;

    m_HPFLoFrequencyList[0] = (long) fLo;
    m_HPFHiFrequencyList[0] = (long) fHi;


    str = settings->value("alex/hpf6_5MHz", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x10;

    fLo = settings->value("alex/hpf6_5MHzLo", 7000000).toDouble();
    if ((fLo < 6000000) || (fLo > 8000000)) fLo = 7000000;

    fHi = settings->value("alex/hpf6_5MHzHi", 7300000).toDouble();
    if ((fHi < 7000000) || (fHi > 9500000)) fHi = 7300000;

    m_HPFLoFrequencyList[1] = (long) fLo;
    m_HPFHiFrequencyList[1] = (long) fHi;


    str = settings->value("alex/hpf9_5MHz", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x20;

    fLo = settings->value("alex/hpf9_5MHzLo", 10100000).toDouble();
    if ((fLo < 9000000) || (fLo > 11000000)) fLo = 10100000;

    fHi = settings->value("alex/hpf9_5MHzHi", 10150000).toDouble();
    if ((fHi < 10000000) || (fHi > 13000000)) fHi = 10150000;

    m_HPFLoFrequencyList[2] = (long) fLo;
    m_HPFHiFrequencyList[2] = (long) fHi;


    str = settings->value("alex/hpf13MHz", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x40;

    fLo = settings->value("alex/hpf13MHzLo", 14000000).toDouble();
    if ((fLo < 12000000) || (fLo > 15000000)) fLo = 14000000;

    fHi = settings->value("alex/hpf13MHzHi", 18168000).toDouble();
    if ((fHi < 13700000) || (fHi > 19000000)) fHi = 18168000;

    m_HPFLoFrequencyList[3] = (long) fLo;
    m_HPFHiFrequencyList[3] = (long) fHi;


    str = settings->value("alex/hpf20MHz", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x80;

    fLo = settings->value("alex/hpf20MHzLo", 21000000).toDouble();
    if ((fLo < 18000000) || (fLo > 25000000)) fLo = 21000000;

    fHi = settings->value("alex/hpf20MHzHi", 29700000).toDouble();
    if ((fHi < 25000000) || (fHi > 32000000)) fHi = 29700000;

    m_HPFLoFrequencyList[4] = (long) fLo;
    m_HPFHiFrequencyList[4] = (long) fHi;


    str = settings->value("alex/lpf160m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x100;

    fLo = settings->value("alex/lpf160mLo", 1800000).toDouble();
    if ((fLo < 0) || (fLo > 1900000)) fLo = 1800000;

    fHi = settings->value("alex/lpf160mHi", 2000000).toDouble();
    if ((fHi < 1000000) || (fHi > 3000000)) fHi = 2000000;

    m_LPFLoFrequencyList[0] = (long) fLo;
    m_LPFHiFrequencyList[0] = (long) fHi;


    str = settings->value("alex/lpf80m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x200;

    fLo = settings->value("alex/lpf80mLo", 3500000).toDouble();
    if ((fLo < 2000000) || (fLo > 4000000)) fLo = 3500000;

    fHi = settings->value("alex/lpf80mHi", 4000000).toDouble();
    if ((fHi < 2000000) || (fHi > 5000000)) fHi = 4000000;

    m_LPFLoFrequencyList[1] = (long) fLo;
    m_LPFHiFrequencyList[1] = (long) fHi;


    str = settings->value("alex/lpf60_40m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x400;

    fLo = settings->value("alex/lpf60_40mLo", 5330000).toDouble();
    if ((fLo < 5000000) || (fLo > 11000000)) fLo = 5330000;

    fHi = settings->value("alex/lpf60_40mHi", 7300000).toDouble();
    if ((fHi < 5000000) || (fHi > 8000000)) fHi = 7300000;

    m_LPFLoFrequencyList[2] = (long) fLo;
    m_LPFHiFrequencyList[2] = (long) fHi;


    str = settings->value("alex/lpf30_20m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x800;

    fLo = settings->value("alex/lpf30_20mLo", 10100000).toDouble();
    if ((fLo < 9000000) || (fLo > 15000000)) fLo = 10100000;

    fHi = settings->value("alex/lpf30_20mHi", 14350000).toDouble();
    if ((fHi < 9000000) || (fHi > 15000000)) fHi = 14350000;

    m_LPFLoFrequencyList[3] = (long) fLo;
    m_LPFHiFrequencyList[3] = (long) fHi;


    str = settings->value("alex/lpf17_15m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x1000;

    fLo = settings->value("alex/lpf17_15mLo", 18068000).toDouble();
    if ((fLo < 17000000) || (fLo > 22000000)) fLo = 18068000;

    fHi = settings->value("alex/lpf17_15mHi", 21450000).toDouble();
    if ((fHi < 17000000) || (fHi > 22000000)) fHi = 21450000;

    m_LPFLoFrequencyList[4] = (long) fLo;
    m_LPFHiFrequencyList[4] = (long) fHi;


    str = settings->value("alex/lpf12_10m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x2000;

    fLo = settings->value("alex/lpf12_10mLo", 24890000).toDouble();
    if ((fLo < 23000000) || (fLo > 30000000)) fLo = 24890000;

    fHi = settings->value("alex/lpf12_10mHi", 29700000).toDouble();
    if ((fHi < 23000000) || (fHi > 30000000)) fHi = 29700000;

    m_LPFLoFrequencyList[5] = (long) fLo;
    m_LPFHiFrequencyList[5] = (long) fHi;


    str = settings->value("alex/lpf6m", "off").toString();
    if (str.toLower() == "on")
        m_alexConfig |= 0x4000;

    fLo = settings->value("alex/lpf6mLo", 50000000).toDouble();
    if ((fLo < 30000000) || (fLo > 52000000)) fLo = 50000000;

    fHi = settings->value("alex/lpf6mHi", 54000000).toDouble();
    if ((fHi < 52000000) || (fHi > 66000000)) fHi = 54000000;

    m_LPFLoFrequencyList[6] = (long) fLo;
    m_LPFHiFrequencyList[6] = (long) fHi;


    SETTINGS_DEBUG << "Alex config: " << m_alexConfig;

    if (m_alexStates.length() == MAX_BANDS && m_bandList.length() == MAX_BANDS) {

        for (int i = 0; i < MAX_BANDS; i++) {

            str = "alex/state";
            str.append(m_bandList.at(i).bandString);

            value = settings->value(str, 33).toInt();
            setAlexState(i, value);
        }
    }

    //******************************************************************
    // Penny open collector settings

    str = settings->value("penny/OCenabled", "off").toString();
    if (str.toLower() == "on")
        m_pennyOCEnabled = true;
    else
        m_pennyOCEnabled = false;

    if (m_rxJ6pinList.length() == MAX_BANDS - 1 && m_txJ6pinList.length() == MAX_BANDS - 1 &&
        m_bandList.length() == MAX_BANDS) {

        for (int i = 0; i < MAX_BANDS - 1; i++) {

            str = "penny/rxState";
            str.append(m_bandList.at(i).bandString);

            value = settings->value(str, 0).toInt();
            if (value < 0 || value > 255) value = 0;
            setRxJ6Pin((HamBand) i, value);
        }

        for (int i = 0; i < MAX_BANDS - 1; i++) {

            str = "penny/txState";
            str.append(m_bandList.at(i).bandString);

            value = settings->value(str, 0).toInt();
            if (value < 0 || value > 255) value = 0;
            setTxJ6Pin((HamBand) i, value);
        }
    } else {

        qWarning() << "rxJ6pinList or txJ6pinList or bandList have wrong length!";
    }

    //******************************************************************
    // wideband settings

    str = settings->value("wideband/widebandData", "on").toString();
    if (str.toLower() == "on")
        m_widebandOptions.wideBandData = true;
    else if (str.toLower() == "off")
        m_widebandOptions.wideBandData = false;
    else
        m_widebandOptions.wideBandData = true;

    str = settings->value("wideband/widebandDisplay", "off").toString();
    if (str.toLower() == "on")
        m_widebandOptions.wideBandDisplayStatus = true;
    else if (str.toLower() == "off")
        m_widebandOptions.wideBandDisplayStatus = false;
    else
        m_widebandOptions.wideBandDisplayStatus = false;

    if (!m_widebandOptions.wideBandData) m_widebandOptions.wideBandDisplayStatus = false;

    str = settings->value("wideband/averaging", "on").toString();
    if (str.toLower() == "on")
        m_widebandOptions.averaging = true;
    else if (str.toLower() == "off")
        m_widebandOptions.averaging = false;
    else
        m_widebandOptions.averaging = true;

    value = settings->value("wideband/averagingCnt", 5).toInt();
    if ((value < 1) || (value > 100)) value = 5;
    m_widebandOptions.averagingCnt = value;

    value = settings->value("wideband/dBmWideBandScaleMin", -140).toInt();
    if ((value < -200) || (value > 0)) value = -140;
    m_widebandOptions.dBmWBScaleMin = (qreal) (1.0 * value);

    value = settings->value("wideband/dBmWideBandScaleMax", -10).toInt();
    if ((value < -100) || (value > 0)) value = -10;
    m_widebandOptions.dBmWBScaleMax = (qreal) (1.0 * value);

    str = settings->value("wideband/panMode", "LINE").toString();
    if (str == "LINE")
        m_widebandOptions.panMode = Line;
    else if (str == "FILLEDLINE")
        m_widebandOptions.panMode = FilledLine;
    else if (str == "SOLID")
        m_widebandOptions.panMode = Solid;

    //******************************************************************
    // receiver data settings

    for (int i = 0; i < MAX_RECEIVERS; i++) {

        QString cstr = m_rxStringList.at(i);
        cstr.append("/dspCore");

        str = settings->value(cstr, "qtdsp").toString();
        if (str == "qtdsp") {
            m_receiverConfigs[i]->setDspCore(QSDR::QtDSP);
        }
        m_receiverDataList[i].dspCore = m_receiverConfigs[i]->dspCore();
        if (m_receiverDataList[i].dspCore == QSDR::QtDSP) {
            setSpectrumSize(4096);
        }

        cstr = m_rxStringList.at(i);
        cstr.append("/centerFrequency");
        m_receiverConfigs[i]->setCtrFrequency(static_cast<long>(settings->value(cstr, (int) 7050000).toDouble()));
        m_receiverDataList[i].ctrFrequency = m_receiverConfigs[i]->ctrFrequency();

        cstr = m_rxStringList.at(i);
        cstr.append("/vfoFrequency");
        m_receiverConfigs[i]->setVfoFrequency(static_cast<long>(settings->value(cstr, (int) 7050000).toDouble()));
        m_receiverDataList[i].vfoFrequency = m_receiverConfigs[i]->vfoFrequency();

        cstr = m_rxStringList.at(i);
        cstr.append("/nr");
        m_receiverDataList[i].nr = settings->value(cstr, 1).toInt();
        cstr = m_rxStringList.at(i);
        cstr.append("/nbMode");
        m_receiverDataList[i].nbMode = settings->value(cstr, 1).toInt();

        cstr = m_rxStringList.at(i);
        cstr.append("/anf");
        m_receiverDataList[i].anf = settings->value(cstr, 1).toBool();

        cstr = m_rxStringList.at(i);
        cstr.append("/snb");
        m_receiverDataList[i].snb = settings->value(cstr, 1).toBool();

        cstr = m_rxStringList.at(i);
        cstr.append("/nr2_gain_method");
        m_receiverDataList[i].nr2_gain_method = settings->value(cstr, 1).toInt();

        cstr = m_rxStringList.at(i);
        cstr.append("/nr2_npe_method");
        m_receiverDataList[i].nr2_npe_method = settings->value(cstr, 1).toInt();

        cstr = m_rxStringList.at(i);
        cstr.append("/nr_agc");
        value = settings->value(cstr, 1).toInt();
        m_receiverDataList[i].nr_agc = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/nr2_ae");
        value = settings->value(cstr, 1).toBool();
        m_receiverDataList[i].nr2_ae = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/fftSize");

        value = settings->value(cstr, 1).toInt();
        m_receiverDataList[i].fftsize = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/PanAverageMode");

        value = settings->value(cstr, 1).toInt();
        m_receiverDataList[i].panAvMode = (PanAveragingMode) value;

        cstr = m_rxStringList.at(i);
        cstr.append("/PanDetectorMode");

        value = settings->value(cstr, 1).toInt();
        m_receiverDataList[i].panDetMode = (PanDetectorMode) value;

        cstr = m_rxStringList.at(i);
        cstr.append("/freqRulerPosition");

        value = settings->value(cstr, 5).toInt();
        if (value < 0) value = 0;
        if (value > 10) value = 10;
        m_receiverDataList[i].freqRulerPosition = value / 10.0f;

        cstr = m_rxStringList.at(i);
        cstr.append("/audioVolume");

        value = settings->value(cstr, 10).toInt();
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        m_receiverDataList[i].audioVolume = value / 100.0f;

        cstr = m_rxStringList.at(i);
        cstr.append("/mouseWheelFreqStep");

        value = settings->value(cstr, 100).toInt();
        if ((value != 1) & (value != 10) & (value != 100) & (value != 1000) & (value != 10000) & (value != 100000) &
            (value != 5) & (value != 50) & (value != 500) & (value != 5000) & (value != 50000) & (value != 500000))
            value = 100;
        m_receiverDataList[i].mouseWheelFreqStep = (qreal) value;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcGain");

        value = settings->value(cstr, 100).toInt();
        if (value < -20) value = -20;
        if (value > 120) value = 120;
        m_receiverDataList[i].acgGain = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcFixedGain");

        value = settings->value(cstr, 30).toInt();
        if (value < -20) value = -20;
        if (value > 50) value = 50;
        m_receiverDataList[i].agcFixedGain_dB = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcMaximumGain");

        value = settings->value(cstr, 30).toInt();
        if (value < -20) value = -20;
        if (value > 150) value = 150;
        m_receiverDataList[i].agcMaximumGain_dB = value - 0;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcSlope");

        value = settings->value(cstr, 0).toInt();
        if (value < 0) value = 0;
        if (value > 20) value = 20;
        m_receiverDataList[i].agcSlope = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcAttacktime");

        value = settings->value(cstr, 1).toInt();
        if (value < 1) value = 1;
        if (value > 10) value = 10;
        m_receiverDataList[i].agcAttackTime = value / 1000.0;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcDecaytime");

        value = settings->value(cstr, 250).toInt();
        if (value < 10) value = 10;
        if (value > 5000) value = 5000;
        m_receiverDataList[i].agcDecayTime = value / 1000.0;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcHangtime");

        value = settings->value(cstr, 100).toInt();
        if (value < 10) value = 10;
        if (value > 5000) value = 5000;
        m_receiverDataList[i].agcHangTime = value / 1000.0;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcLines");

        str = settings->value(cstr, "on").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].agcLines = true;
        else
            m_receiverDataList[i].agcLines = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/agcMode");

        str = settings->value(cstr, "MED").toString();
        if (str == "LONG")
            m_receiverDataList[i].agcMode = agcLONG;
        else if (str == "SLOW")
            m_receiverDataList[i].agcMode = agcSLOW;
        else if (str == "MED")
            m_receiverDataList[i].agcMode = agcMED;
        else if (str == "FAST")
            m_receiverDataList[i].agcMode = agcFAST;
        else
            m_receiverDataList[i].agcMode = agcOFF;

        if (str == "MED" || str == "FAST")
            m_receiverDataList[i].hangEnabled = false;
        else
            m_receiverDataList[i].hangEnabled = true;

        cstr = m_rxStringList.at(i);
        cstr.append("/adcMode");
        str = settings->value(cstr, "ADC1").toString();
        if (str == "ADC1")
            m_receiverDataList[i].adcMode = adc1;
        else
            m_receiverDataList[i].adcMode = adc2;

        cstr = m_rxStringList.at(i);
        cstr.append("/panMode");

        str = settings->value(cstr, "LINE").toString();
        if (str == "LINE")
            m_receiverDataList[i].panMode = Line;
        else if (str == "FILLEDLINE")
            m_receiverDataList[i].panMode = FilledLine;
        else if (str == "SOLID")
            m_receiverDataList[i].panMode = Solid;

        cstr = m_rxStringList.at(i);
        cstr.append("/waterfallMode");

        str = settings->value(cstr, "ENHANCED").toString();
        if (str == "SIMPLE")
            m_receiverDataList[i].waterfallMode = Simple;
        else if (str == "ENHANCED")
            m_receiverDataList[i].waterfallMode = Enhanced;

        cstr = m_rxStringList.at(i);
        cstr.append("/framesPerSecond");
        value = settings->value(cstr, 25).toInt();
        if (value < 0 || value > 200) value = 25;
        m_receiverDataList[i].framesPerSecond = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/waterfallOffsetLo");
        value = settings->value(cstr, -5).toInt();
        if ((value < -50) || (value > 50)) value = -5;
        m_receiverDataList[i].waterfallOffsetLo = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/waterfallOffsetHi");
        value = settings->value(cstr, 20).toInt();
        if ((value < -50) || (value > 50)) value = 20;
        m_receiverDataList[i].waterfallOffsetHi = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/filterHi");
        value = settings->value(cstr, -150).toInt();
        if (value < -20000 || value > 20000) value = -150;
        m_receiverDataList[i].filterHi = (qreal) (1.0f * value);

        cstr = m_rxStringList.at(i);
        cstr.append("/filterLo");
        value = settings->value(cstr, -3050).toInt();
        if (value < -20000 || value > 20000) value = -3050;
        m_receiverDataList[i].filterLo = (qreal) (1.0f * value);

        cstr = m_rxStringList.at(i);
        cstr.append("/averaging");
        str = settings->value(cstr, "on").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].spectrumAveraging = true;
        else
            m_receiverDataList[i].spectrumAveraging = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/averagingCnt");
        value = settings->value(cstr, 5).toInt();
        if (value < 1)  value = 100;
        m_receiverDataList[i].averagingCnt = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/grid");
        str = settings->value(cstr, "on").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].panGrid = true;
        else
            m_receiverDataList[i].panGrid = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/hairCross");
        str = settings->value(cstr, "off").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].hairCross = true;
        else
            m_receiverDataList[i].hairCross = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/panLocked");
        str = settings->value(cstr, "off").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].panLocked = true;
        else
            m_receiverDataList[i].panLocked = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/clickVFO");
        str = settings->value(cstr, "off").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].clickVFO = true;
        else
            m_receiverDataList[i].clickVFO = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency2200m");
        lvalue = settings->value(cstr, 135700).toInt();
        if ((lvalue < 135700) || (lvalue > 137800)) lvalue = 135700;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency2200m");
        lvalue = settings->value(cstr, 135700).toInt();
        if ((lvalue < 135700) || (lvalue > 137800)) lvalue = 135700;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency630m");
        lvalue = settings->value(cstr, 472000).toInt();
        if ((lvalue < 472000) || (lvalue > 479000)) lvalue = 472000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency630m");
        lvalue = settings->value(cstr, 472000).toInt();
        if ((lvalue < 472000) || (lvalue > 479000)) lvalue = 472000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency160m");
        lvalue = settings->value(cstr, 1810000).toInt();
        if ((lvalue < 1810000) || (lvalue > 2000000)) lvalue = 1810000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency160m");
        lvalue = settings->value(cstr, 1800000).toInt();
        if ((lvalue < 1810000) || (lvalue > 2000000)) lvalue = 1810000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency80m");
        lvalue = settings->value(cstr, 3500000).toInt();
        if ((lvalue < 3500000) || (lvalue > 3800000)) lvalue = 3500000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency80m");
        lvalue = settings->value(cstr, 3500000).toInt();
        if ((lvalue < 3500000) || (lvalue > 3800000)) lvalue = 3500000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency60m");
        lvalue = settings->value(cstr, 5260000).toInt();
        if ((lvalue < 5260000) || (lvalue > 5410000)) lvalue = 5260000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency60m");
        lvalue = settings->value(cstr, 5260000).toInt();
        if ((lvalue < 5260000) || (lvalue > 5410000)) lvalue = 5260000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency40m");
        lvalue = settings->value(cstr, 7000000).toInt();
        if ((lvalue < 7000000) || (lvalue > 7200000)) lvalue = 7000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency40m");
        lvalue = settings->value(cstr, 7000000).toInt();
        if ((lvalue < 7000000) || (lvalue > 7200000)) lvalue = 7000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency30m");
        lvalue = settings->value(cstr, 10100000).toInt();
        if ((lvalue < 10100000) || (lvalue > 10150000)) lvalue = 10100000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency30m");
        lvalue = settings->value(cstr, 10100000).toInt();
        if ((lvalue < 10100000) || (lvalue > 10150000)) lvalue = 10100000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency20m");
        lvalue = settings->value(cstr, 14000000).toInt();
        if ((lvalue < 14000000) || (lvalue > 14350000)) lvalue = 14000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency20m");
        lvalue = settings->value(cstr, 14000000).toInt();
        if ((lvalue < 14000000) || (lvalue > 14350000)) lvalue = 14000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency17m");
        lvalue = settings->value(cstr, 18068000).toInt();
        if ((lvalue < 18068000) || (lvalue > 18168000)) lvalue = 18068000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency17m");
        lvalue = settings->value(cstr, 18068000).toInt();
        if ((lvalue < 18068000) || (lvalue > 18168000)) lvalue = 18068000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency15m");
        lvalue = settings->value(cstr, 21000000).toInt();
        if ((lvalue < 21000000) || (lvalue > 21450000)) lvalue = 21000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency15m");
        lvalue = settings->value(cstr, 21000000).toInt();
        if ((lvalue < 21000000) || (lvalue > 21450000)) lvalue = 21000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency12m");
        lvalue = settings->value(cstr, 24890000).toInt();
        if ((lvalue < 24890000) || (lvalue > 24990000)) lvalue = 24890000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency12m");
        lvalue = settings->value(cstr, 24890000).toInt();
        if ((lvalue < 24890000) || (lvalue > 24990000)) lvalue = 24890000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency10m");
        lvalue = settings->value(cstr, 28000000).toInt();
        if ((lvalue < 28000000) || (lvalue > 29700000)) lvalue = 28000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency10m");
        lvalue = settings->value(cstr, 28000000).toInt();
        if ((lvalue < 28000000) || (lvalue > 29700000)) lvalue = 28000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency6m");
        lvalue = settings->value(cstr, 50000000).toInt();
        if ((lvalue < 50000000) || (lvalue > 54000000)) lvalue = 50000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency6m");
        lvalue = settings->value(cstr, 50000000).toInt();
        if ((lvalue < 50000000) || (lvalue > 54000000)) lvalue = 50000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency2m");
        lvalue = settings->value(cstr, 144000000).toInt();
        if ((lvalue < 144000000) || (lvalue > 148000000)) lvalue = 144000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency2m");
        lvalue = settings->value(cstr, 144000000).toInt();
        if ((lvalue < 144000000) || (lvalue > 148000000)) lvalue = 144000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency125cm");
        lvalue = settings->value(cstr, 222000000).toInt();
        if ((lvalue < 222000000) || (lvalue > 225000000)) lvalue = 222000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency125cm");
        lvalue = settings->value(cstr, 222000000).toInt();
        if ((lvalue < 222000000) || (lvalue > 225000000)) lvalue = 222000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency70cm");
        lvalue = settings->value(cstr, 420000000).toInt();
        if ((lvalue < 420000000) || (lvalue > 450000000)) lvalue = 420000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency70cm");
        lvalue = settings->value(cstr, 420000000).toInt();
        if ((lvalue < 420000000) || (lvalue > 450000000)) lvalue = 420000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency33cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

//RRK TODO FIX
        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency33cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency23cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency23cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency13cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency13cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency10cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency10cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency5cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency5cm");
        lvalue = settings->value(cstr, 902000000).toInt();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequencyGen");
        lvalue = settings->value(cstr, 1800000).toInt();
        if ((lvalue < 0) || (lvalue > 50000000)) lvalue = 3500000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequencyGen");
        lvalue = settings->value(cstr, 1800000).toInt();
        if ((lvalue < 0) || (lvalue > MAXFREQUENCY)) lvalue = 1800000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/centerFrequency");
        lvalue = settings->value(cstr, 3672000).toInt();
        if ((lvalue < 0) || (lvalue > MAXFREQUENCY)) lvalue = 1800000;
        m_receiverDataList[i].ctrFrequency = lvalue;

        setCtrFrequency(i, lvalue);

        cstr = m_rxStringList.at(i);
        cstr.append("/vfoFrequency");
        lvalue = settings->value(cstr, 3672000).toInt();
        if ((lvalue < 0) || (lvalue > 50000000)) lvalue = 3600000;
        m_receiverDataList[i].vfoFrequency = lvalue;

        setVfoFrequency(i, lvalue);


        if (m_receiverDataList[i].dspModeList.length() == MAX_BANDS && m_bandList.length() == MAX_BANDS) {

            for (int j = 0; j < MAX_BANDS; j++) {

                cstr = m_rxStringList.at(i);
                cstr.append("/dspMode");
                cstr.append(m_bandList.at(j).bandString);

                str = settings->value(cstr, "").toString();

                if (str == "USB")
                    m_receiverDataList[i].dspModeList[j] = USB;
                else if (str == "DSB")
                    m_receiverDataList[i].dspModeList[j] = DSB;
                else if (str == "CWL")
                    m_receiverDataList[i].dspModeList[j] = CWL;
                else if (str == "CWU")
                    m_receiverDataList[i].dspModeList[j] = CWU;
                else if (str == "FMN")
                    m_receiverDataList[i].dspModeList[j] = FMN;
                else if (str == "AM")
                    m_receiverDataList[i].dspModeList[j] = AM;
                else if (str == "DIGU")
                    m_receiverDataList[i].dspModeList[j] = DIGU;
                else if (str == "SPEC")
                    m_receiverDataList[i].dspModeList[j] = SPEC;
                else if (str == "DIGL")
                    m_receiverDataList[i].dspModeList[j] = DIGL;
                else if (str == "SAM")
                    m_receiverDataList[i].dspModeList[j] = SAM;
                else if (str == "FDV" || str == "DRM" || str == "FreeDV")
                    m_receiverDataList[i].dspModeList[j] = FDV;
                else
                    m_receiverDataList[i].dspModeList[j] = LSB;

                //SETTINGS_DEBUG << cstr << ": " << getDSPModeString(m_receiverDataList[i].dspModeList[j]);
            }
        }

        if (m_receiverDataList[i].mercuryAttenuators.length() == MAX_BANDS && m_bandList.length() == MAX_BANDS) {

            for (int j = 0; j < MAX_BANDS; j++) {

                cstr = m_rxStringList.at(i);
                cstr.append("/attenuator");
                cstr.append(m_bandList.at(j).bandString);

                // New format: numeric step attenuator value (0..3 => 0/10/20/30 dB).
                // Backward-compatible with legacy string values: "on"/"off".
                const QVariant attnValue = settings->value(cstr, 0);
                bool ok = false;
                int attn = attnValue.toInt(&ok);
                if (!ok) {
                    const QString legacy = attnValue.toString().trimmed().toLower();
                    if (legacy == "on")
                        attn = 1;   // attenuator ON = 10 dB (was erroneously 0)
                    else if (legacy == "off")
                        attn = 0;   // attenuator OFF = 0 dB (was erroneously 1)
                    else
                        attn = 0;
                }
                m_receiverDataList[i].mercuryAttenuators[j] = qBound(0, attn, 3);
            }
        }

        int valueMin, valueMax;
        if (m_receiverDataList[i].dBmPanScaleMinList.length() == MAX_BANDS &&
            m_receiverDataList[i].dBmPanScaleMaxList.length() == MAX_BANDS &&
            m_bandList.length() == MAX_BANDS
                ) {
            for (int j = 0; j < MAX_BANDS; j++) {

                cstr = m_rxStringList.at(i);
                cstr.append("/dBmPanScaleMin");
                cstr.append(m_bandList.at(j).bandString);

                valueMin = settings->value(cstr, -120).toInt();
                if ((valueMin < -200) || (valueMin > 0)) valueMin = -120;

                cstr = m_rxStringList.at(i);
                cstr.append("/dBmPanScaleMax");
                cstr.append(m_bandList.at(j).bandString);

                valueMax = settings->value(cstr, -10).toInt();
                if ((valueMax < -200) || (valueMax > 0)) valueMax = -10;

                if (valueMax <= valueMin) {

                    valueMin = -120;
                    valueMax = -10;
                }

                m_receiverDataList[i].dBmPanScaleMinList[j] = (qreal) (1.0 * valueMin);
                m_receiverDataList[i].dBmPanScaleMaxList[j] = (qreal) (1.0 * valueMax);
            }
        }
    }

     //******************************************************************
    // graphics settings

    value = settings->value("graphics/dBmDistScaleMin", -20).toInt();
    if ((value < -200) || (value > 0)) value = -20;
    m_displayConfig->setdBmDistScaleMin((qreal) (1.0 * value));
    m_dBmDistScaleMin = m_displayConfig->dBmDistScaleMin();

    value = settings->value("graphics/dBmDistScaleMax", 100).toInt();
    if ((value < -100) || (value > 200)) value = 100;
    m_displayConfig->setdBmDistScaleMax((qreal) (1.0 * value));
    m_dBmDistScaleMax = m_displayConfig->dBmDistScaleMax();

    value = settings->value("graphics/sMeterHoldTime", 2000).toInt();
    if ((value < 0) || (value > 10000)) value = 2000;
    m_displayConfig->setSMeterHoldTime(value);
    m_sMeterHoldTime = m_displayConfig->sMeterHoldTime();


    //******************************************************************
    // color settings
    QColor color;
    TPanadapterColors colors = m_displayConfig->panadapterColors();

    color = settings->value("colors/panBackground", QColor(102, 69, 8)).value<QColor>();
    if (!color.isValid()) color = QColor(102, 69, 8);
    colors.panBackgroundColor = color;

    color = settings->value("colors/waterfall", QColor(246, 146, 6)).value<QColor>();
    if (!color.isValid()) color = QColor(246, 146, 6);
    colors.waterfallColor = color;

    color = settings->value("colors/panLine", QColor(246, 164, 76)).value<QColor>();
    if (!color.isValid()) color = QColor(246, 164, 76);
    colors.panLineColor = color;

    color = settings->value("colors/panLineFilled", QColor(246, 159, 7)).value<QColor>();
    if (!color.isValid()) color = QColor(246, 159, 7);
    colors.panLineFilledColor = color;

    color = settings->value("colors/panSolidTop", QColor(230, 246, 204)).value<QColor>();
    if (!color.isValid()) color = QColor(230, 246, 204);
    colors.panSolidTopColor = color;

    color = settings->value("colors/panSolidBottom", QColor(102, 96, 8)).value<QColor>();
    if (!color.isValid()) color = QColor(102, 96, 8);
    colors.panSolidBottomColor = color;

    color = settings->value("colors/panWideBandLine", QColor(73, 111, 7)).value<QColor>();
    if (!color.isValid()) color = QColor(73, 111, 7);
    colors.wideBandLineColor = color;

    color = settings->value("colors/panWideBandFilled", QColor(137, 172, 62)).value<QColor>();
    if (!color.isValid()) color = QColor(137, 172, 62);
    colors.wideBandFilledColor = color;

    color = settings->value("colors/panWideBandSolidTop", QColor(236, 38, 16)).value<QColor>();
    if (!color.isValid()) color = QColor(236, 38, 16);
    colors.wideBandSolidTopColor = color;

    color = settings->value("colors/panWideBandSolidBottom", QColor(232, 134, 29)).value<QColor>();
    if (!color.isValid()) color = QColor(232, 134, 29);
    colors.wideBandSolidBottomColor = color;

    color = settings->value("colors/distanceLine", QColor(246, 27, 45)).value<QColor>();
    if (!color.isValid()) color = QColor(246, 27, 45);
    colors.distanceLineColor = color;

    color = settings->value("colors/distanceLineFilled", QColor(232, 29, 86)).value<QColor>();
    if (!color.isValid()) color = QColor(232, 29, 86);
    colors.distanceLineFilledColor = color;

    color = settings->value("colors/panCenterLine", QColor(246, 7, 19)).value<QColor>();
    if (!color.isValid()) color = QColor(246, 7, 19);
    colors.panCenterLineColor = color;

    color = settings->value("colors/gridLine", QColor(7, 96, 96)).value<QColor>();
    if (!color.isValid()) color = QColor(7, 96, 96);
    colors.gridLineColor = color;

    m_displayConfig->setPanadapterColors(colors);
    m_panadapterColors = m_displayConfig->panadapterColors();


    SETTINGS_DEBUG << "reading done.";

    return 0;
}

int Settings::saveSettings() {

    QString str;
    //QList<QString> bandList = HamBandStrings();

    settings->setValue(getTitleStr(), getVersionStr());
    settings->setValue("saved",
                       QDateTime::currentDateTime().toString("dddd dd MMMM yyyy hh:mm:ss"));// << " local time\n\n");
    settings->setValue("user/callSign", m_callsignString);

    // window settings
    settings->setValue("window/minimumWidgetWidth", m_minimumWidgetWidth);
    settings->setValue("window/minimumGroupBoxWidth", m_minimumGroupBoxWidth);
    settings->setValue("window/multiRxView", m_multiRxView);

    // network settings
    m_networkConfig->setServerAddress(m_serverAddress);
    m_networkConfig->setLocalAddress(m_hpsdrDeviceLocalAddr);
    m_networkConfig->setServerPort(m_serverPort);
    m_networkConfig->setListenPort(m_listenerPort);
    m_networkConfig->setAudioPort(m_audioPort);
    m_networkConfig->setMetisPort(m_metisPort);
    m_networkConfig->setSocketBufferSize(m_socketBufferSize);

    settings->setValue("network/server_ipAddress", m_networkConfig->serverAddress());
    settings->setValue("network/hpsdr_local_ipAddress", m_networkConfig->localAddress());
    settings->setValue("network/server_port", m_networkConfig->serverPort());
    settings->setValue("network/listen_port", m_networkConfig->listenPort());
    settings->setValue("network/audio_port", m_networkConfig->audioPort());
    settings->setValue("network/metis_port", m_networkConfig->metisPort());
    settings->setValue("network/socketBufferSize", m_networkConfig->socketBufferSize());
    settings->setValue("hpsdr/receivers", m_mercuryReceivers);


    // HPSDR hardware
    m_hardwareConfig->setHpsdrHardware(m_hpsdrHardware);
    m_hardwareConfig->setDevices(m_devices);
    m_hardwareConfig->setCheckFirmwareVersions(m_checkFirmwareVersions);

    settings->setValue("hpsdr/hardware", m_hardwareConfig->hpsdrHardware());

    THPSDRDevices devices = m_hardwareConfig->devices();

    switch (m_hardwareConfig->hpsdrHardware()) {

        // Mercury/Penelope, PennyLane
        case 0:

            if (devices.mercuryPresence)
                settings->setValue("hpsdr/mercury", "true");
            else
                settings->setValue("hpsdr/mercury", "false");

            if (devices.penelopePresence)
                settings->setValue("hpsdr/penelope", "true");
            else
                settings->setValue("hpsdr/penelope", "false");

            if (devices.pennylanePresence)
                settings->setValue("hpsdr/pennylane", "true");
            else
                settings->setValue("hpsdr/pennylane", "false");

            if (devices.excaliburPresence)
                settings->setValue("hpsdr/excalibur", "true");
            else
                settings->setValue("hpsdr/excalibur", "false");
            break;

            // Hermes
        case 1:

            if (devices.mercuryPresence)
                settings->setValue("hpsdr/mercury", "true");
            else
                settings->setValue("hpsdr/mercury", "false");

            if (devices.penelopePresence)
                settings->setValue("hpsdr/penelope", "true");
            else
                settings->setValue("hpsdr/penelope", "false");

            if (devices.pennylanePresence)
                settings->setValue("hpsdr/pennylane", "true");
            else
                settings->setValue("hpsdr/pennylane", "false");

            if (devices.excaliburPresence)
                settings->setValue("hpsdr/excalibur", "true");
            else
                settings->setValue("hpsdr/excalibur", "false");
            break;

            // Cyclops
        case 2:
            break;
    }

    if (devices.alexPresence)
        settings->setValue("hpsdr/alex", "true");
    else
        settings->setValue("hpsdr/alex", "false");


//	if (devices.hermesPresence)
//		settings->setValue("hpsdr/hermes", "true");
//	else
//		settings->setValue("hpsdr/hermes", "false");

    switch (m_hardwareConfig->hpsdrHardware()) {

        // Mercury/Penelope
        case 0:

            if (m_hwInterface == QSDR::Metis)
                settings->setValue("hpsdr/interface", "metis");
            else if (m_hwInterface == QSDR::NoInterfaceMode)
                settings->setValue("hpsdr/interface", "noInterface");
            break;

            // Hermes
        case 1:

            if (m_hwInterface == QSDR::Hermes)
                settings->setValue("hpsdr/interface", "hermes");
            break;

            // Cyclops
        case 2:
            break;
    }

    if (m_hardwareConfig->checkFirmwareVersions())
        settings->setValue("hpsdr/checkfw", "true");
    else
        settings->setValue("hpsdr/checkfw", "false");


    // server settings
    settings->setValue("server/sample_rate", getSampleRate());

    if (m_mercuryDither == 1)
        settings->setValue("server/dither", "on");
    else
        settings->setValue("server/dither", "off");

    if (m_mercuryRandom == 1)
        settings->setValue("server/random", "on");
    else
        settings->setValue("server/random", "off");

    m_hardwareConfig->setSource10Mhz(m_10MHzSource);
    m_hardwareConfig->setSource122_88Mhz(m_122_8MHzSource);

    if (m_hardwareConfig->source10Mhz() == 0)
        settings->setValue("server/10mhzsource", "atlas");
    else if (m_hardwareConfig->source10Mhz() == 1)
        settings->setValue("server/10mhzsource", "penelope");
    else if (m_hardwareConfig->source10Mhz() == 2)
        settings->setValue("server/10mhzsource", "mercury");
    else if (m_hardwareConfig->source10Mhz() == 3)
        settings->setValue("server/10mhzsource", "none");
    else
        settings->setValue("server/10mhzsource", "mercury");

    if (m_hardwareConfig->source122_88Mhz() == 0)
        settings->setValue("server/122_88mhzsource", "penelope");
    else if (m_hardwareConfig->source122_88Mhz() == 1)
        settings->setValue("server/122_88mhzsource", "mercury");

    // audio settings
    m_audioConfig->setMicSource(m_micSource);
    m_audioConfig->setMicInputDev(m_micInputDev);
    m_audioConfig->setMicInputSourceName(m_micInputSourceName);
    m_audioConfig->setDigitalAudioInputDev(m_digitalAudioInputDev);
    m_audioConfig->setDigitalInputSourceName(m_digitalInputSourceName);
    m_audioConfig->setMicGain(m_micGain);
    m_audioConfig->setDriveLevel(m_drivelevel);
    m_audioConfig->setFmPreemphasis(m_fmPremphasize);
    m_audioConfig->setAmCarrierLevel(m_amCarrierLevel);
    m_audioConfig->setAudioCompression(m_audioCompression);
    m_audioConfig->setFmDeviation(m_fmDeveation);
    m_audioConfig->setMainVolume(m_mainVolume);

    if (m_audioConfig->micSource() == 0)
        settings->setValue("server/mic_source", "janus");
    else if (m_audioConfig->micSource() == 1)
        settings->setValue("server/mic_source", "penelope");

    settings->setValue("mic_InputDevice", m_audioConfig->micInputDev());
    settings->setValue("mic_input_source", m_audioConfig->micInputSourceName());
    settings->setValue("digital_audio_InputDevice", m_audioConfig->digitalAudioInputDev());
    settings->setValue("digital_input_source", m_audioConfig->digitalInputSourceName());
    settings->setValue("micGain", m_audioConfig->micGain());
    settings->setValue("driveLevel", m_audioConfig->driveLevel());
    settings->setValue("fm_preemphesize", m_audioConfig->fmPreemphasis());
    settings->setValue("am_carrierlevel", m_audioConfig->amCarrierLevel());
    settings->setValue("audiocompression", m_audioConfig->audioCompression());
    settings->setValue("fmdeveation", m_audioConfig->fmDeviation());
    settings->setValue("server/mainVolume", (int) (m_audioConfig->mainVolume() * 100));


    settings->setValue("server/class", m_RxClass);
    settings->setValue("server/timing", m_RxTiming);

    settings->setValue("repeater_offset",m_repeaterOffset);
    settings->setValue("fm_preemphesize",m_fmPremphasize);
    settings->setValue("am_carrierlevel",m_amCarrierLevel);
    settings->setValue("audiocompression",m_audioCompression);
    settings->setValue("fmdeveation",m_fmDeveation);
    // CW settings
    m_cwConfig->setInternalCw(m_internal_cw);
    m_cwConfig->setKeyReversed(m_cw_key_reversed);
    m_cwConfig->setKeyerSpacing(m_cw_keyer_spacing);
    m_cwConfig->setKeyerSpeed(m_cw_keyer_speed);
    m_cwConfig->setKeyerMode(m_cw_keyer_mode);
    m_cwConfig->setSidetoneVolume(m_cw_sidetone_volume);
    m_cwConfig->setSidetoneFreq(m_cw_sidetone_freq);
    m_cwConfig->setHangTime(m_cw_hang_time);
    m_cwConfig->setPttDelay(m_cw_ptt_delay);
    m_cwConfig->setKeyerWeight(m_cw_keyer_weight);

    if (m_cwConfig->internalCw())
        settings->setValue("cw/internal", "on");
    else settings->setValue("cw/internal", "off");
    if (m_cwConfig->keyReversed())
        settings->setValue("cw/key_reversed","on");
    else settings->setValue("cw/key_reversed","off");
    if (m_cwConfig->keyerSpacing())
        settings->setValue("cw/key_spacing","on");
    else settings->setValue("cw/key_spacing","off");

    settings->setValue("cw/keyer_speed", m_cwConfig->keyerSpeed());
    settings->setValue("cw/keyer_mode", m_cwConfig->keyerMode());
    settings->setValue("cw/sidetone_volume", m_cwConfig->sidetoneVolume());
    settings->setValue("cw/sidetone_freq", m_cwConfig->sidetoneFreq());
    settings->setValue("cw/hang_time", m_cwConfig->hangTime());
    settings->setValue("cw/ptt_delay", m_cwConfig->pttDelay());
    settings->setValue("cw/keyer_weight", m_cwConfig->keyerWeight());




    //settings->setValue("server/mainVolume", (int)(m_mainVolume * 100));

    //if (m_serverMode == QSDR::SDRMode)
    settings->setValue("server/mode", "sdr");

    //settings->setValue("server/mouseWheelFreqStep", m_mouseWheelFreqStep);

    //******************************************************************
    // Alexiares data settings

    // m_alexConfig (qint16)
    //
    // 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
    //   | | | | | | | | | | | | | | |
    //   | | | | | | | | | | | | | | +-----Alex   - manual HPF/LPF filter select (0 = disable, 1 = enable)
    //   | | | | | | | | | | | | | +------ Alex   -	Bypass all HPFs   (0 = disable, 1 = enable)*
    //   | | | | | | | | | | | | +-------- Alex   -	6M low noise amplifier (0 = disable, 1 = enable)*
    //   | | | | | | | | | | | +---------- Alex   -	select 1.5MHz HPF (0 = disable, 1 = enable)*
    //   | | | | | | | | | | +------------ Alex   -	select 6.5MHz HPF (0 = disable, 1 = enable)*
    //   | | | | | | | | | +-------------- Alex   -	select 9.5MHz HPF (0 = disable, 1 = enable)*
    //   | | | | | | | | +---------------- Alex   -	select 13MHz  HPF (0 = disable, 1 = enable)*
    //   | | | | | | | +------------------ Alex   -	select 20MHz  HPF (0 = disable, 1 = enable)*
    //   | | | | | | +-------------------- Alex   - select 160m   LPF (0 = disable, 1 = enable)*
    //   | | | | | +---------------------- Alex   - select 80m    LPF (0 = disable, 1 = enable)*
    //   | | | | +------------------------ Alex   - select 60/40m LPF (0 = disable, 1 = enable)*
    //   | | | +-------------------------- Alex   - select 30/20m LPF (0 = disable, 1 = enable)*
    //   | | +---------------------------- Alex   - select 17/15m LPF (0 = disable, 1 = enable)*
    //   | +------------------------------ Alex   - select 12/10m LPF (0 = disable, 1 = enable)*
    //   +-------------------------------- Alex   - select 6m     LPF (0 = disable, 1 = enable)*

    if (m_alexConfig & 0x01)
        settings->setValue("alex/manual", "on");
    else
        settings->setValue("alex/manual", "off");

    if (m_alexConfig & 0x02)
        settings->setValue("alex/bypassAll", "on");
    else
        settings->setValue("alex/bypassAll", "off");

    if (m_alexConfig & 0x04)
        settings->setValue("alex/amp6m", "on");
    else
        settings->setValue("alex/amp6m", "off");

    settings->setValue("alex/amp6mLo", (int) m_HPFLoFrequencyList.at(5));
    settings->setValue("alex/amp6mHi", (int) m_HPFHiFrequencyList.at(5));

    if (m_alexConfig & 0x08)
        settings->setValue("alex/hpf1_5MHz", "on");
    else
        settings->setValue("alex/hpf1_5MHz", "off");

    settings->setValue("alex/hpf1_5MHzLo", (int) m_HPFLoFrequencyList.at(0));
    settings->setValue("alex/hpf1_5MHzHi", (int) m_HPFHiFrequencyList.at(0));

    if (m_alexConfig & 0x10)
        settings->setValue("alex/hpf6_5MHz", "on");
    else
        settings->setValue("alex/hpf6_5MHz", "off");

    settings->setValue("alex/hpf6_5MHzLo", (int) m_HPFLoFrequencyList.at(1));
    settings->setValue("alex/hpf6_5MHzHi", (int) m_HPFHiFrequencyList.at(1));

    if (m_alexConfig & 0x20)
        settings->setValue("alex/hpf9_5MHz", "on");
    else
        settings->setValue("alex/hpf9_5MHz", "off");

    settings->setValue("alex/hpf9_5MHzLo", (int) m_HPFLoFrequencyList.at(2));
    settings->setValue("alex/hpf9_5MHzHi", (int) m_HPFHiFrequencyList.at(2));

    if (m_alexConfig & 0x40)
        settings->setValue("alex/hpf13MHz", "on");
    else
        settings->setValue("alex/hpf13MHz", "off");

    settings->setValue("alex/hpf13MHzLo", (int) m_HPFLoFrequencyList.at(3));
    settings->setValue("alex/hpf13MHzHi", (int) m_HPFHiFrequencyList.at(3));

    if (m_alexConfig & 0x80)
        settings->setValue("alex/hpf20MHz", "on");
    else
        settings->setValue("alex/hpf20MHz", "off");

    settings->setValue("alex/hpf20MHzLo", (int) m_HPFLoFrequencyList.at(4));
    settings->setValue("alex/hpf20MHzHi", (int) m_HPFHiFrequencyList.at(4));

    if (m_alexConfig & 0x100)
        settings->setValue("alex/lpf160m", "on");
    else
        settings->setValue("alex/lpf160m", "off");

    settings->setValue("alex/lpf160mLo", (int) m_LPFLoFrequencyList.at(0));
    settings->setValue("alex/lpf160mHi", (int) m_LPFHiFrequencyList.at(0));

    if (m_alexConfig & 0x200)
        settings->setValue("alex/lpf80m", "on");
    else
        settings->setValue("alex/lpf80m", "off");

    settings->setValue("alex/lpf80mLo", (int) m_LPFLoFrequencyList.at(1));
    settings->setValue("alex/lpf80mHi", (int) m_LPFHiFrequencyList.at(1));

    if (m_alexConfig & 0x400)
        settings->setValue("alex/lpf60_40m", "on");
    else
        settings->setValue("alex/lpf60_40m", "off");

    settings->setValue("alex/lpf60_40mLo", (int) m_LPFLoFrequencyList.at(2));
    settings->setValue("alex/lpf60_40mHi", (int) m_LPFHiFrequencyList.at(2));

    if (m_alexConfig & 0x800)
        settings->setValue("alex/lpf30_20m", "on");
    else
        settings->setValue("alex/lpf30_20m", "off");

    settings->setValue("alex/lpf30_20mLo", (int) m_LPFLoFrequencyList.at(3));
    settings->setValue("alex/lpf30_20mHi", (int) m_LPFHiFrequencyList.at(3));

    if (m_alexConfig & 0x1000)
        settings->setValue("alex/lpf17_15m", "on");
    else
        settings->setValue("alex/lpf17_15m", "off");

    settings->setValue("alex/lpf17_15mLo", (int) m_LPFLoFrequencyList.at(4));
    settings->setValue("alex/lpf17_15mHi", (int) m_LPFHiFrequencyList.at(4));

    if (m_alexConfig & 0x2000)
        settings->setValue("alex/lpf12_10m", "on");
    else
        settings->setValue("alex/lpf12_10m", "off");

    settings->setValue("alex/lpf12_10mLo", (int) m_LPFLoFrequencyList.at(5));
    settings->setValue("alex/lpf12_10mHi", (int) m_LPFHiFrequencyList.at(5));

    if (m_alexConfig & 0x4000)
        settings->setValue("alex/lpf6m", "on");
    else
        settings->setValue("alex/lpf6m", "off");

    settings->setValue("alex/lpf6mLo", (int) m_LPFLoFrequencyList.at(6));
    settings->setValue("alex/lpf6mHi", (int) m_LPFHiFrequencyList.at(6));


    //***********************************************************************
    for (int i = 0; i < MAX_BANDS; i++) {

        str = "alex/state";
        str.append(m_bandList.at(i).bandString);

        settings->setValue(str, m_alexStates.at(i));
    }

    if (m_alexStates.length() == MAX_BANDS && m_bandList.length() == MAX_BANDS) {

        for (int i = 0; i < MAX_BANDS - 1; i++) {

            str = "alex/state";
            str.append(m_bandList.at(i).bandString);

            settings->setValue(str, m_alexStates.at(i));
        }
    }


    //******************************************************************
    // Penny open collector settings

    if (m_pennyOCEnabled)
        settings->setValue("penny/OCenabled", "on");
    else
        settings->setValue("penny/OCenabled", "off");

    if (m_rxJ6pinList.length() == MAX_BANDS - 1 && m_txJ6pinList.length() == MAX_BANDS - 1 &&
        m_bandList.length() == MAX_BANDS) {

        for (int i = 0; i < MAX_BANDS - 1; i++) {

            str = "penny/rxState";
            str.append(m_bandList.at(i).bandString);

            settings->setValue(str, m_rxJ6pinList.at(i));
        }

        for (int i = 0; i < MAX_BANDS - 1; i++) {

            str = "penny/txState";
            str.append(m_bandList.at(i).bandString);

            settings->setValue(str, m_txJ6pinList.at(i));
        }
    }

    //******************************************************************
    // wideband settings

    if (m_widebandOptions.wideBandData)
        settings->setValue("wideband/widebandData", "on");
    else
        settings->setValue("wideband/widebandData", "off");

    if (!m_widebandOptions.wideBandData) m_widebandOptions.wideBandDisplayStatus = false;

    if (m_widebandOptions.wideBandDisplayStatus)
        settings->setValue("wideband/widebandDisplay", "on");
    else
        settings->setValue("wideband/widebandDisplay", "off");

    if (m_widebandOptions.averaging)
        settings->setValue("wideband/averaging", "on");
    else
        settings->setValue("wideband/averaging", "off");

    settings->setValue("wideband/averagingCnt", m_widebandOptions.averagingCnt);
    settings->setValue("wideband/dBmWideBandScaleMin", (int) m_widebandOptions.dBmWBScaleMin);
    settings->setValue("wideband/dBmWideBandScaleMax", (int) m_widebandOptions.dBmWBScaleMax);

    if (m_widebandOptions.panMode == Line)
        settings->setValue("wideband/panMode", "LINE");
    else if (m_widebandOptions.panMode == FilledLine)
        settings->setValue("wideband/panMode", "FILLEDLINE");
    else if (m_widebandOptions.panMode == Solid)
        settings->setValue("wideband/panMode", "SOLID");



    //******************************************************************
    // receiver data settings

    for (int i = 0; i < MAX_RECEIVERS; i++) {


        QString str = m_rxStringList.at(i);
        str.append("/dspCore");

        if (m_receiverDataList[i].dspCore == QSDR::QtDSP)
            settings->setValue(str, "qtdsp");

        str = m_rxStringList.at(i);
        str.append("/nr_agc");
        settings->setValue(str,  (m_receiverDataList[i].nr_agc));

        str = m_rxStringList.at(i);
        str.append("/nr2_gain_method");
        settings->setValue(str,  (m_receiverDataList[i].nr2_gain_method));

        str = m_rxStringList.at(i);
        str.append("/nr2_npe_method");
        settings->setValue(str, (m_receiverDataList[i].nr2_npe_method));

        str = m_rxStringList.at(i);
        str.append("/nr2_ae");
        settings->setValue(str, (bool) (m_receiverDataList[i].nr2_ae));

        str = m_rxStringList.at(i);
        str.append("/nr");
        settings->setValue(str,  (m_receiverDataList[i].nr));

        str = m_rxStringList.at(i);
        str.append("/nbMode");
        settings->setValue(str,  (m_receiverDataList[i].nbMode));

        str = m_rxStringList.at(i);
        str.append("/anf");
        settings->setValue(str, (bool) (m_receiverDataList[i].anf));

        str = m_rxStringList.at(i);
        str.append("/snb");
        settings->setValue(str, (bool) (m_receiverDataList[i].snb));


        str = m_rxStringList.at(i);
        str.append("/fftSize");
        settings->setValue(str, (m_receiverDataList[i].fftsize));

        str = m_rxStringList.at(i);
        str.append("/PanAverageMode");
        settings->setValue(str, (int) (m_receiverDataList[i].panAvMode));

        str = m_rxStringList.at(i);
        str.append("/PanDetectorMode");
        settings->setValue(str, (int) (m_receiverDataList[i].panDetMode));

        str = m_rxStringList.at(i);
        str.append("/freqRulerPosition");
        settings->setValue(str, (int) (m_receiverDataList[i].freqRulerPosition * 10));

        str = m_rxStringList.at(i);
        str.append("/audioVolume");
        settings->setValue(str, (int) (m_receiverDataList[i].audioVolume * 100));

        str = m_rxStringList.at(i);
        str.append("/mouseWheelFreqStep");
        settings->setValue(str, (int) (m_receiverDataList[i].mouseWheelFreqStep));

        str = m_rxStringList.at(i);
        str.append("/agcGain");
        settings->setValue(str, m_receiverDataList[i].acgGain);

        str = m_rxStringList.at(i);
        str.append("/agcFixedGain");
        settings->setValue(str, (int) m_receiverDataList[i].agcFixedGain_dB);

        str = m_rxStringList.at(i);
        str.append("/agcMaximumGain");
        settings->setValue(str,  m_receiverDataList[i].agcMaximumGain_dB);

        str = m_rxStringList.at(i);
        str.append("/agcSlope");
        settings->setValue(str, m_receiverDataList[i].agcSlope);

        str = m_rxStringList.at(i);
        str.append("/agcAttacktime");
        settings->setValue(str, (int) (m_receiverDataList[i].agcAttackTime * 1000));

        str = m_rxStringList.at(i);
        str.append("/agcDecaytime");
        settings->setValue(str, (int) (m_receiverDataList[i].agcDecayTime * 1000));

        str = m_rxStringList.at(i);
        str.append("/agcHangTime");
        settings->setValue(str, (int) (m_receiverDataList[i].agcHangTime * 1000));

        str = m_rxStringList.at(i);
        str.append("/agcLines");

        if (m_receiverDataList[i].agcLines)
            settings->setValue(str, "on");
        else
            settings->setValue(str, "off");

//		str = m_rxStringList.at(i);
//		str.append("/attenuator");
//		str.append(m_bandList.at(j).bandString);
//
//		if (m_receiverDataList.at(i).mercuryAttenuators.at(j))
//			settings->setValue(str, "off");
//		else
//			settings->setValue(str, "on");

        str = m_rxStringList.at(i);
        str.append("/agcMode");

        if (m_receiverDataList[i].agcMode == agcOFF)
            settings->setValue(str, "OFF");
        else if (m_receiverDataList[i].agcMode == agcLONG)
            settings->setValue(str, "LONG");
        else if (m_receiverDataList[i].agcMode == agcSLOW)
            settings->setValue(str, "SLOW");
        else if (m_receiverDataList[i].agcMode == agcMED)
            settings->setValue(str, "MED");
        else if (m_receiverDataList[i].agcMode == agcFAST)
            settings->setValue(str, "FAST");

        str = m_rxStringList.at(i);
        str.append("/adcMode");

        if (m_receiverDataList[i].adcMode == adc1)
            settings->setValue(str, "ADC1");
        else if (m_receiverDataList[i].adcMode == adc2)
            settings->setValue(str, "ADC2");

        str = m_rxStringList.at(i);
        str.append("/panMode");

        if (m_receiverDataList[i].panMode == Line)
            settings->setValue(str, "LINE");
        else if (m_receiverDataList[i].panMode == FilledLine)
            settings->setValue(str, "FILLEDLINE");
        else if (m_receiverDataList[i].panMode == Solid)
            settings->setValue(str, "SOLID");

        str = m_rxStringList.at(i);
        str.append("/waterfallMode");

        if (m_receiverDataList[i].waterfallMode == Simple)
            settings->setValue(str, "SIMPLE");
        else if (m_receiverDataList[i].waterfallMode == Enhanced)
            settings->setValue(str, "ENHANCED");

        str = m_rxStringList.at(i);
        str.append("/framesPerSecond");
        settings->setValue(str, m_receiverDataList[i].framesPerSecond);

        str = m_rxStringList.at(i);
        str.append("/waterfallOffsetLo");
        settings->setValue(str, m_receiverDataList[i].waterfallOffsetLo);

        str = m_rxStringList.at(i);
        str.append("/waterfallOffsetHi");
        settings->setValue(str, m_receiverDataList[i].waterfallOffsetHi);

        str = m_rxStringList.at(i);
        str.append("/filterHi");
        settings->setValue(str, m_receiverDataList[i].filterHi);

        str = m_rxStringList.at(i);
        str.append("/filterLo");
        settings->setValue(str, m_receiverDataList[i].filterLo);
        str = m_rxStringList.at(i);

        str.append("/filterIndex");
        settings->setValue(str, m_receiverDataList[i].m_filterIndex);


        str = m_rxStringList.at(i);
        str.append("/averaging");
        if (m_receiverDataList[i].spectrumAveraging)
            settings->setValue(str, "on");
        else
            settings->setValue(str, "off");

        str = m_rxStringList.at(i);
        str.append("/averagingCnt");
        settings->setValue(str, m_receiverDataList[i].averagingCnt);

        str = m_rxStringList.at(i);
        str.append("/grid");
        if (m_receiverDataList[i].panGrid)
            settings->setValue(str, "on");
        else
            settings->setValue(str, "off");

        str = m_rxStringList.at(i);
        str.append("/hairCross");
        if (m_receiverDataList[i].hairCross)
            settings->setValue(str, "on");
        else
            settings->setValue(str, "off");

        str = m_rxStringList.at(i);
        str.append("/panLocked");
        if (m_receiverDataList[i].panLocked)
            settings->setValue(str, "on");
        else
            settings->setValue(str, "off");

        str = m_rxStringList.at(i);
        str.append("/clickVFO");
        if (m_receiverDataList[i].clickVFO)
            settings->setValue(str, "on");
        else
            settings->setValue(str, "off");

        // center frequencies
        for (int j = 0; j < MAX_BANDS; j++) {

            str = m_rxStringList.at(i);
            str.append("/lastCenterFrequency");
            str.append(m_bandList.at(j).bandString);

            settings->setValue(str, (int) m_receiverDataList[i].lastCenterFrequencyList.at(j));
        }

        // vfo frequencies
        for (int j = 0; j < MAX_BANDS; j++) {

            str = m_rxStringList.at(i);
            str.append("/lastVfoFrequency");
            str.append(m_bandList.at(j).bandString);

            settings->setValue(str, (int) m_receiverDataList[i].lastVfoFrequencyList.at(j));
        }

        m_receiverConfigs[i]->setCtrFrequency(m_receiverDataList[i].ctrFrequency);
        m_receiverConfigs[i]->setVfoFrequency(m_receiverDataList[i].vfoFrequency);

        str = m_rxStringList.at(i);
        str.append("/centerFrequency");
        settings->setValue(str, (int) m_receiverConfigs[i]->ctrFrequency());

        str = m_rxStringList.at(i);
        str.append("/vfoFrequency");
        settings->setValue(str, (int) m_receiverConfigs[i]->vfoFrequency());

        str = m_rxStringList.at(i);
        str.append("/digitalVoiceEngine");
        settings->setValue(str, DV_ENGINE_FREEDV);

        for (int j = 0; j < MAX_BANDS; j++) {

            str = m_rxStringList.at(i);
            str.append("/dspMode");
            str.append(m_bandList.at(j).bandString);

            DSPMode mode = m_receiverDataList.at(i).dspModeList.at(j);
            if (mode == LSB)
                settings->setValue(str, "LSB");
            else if (mode == USB)
                settings->setValue(str, "USB");
            else if (mode == DSB)
                settings->setValue(str, "DSB");
            else if (mode == CWL)
                settings->setValue(str, "CWL");
            else if (mode == CWU)
                settings->setValue(str, "CWU");
            else if (mode == FMN)
                settings->setValue(str, "FMN");
            else if (mode == AM)
                settings->setValue(str, "AM");
            else if (mode == DIGU)
                settings->setValue(str, "DIGU");
            else if (mode == SPEC)
                settings->setValue(str, "SPEC");
            else if (mode == DIGL)
                settings->setValue(str, "DIGL");
            else if (mode == SAM)
                settings->setValue(str, "SAM");
            else if (mode == FDV)
                settings->setValue(str, "FDV");
        }

        for (int j = 0; j < MAX_BANDS; j++) {

            str = m_rxStringList.at(i);
            str.append("/attenuator");
            str.append(m_bandList.at(j).bandString);

            // Persist step attenuator directly (0..3 => 0/10/20/30 dB).
            settings->setValue(str, qBound(0, m_receiverDataList.at(i).mercuryAttenuators.at(j), 3));
        }

        for (int j = 0; j < MAX_BANDS; j++) {

            str = m_rxStringList.at(i);
            str.append("/dBmPanScaleMin");
            str.append(m_bandList.at(j).bandString);

            settings->setValue(str, (int) m_receiverDataList[i].dBmPanScaleMinList[j]);

            str = m_rxStringList.at(i);
            str.append("/dBmPanScaleMax");
            str.append(m_bandList.at(j).bandString);

            settings->setValue(str, (int) m_receiverDataList[i].dBmPanScaleMaxList[j]);
        }
    }


    //******************************************************************
    // Graphics settings

    /*if (m_specAveraging)
        settings->setValue("graphics/averaging", "on");
    else
        settings->setValue("graphics/averaging", "off");*/

    /*if (m_panGrid)
        settings->setValue("graphics/grid", "on");
    else
        settings->setValue("graphics/grid", "off");*/

    m_displayConfig->setdBmDistScaleMin(m_dBmDistScaleMin);
    m_displayConfig->setdBmDistScaleMax(m_dBmDistScaleMax);

    settings->setValue("graphics/dBmDistScaleMin", (int) m_displayConfig->dBmDistScaleMin());
    settings->setValue("graphics/dBmDistScaleMax", (int) m_displayConfig->dBmDistScaleMax());

    /*if (m_waterfallColorScheme == QSDRGraphics::simple)
        settings->setValue("graphics/waterfall", "simple");
    else
    if (m_waterfallColorScheme == QSDRGraphics::enhanced)
        settings->setValue("graphics/waterfall", "enhanced");
    else
    if (m_waterfallColorScheme == QSDRGraphics::spectran)
        settings->setValue("graphics/waterfall", "spectran");*/

    m_displayConfig->setSMeterHoldTime(m_sMeterHoldTime);
    m_displayConfig->setPanadapterColors(m_panadapterColors);

    settings->setValue("graphics/sMeterHoldTime", m_displayConfig->sMeterHoldTime());


    // Colors
    TPanadapterColors colors = m_displayConfig->panadapterColors();
    settings->setValue("colors/panBackground", QVariant(colors.panBackgroundColor).toString());
    settings->setValue("colors/waterfall", QVariant(colors.waterfallColor).toString());
    settings->setValue("colors/panLine", QVariant(colors.panLineColor).toString());
    settings->setValue("colors/panLineFilled", QVariant(colors.panLineFilledColor).toString());
    settings->setValue("colors/panSolidTop", QVariant(colors.panSolidTopColor).toString());
    settings->setValue("colors/panSolidBottom", QVariant(colors.panSolidBottomColor).toString());
    settings->setValue("colors/panWideBandLine", QVariant(colors.wideBandLineColor).toString());
    settings->setValue("colors/panWideBandFilled", QVariant(colors.wideBandFilledColor).toString());
    settings->setValue("colors/panWideBandSolidTop", QVariant(colors.wideBandSolidTopColor).toString());
    settings->setValue("colors/panWideBandSolidBottom",
                       QVariant(colors.wideBandSolidBottomColor).toString());
    settings->setValue("colors/distanceLine", QVariant(colors.distanceLineColor).toString());
    settings->setValue("colors/distanceLineFilled", QVariant(colors.distanceLineFilledColor).toString());
    settings->setValue("colors/panCenterLine", QVariant(colors.panCenterLineColor).toString());
    settings->setValue("colors/gridLine", QVariant(colors.gridLineColor).toString());

    SETTINGS_DEBUG << "save settings done.";
    return 0;
}

//void Settings::setMainWindowsState() {
//
//	settings->setValue("geometry", .saveGeometry());
//	settings->setValue("windowState", saveState());
//}

//*******************************************************

QList<long> Settings::getCtrFrequencies() {

    QList<long> frequencies;

    for (int i = 0; i < MAX_RECEIVERS; i++)
        frequencies << m_receiverDataList[i].ctrFrequency;

    return frequencies;
}

QList<long> Settings::getVfoFrequencies() {

    QList<long> frequencies;

    for (int i = 0; i < MAX_RECEIVERS; i++)
        frequencies << m_receiverDataList[i].vfoFrequency;

    return frequencies;
}

QString Settings::getDSPModeString(int mode) {

    switch (mode) {

        case 0:
            return QString("LSB");

        case 1:
            return QString("USB");

        case 2:
            return QString("DSB");

        case 3:
            return QString("CWL");

        case 4:
            return QString("CWU");

        case 5:
            return QString("FMN");

        case 6:
            return QString("AM");

        case 7:
            return QString("DIGU");

        case 8:
            return QString("SPEC");

        case 9:
            return QString("DIGL");

        case 10:
            return QString("SAM");

        case 11:
            return QString("FreeDV");

        default:
            return QString("unknown mode");
    }
}

//*******************************************************

QString Settings::getTitleStr() {

    return m_titleString;
}

QString Settings::getVersionStr() {

    return m_versionString;
}

QString Settings::getSettingsFilename() {

    return settingsFilename;
}

QString Settings::getCallsign() {

    return m_callsignString;
}

QString Settings::getValue1000(

        double value,            /*!<[in] Value to print. */
        int valuePrefix,        /*!<[in] Value current prefix. */
        QString unitBase)        /*!<[in] Unit base string. */
{
    const int prefixBase = 1000;
    int resPrefix = valuePrefix;

    static const char *prefixTab[prefixSiMax + 1] = {
            "",    /* prefixNothing */
            "k",    /* prefixKilo */
            "M",    /* prefixMega */
            "G",    /* prefixGiga */
            "T",    /* prefixTera */
            "P",    /* prefixPeta */
            "E",    /* prefixExa */
            "Z",    /* prefixZetta */
            "Y",    /* prefixYotta */
    };

    /*while((value > (10 * prefixBase)) && (resPrefix < prefixSiMax)) {
        value /= prefixBase;
        resPrefix++;
    }*/
    while ((value > (prefixBase / 10)) && (resPrefix < prefixSiMax)) {
        value /= prefixBase;
        resPrefix++;
    }

    return QString("%1 %2%3").arg(value).arg(prefixTab[resPrefix]).arg(unitBase);
}

QString Settings::getValue1024(

        double value,            /*!<[in] Value to print. */
        int valuePrefix,        /*!<[in] Value current prefix. */
        QString unitBase)        /*!<[in] Unit base string. */
{
    const int prefixBase = 1024;
    int resPrefix = valuePrefix;

    static const char *prefixTab[prefixIecMax + 1] = {
            "",    /* prefixNothing */
            "Ki",    /* prefixKibi */
            "Mi",    /* prefixMebi */
            "Gi",    /* prefixGibi */
            "Ti",    /* prefixTebi */
            "Pi",    /* prefixPebi */
            "Ei",    /* prefixExbi */
            "Zi",    /* prefixZebi */
            "Yi",    /* prefixYobi */
    };

    while ((value > (10 * prefixBase)) && (resPrefix < prefixIecMax)) {
        value /= prefixBase;
        resPrefix++;
    }

    return QString("%1 %2%3").arg(value).arg(prefixTab[resPrefix]).arg(unitBase);
}

int Settings::getMinimumWidgetWidth() {

    return m_minimumWidgetWidth;
}

int Settings::getMinimumGroupBoxWidth() {

    return m_minimumGroupBoxWidth;
}

void Settings::debugSystemState() {

    qDebug() << " ";
    SETTINGS_DEBUG << "**********************************************************";
    SETTINGS_DEBUG << "Error:\t\t\t" << qPrintable(getErrorString(m_systemError));
    SETTINGS_DEBUG << "HW Interface:\t\t" << qPrintable(getHWInterfaceModeString(m_hwInterface));
    SETTINGS_DEBUG << "Server Mode:\t\t" << qPrintable(getServerModeString(m_serverMode));
    SETTINGS_DEBUG << "DataEngine State:\t" << qPrintable(getHDataEngineStateString(m_dataEngineState));
    qDebug() << " ";
}

void Settings::setMainPower(bool power) {

    if (m_mainPower == power) return;

    m_mainPower = power;
    if (power)
        m_mainPower = true;
    else
        m_mainPower = false;

    emit masterSwitchChanged(m_mainPower);
}

bool Settings::getMainPower() {

    return m_mainPower;
}

void Settings::setSystemMessage(const QString &msg, int time) {

    emit systemMessageEvent(msg, time);
}

void Settings::setSystemState(

        QSDR::_Error err,
        QSDR::_HWInterfaceMode hwmode,
        QSDR::_ServerMode mode,
        QSDR::_DataEngineState state) {
    QMutexLocker locker(&settingsMutex);

    if (m_systemError != err)
        m_systemError = err;

    if (m_hwInterface != hwmode)
        m_hwInterface = hwmode;

    if (m_serverMode != mode) {

        m_serverMode = mode;

    }

    if (m_dataEngineState != state)
        m_dataEngineState = state;

  //if (m_dataEngineState == QSDR::DataEngineDown)
//      setCurrentReceiver(0);
    //m_currentReceiver = 0;

    //locker.unlock();

    debugSystemState();
    emit systemStateChanged(m_systemError, m_hwInterface, m_serverMode, m_dataEngineState);
}

QSDR::_ServerMode Settings::getCurrentServerMode() {

    return m_serverMode;
}

QSDR::_HWInterfaceMode Settings::getHWInterface() {

    return m_hwInterface;
}

QSDR::_DataEngineState Settings::getDataEngineState() {

    return m_dataEngineState;
}

QString Settings::getErrorString(QSDR::_Error err) {

    QString str;
    switch (err) {

        case QSDR::_Error::NoError:
            str = "No error";
            break;

        case QSDR::_Error::NotImplemented:
            str = "Not implemented";
            break;

        case QSDR::_Error::HwIOError:
            str = "Hardware IO error";
            break;

        case QSDR::_Error::ServerModeError:
            str = "Server mode error";
            break;

        case QSDR::_Error::OpenError:
            str = "open device error";
            break;

        case QSDR::_Error::DataReceiverThreadError:
            str = "dataReceiverThread error";
            break;

        case QSDR::_Error::DataProcessThreadError:
            str = "dataProcessThread error";
            break;

        case QSDR::_Error::WideBandDataProcessThreadError:
            str = "widebandDataProcessThread error";
            break;

        case QSDR::_Error::AudioThreadError:
            str = "audioThread error";
            break;


        case QSDR::_Error::UnderrunError:
            str = "underrun error";
            break;

        case QSDR::_Error::FirmwareError:
            str = "firmware error";
            break;

        case QSDR::_Error::FatalError:
            str = "fatal error";
            break;
    }
    return str;
}

QString Settings::getHDataEngineStateString(QSDR::_DataEngineState mode) {

    QString str;
    switch (mode) {

        case QSDR::_DataEngineState::DataEngineDown:
            str = "down";
            break;

        case QSDR::_DataEngineState::DataEngineUp:
            str = "up";
            break;
    }
    return str;
}

QString Settings::getServerModeString(QSDR::_ServerMode mode) {

    QString str;
    switch (mode) {

        case QSDR::_ServerMode::NoServerMode:
            str = "no server mode";
            break;

        case QSDR::_ServerMode::SDRMode :
            str = "SDR mode";
            break;

    }
    return str;
}

QString Settings::getHWInterfaceModeString(QSDR::_HWInterfaceMode mode) {

    QString str;
    switch (mode) {

        case QSDR::_HWInterfaceMode::NoInterfaceMode:
            str = "no interface";
            break;

        case QSDR::_HWInterfaceMode::Metis:
            str = "Metis";
            break;

        case QSDR::_HWInterfaceMode::Hermes:
            str = "Hermes";
            break;

        case QSDR::_HWInterfaceMode::SoapySDR:
            str = "SoapySDR";
            break;
    }
    return str;
}

void Settings::setTxAllowed(bool value) {

    if (m_devices.penelopePresence || m_devices.pennylanePresence || (m_hwInterface == QSDR::Hermes))
        m_transmitter.txAllowed = value;
    else
        m_transmitter.txAllowed = false;

    emit txAllowedChanged(m_transmitter.txAllowed);
}

bool Settings::getTxAllowed() {

    return m_transmitter.txAllowed;
}

void Settings::setGraphicsState(

        int rx,
        PanGraphicsMode panMode,
        WaterfallColorMode waterfallColorMode) {
    QMutexLocker locker(&settingsMutex);

    if (rx == -1) {

        m_widebandOptions.panMode = panMode;
    } else {

        m_receiverDataList[rx].panMode = panMode;
        m_receiverDataList[rx].waterfallMode = waterfallColorMode;
    }

    //locker.unlock();

    //SETTINGS_DEBUG << "graphics mode:" << panMode << waterfallColorMode;
    emit graphicModeChanged(rx, panMode, waterfallColorMode);
}

PanGraphicsMode Settings::getPanadapterMode(int rx) {

    return m_receiverDataList[rx].panMode;
}

PanAveragingMode Settings::getPanAveragingMode(int rx) {

    return m_receiverDataList[rx].panAvMode;
}


PanDetectorMode Settings::getPanDetectorMode(int rx) {

    return m_receiverDataList[rx].panDetMode;
}


WaterfallColorMode Settings::getWaterfallColorMode(int rx) {

    return m_receiverDataList.at(rx).waterfallMode;
}

//QSDRGraphics::_Colors Settings::getColorItem() {
//
//	return m_colorItem;
//}

void Settings::setDefaultSkin(bool value) {

    m_defaultSkin = value;
}

bool Settings::getDefaultSkin() {

    return m_defaultSkin;
}

void Settings::setSettingsFilename(QString filename) {

    filename = filename.trimmed();

    QMutexLocker locker(&settingsMutex);

    settingsFilename = filename;
    //locker.unlock();

    emit settingsFilenameChanged(filename);
}

void Settings::setSettingsLoaded(bool value) {

    QMutexLocker locker(&settingsMutex);

    setLoaded = value;

    locker.unlock();

    emit settingsLoadedChanged(setLoaded);
}

bool Settings::getSettingsLoaded() {

    return setLoaded;
}

void Settings::setCPULoad(short load) {

    emit cpuLoadChanged(load);
}

void Settings::setCallsign(const QString &callsign) {

    QString cs = callsign.trimmed();

    QMutexLocker locker(&settingsMutex);

    if (m_callsignString == cs) return;

    m_callsignString = cs;

    locker.unlock();
    emit callsignChanged();
}

void Settings::setRxList(QList<Receiver *> rxList) {

    emit rxListChanged(rxList);
}

void Settings::setMultiRxView(int view) {

    QMutexLocker locker(&settingsMutex);

    if (m_multiRxView == view) return;
    m_multiRxView = view;

    locker.unlock();
    emit multiRxViewChanged(m_multiRxView);
}

int Settings::getMultiRxView() {

    return m_multiRxView;
}

void Settings::setMetisCardList(QList<TNetworkDevicecard> list) {

    QMutexLocker locker(&settingsMutex);

    m_metisCards = list;

    locker.unlock();
    emit metisCardListChanged(m_metisCards);
}

void Settings::searchHpsdrNetworkDevices() {

    emit searchMetisSignal();
}

#ifdef HAVE_SOAPYSDR
void Settings::searchSoapyDevices() {
    emit searchSoapySignal();
}
#endif

void Settings::searchDevices() {
    emit searchMetisSignal();
#ifdef HAVE_SOAPYSDR
    emit searchSoapySignal();
#endif
}

void Settings::clearMetisCardList() {

    m_metisCards.clear();

    //emit metisCardListChanged(m_metisCards);
}

void Settings::setCurrentHPSDRDevice(TNetworkDevicecard card) {

    m_currentHPSDRDevice = card;

    if (card.frequency_max > 0) {
        m_maxFrequency = static_cast<long>(card.frequency_max);
        m_minFrequency = static_cast<long>(card.frequency_min);
        emit maxFrequencyChanged(m_maxFrequency);
    }

    emit hpsdrNetworkDeviceChanged(m_currentHPSDRDevice);
}

#ifdef HAVE_SOAPYSDR
void Settings::setSoapyDeviceList(QList<TSoapyDevice> list) {
    m_soapyDevices = list;
    emit soapyDeviceListChanged(m_soapyDevices);
}

void Settings::setCurrentSoapyDevice(TSoapyDevice device) {
    m_currentSoapyDevice = device;
    emit soapyDeviceChanged(m_currentSoapyDevice);
}

void Settings::setSoapyMessage(QString message) {
    emit soapyMessageEvent(message);
}
#endif

void Settings::setHPSDRDeviceNumber(int value) {

    m_hpsdrNetworkDevices = value;
    emit networkDeviceNumberChanged(value);
}

void Settings::showNetworkIODialog() {

    emit showNetworkIO();
}

void Settings::showWarningDialog(const QString &msg) {

    emit showWarning(msg);
}

void Settings::addNetworkIOComboBoxEntry(QString str) {

    emit networkIOComboBoxEntryAdded(str);
}

void Settings::clearNetworkIOComboBoxEntry() {

    emit clearNetworkIOComboBoxEntrySignal();
}

void Settings::setPBOPresence(bool value) {

    m_pboFound = value;
}

bool Settings::getPBOPresence() {

    return m_pboFound;
}

void Settings::setFBOPresence(bool value) {

    m_fboFound = value;
}

bool Settings::getFBOPresence() {

    return m_fboFound;
}

//*******************************
// Network settings

void Settings::setNumberOfNetworkInterfaces(int value) {

    m_NetworkInterfacesNo = value;
}

void Settings::addServerNetworkInterface(QString nicName, QString ipAddress) {

    emit newServerNetworkInterface(nicName, ipAddress);
}

void Settings::addHPSDRDeviceNIC(QString nicName, QString ipAddress) {

    emit newHPSDRDeviceNIC(nicName, ipAddress);
}

void Settings::setServerNetworkInterface(int index) {

    setServerAddr(this->m_ipAddressesList.at(index).toString());

    //qDebug() << "m_networkInterfaces.at(index).humanReadableName():" << m_networkInterfaces.at(index).humanReadableName();
    //qDebug() << "m_ipAddressesList.at(index).toString():" << m_ipAddressesList.at(index).toString();

    QString message = "[settings]: network interface set to: %1 (%2).";
    /*emit messageEvent(
        message.arg(
            m_networkInterfaces.at(index).humanReadableName(),
            m_ipAddressesList.at(index).toString() ));*/

}

void Settings::setHPSDRDeviceNIC(int index) {

    setHPSDRDeviceLocalAddr(this->m_ipAddressesList.at(index).toString());

    QString message = "[settings]: HPSDR device network interface set to: %1 (%2).";
    /*emit messageEvent(
        message.arg(
            m_networkInterfaces.at(index).humanReadableName(),
            m_ipAddressesList.at(index).toString() ));*/

}

void Settings::setServerWidgetNIC(int index) {

    /*QString message = "[server]: network interface set to: %1 (%2).";
    emit messageEvent(
        message.arg(
            m_networkInterfaces.at(index).humanReadableName(),
            m_ipAddressesList.at(index).toString() ));*/

    emit serverNICChanged(index);
}

void Settings::setHPSDRWidgetNIC(int index) {

    /*QString message = "[server]: HPSDR device network interface set to: %1 (%2).";
    emit messageEvent(
        message.arg(
            m_networkInterfaces.at(index).humanReadableName(),
            m_ipAddressesList.at(index).toString() ));*/

    emit hpsdrDeviceNICChanged(index);
}

void Settings::setServerAddr(QString addr) {

    QMutexLocker locker(&settingsMutex);

    m_serverAddress = addr;

    locker.unlock();
    emit serverAddrChanged(m_serverAddress);
}

QString Settings::getServerAddr() {

    return m_serverAddress;
}

void Settings::setHPSDRDeviceLocalAddr(QString addr) {

    QMutexLocker locker(&settingsMutex);

    m_hpsdrDeviceLocalAddr = addr;

    locker.unlock();
    emit hpsdrDeviceLocalAddrChanged(m_hpsdrDeviceLocalAddr);
}

QString Settings::getHPSDRDeviceLocalAddr() {

    return m_hpsdrDeviceLocalAddr;
}

void Settings::setServerPort(quint16 port) {

    QMutexLocker locker(&settingsMutex);

    m_serverPort = port;

    locker.unlock();
    emit serverPortChanged(m_serverPort);
}

quint16 Settings::getServerPort() {

    return m_serverPort;
}

void Settings::setListenPort(quint16 port) {

    QMutexLocker locker(&settingsMutex);

    m_listenerPort = port;

    locker.unlock();
    emit listenPortChanged(m_listenerPort);
}

quint16 Settings::getListenPort() {

    return m_listenerPort;
}

void Settings::setAudioPort(quint16 port) {

    QMutexLocker locker(&settingsMutex);
    m_audioPort = port;
    locker.unlock();

    emit audioPortChanged(m_audioPort);
}

quint16 Settings::getAudioPort() {

    return m_audioPort;
}

void Settings::setMetisPort(quint16 port) {

    QMutexLocker locker(&settingsMutex);
    m_metisPort = port;
    locker.unlock();

    emit metisPortChanged(m_metisPort);
}

quint16 Settings::getMetisPort() {

    return m_metisPort;
}

void Settings::setClientConnected(bool value) {

    QMutexLocker locker(&settingsMutex);
    m_clientConnected = value;
    locker.unlock();

    emit clientConnectedChanged(m_clientConnected);
}

bool Settings::getClientConnected() {

    return m_clientConnected;
}

void Settings::setClientNoConnected(int client) {

    QMutexLocker locker(&settingsMutex);
    m_clientNoConnected = client;
    locker.unlock();

    emit clientNoConnectedChanged(m_clientNoConnected);
}

void Settings::setAudioRx(int rx) {

    emit audioRxChanged(rx);
}

void Settings::setConnected(bool value) {

    QMutexLocker locker(&settingsMutex);
    m_connected = value;
    locker.unlock();

    emit connectedChanged(m_connected);
}

bool Settings::getConnected() {

    return m_connected;
}

void Settings::clientDisconnected(int client) {

    emit clientDisconnectedEvent(client);
}

void Settings::setRxConnectedStatus(int rx, bool value) {

    emit rxConnectedStatusChanged(rx, value);
}

void Settings::setSocketBufferSize(int value) {

    m_socketBufferSize = value;
    //SETTINGS_DEBUG << "m_socketBufferSize = " << value;
    emit socketBufferSizeChanged(value);
}

void Settings::setManualSocketBufferSize(bool value) {

    m_manualSocketBufferSize = value;
    //SETTINGS_DEBUG << "m_manualSocketBufferSize = " << value;
    emit manualSocketBufferChanged(m_manualSocketBufferSize);
}


//*******************************
// HPSDR hardware presence and firmware versions

THPSDRDevices Settings::getHPSDRDevices() {

    return m_devices;
}

void Settings::setHPSDRDevices(THPSDRDevices devices) {

    Q_UNUSED(devices)
}

void Settings::checkHPSDRDevices() {

    SETTINGS_DEBUG << "mercuryPresence: " << m_devices.mercuryPresence;
    SETTINGS_DEBUG << "penelopePresence: " << m_devices.penelopePresence;
    SETTINGS_DEBUG << "pennylanePresence: " << m_devices.pennylanePresence;
    SETTINGS_DEBUG << "excaliburPresence: " << m_devices.excaliburPresence;
    SETTINGS_DEBUG << "alexPresence: " << m_devices.alexPresence;
    SETTINGS_DEBUG << "hermesPresence: " << m_devices.hermesPresence;

    if (m_hpsdrHardware == 0) { // 0 = Mercury/Penelope

        if (m_devices.penelopePresence && m_devices.pennylanePresence) {

            m_devices.pennylanePresence = false;
            m_devices.penelopePresence = true;
            SETTINGS_DEBUG << "settings specifies both Penelope and Pennylane - choosing Penelope !";
        }

        if (m_devices.hermesPresence) {

            m_devices.hermesPresence = false;
            SETTINGS_DEBUG << "settings specifies also Hermes - choosing Mercury/Penelope/Pennylane !";
        }
    } else if (m_hpsdrHardware == 1) { // 1 = Hermes

        if (m_devices.mercuryPresence ||
            m_devices.penelopePresence ||
            m_devices.pennylanePresence ||
            m_devices.excaliburPresence) {
            m_devices.mercuryPresence = false;
            m_devices.penelopePresence = false;
            m_devices.pennylanePresence = false;
            m_devices.excaliburPresence = false;
            SETTINGS_DEBUG << "settings specifies HPSDR Modules - choosing Hermes !";
        }
    }
}


void Settings::setHPSDRHardware(int value) {

    m_hpsdrHardware = value; // 0 = Mercury/Penelope, 1 = Hermes, 2 = Cyclops

    emit hpsdrHardwareChanged(m_hpsdrHardware);
}

void Settings::setHermesVersion(int value) {

    QMutexLocker locker(&settingsMutex);
    m_devices.hermesFWVersion = value;
    locker.unlock();

    emit hermesVersionChanged(m_devices.hermesFWVersion);
}

void Settings::setMercuryPresence(bool value) {

    m_devices.mercuryPresence = value;

    emit mercuryPresenceChanged(m_devices.mercuryPresence);
}

void Settings::setMercuryVersion(int value) {

    QMutexLocker locker(&settingsMutex);
    m_devices.mercuryFWVersion = value;
    locker.unlock();

    emit mercuryVersionChanged(m_devices.mercuryFWVersion);
}

void Settings::setPenelopePresence(bool value) {

    m_devices.penelopePresence = value;
    setTxAllowed(value);

    emit penelopePresenceChanged(m_devices.penelopePresence);
}

void Settings::setPenelopeVersion(int value) {

    QMutexLocker locker(&settingsMutex);
    m_devices.penelopeFWVersion = value;
    locker.unlock();

    emit penelopeVersionChanged(m_devices.penelopeFWVersion);
}

void Settings::setPennyLanePresence(bool value) {

    m_devices.pennylanePresence = value;
    setTxAllowed(value);

    emit pennyLanePresenceChanged(m_devices.pennylanePresence);
}

void Settings::setPennyLaneVersion(int value) {

    QMutexLocker locker(&settingsMutex);
    m_devices.pennylaneFWVersion = value;
    locker.unlock();

    emit pennyLaneVersionChanged(m_devices.pennylaneFWVersion);
}

void Settings::setAlexPresence(bool value) {

    m_devices.alexPresence = value;

    emit alexPresenceChanged(m_devices.alexPresence);
}

void Settings::setExcaliburPresence(bool value) {

    m_devices.excaliburPresence = value;

    emit excaliburPresenceChanged(m_devices.excaliburPresence);
}

void Settings::setMetisVersion(int value) {

    QMutexLocker locker(&settingsMutex);
    m_devices.metisFWVersion = value;
    locker.unlock();

    emit metisVersionChanged(m_devices.metisFWVersion);
}

void Settings::setCheckFirmwareVersion(bool value) {

    m_checkFirmwareVersions = value;

    emit checkFirmwareVersionChanged(value);
}

void Settings::setProtocolSync(int value) {

    emit protocolSyncChanged(value);
}

void Settings::setADCOverflow(int value) {

    emit adcOverflowChanged(value);
}

void Settings::setPacketLoss(int value) {

    emit packetLossChanged(value);
}

void Settings::setForwardPower(qreal watts) {

    emit forwardPowerChanged(watts);
}

void Settings::setReversePower(qreal watts) {

    emit reversePowerChanged(watts);
}

void Settings::setSWR(qreal swr) {

    emit swrChanged(swr);
}

void Settings::setSupplyVoltage(qreal volts) {

    emit supplyVoltageChanged(volts);
}

void Settings::setTemperature(qreal temp) {

    emit temperatureChanged(temp);
}

void Settings::setSendIQ(int value) {

    emit sendIQSignalChanged(value);
}

void Settings::setRcveIQ(int value) {

    emit rcveIQSignalChanged(value);
}

/**
 * Set the number of receivers to be supported by this server
 * \param r The number of receivers: 0 to 6
 * This value is embedded into the command & control bytes that are sent to Mercury.
 * Thus it determines how the I & Q samples read from EP6 are placed in the data stream to dspservers.
 */
void Settings::setReceivers(int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_mercuryReceivers == value) return;
    if (value > MAX_RECEIVERS) value = MAX_RECEIVERS;

    m_mercuryReceivers = value;
    locker.unlock();

    SETTINGS_DEBUG << "set number of receivers to: " << m_mercuryReceivers;
    emit numberOfRXChanged(value);
}

//void Settings::setReceiver(int value) {
//
//	QMutexLocker locker(&settingsMutex);
//
//	if (m_currentReceiver == value) return;
//	if (value > MAX_RECEIVERS) value = MAX_RECEIVERS;
//
//	m_currentReceiver = value;
//	locker.unlock();
//
//	SETTINGS_DEBUG << "switch to receiver: " << m_currentReceiver;
//	emit receiverChanged(value);
//	emit frequencyChanged(true, value, m_receiverDataList[value].frequency);
//}

void Settings::setCurrentReceiver(int value) {

    QMutexLocker locker(&settingsMutex);

    if (value > MAX_RECEIVERS) {

        SETTINGS_DEBUG << "receiver number > MAX_RECEIVERS; setting to MAX_RECEIVERS.";
        value = MAX_RECEIVERS;
    }

    m_currentReceiver = value;

    HamBand band = m_receiverDataList.at(m_currentReceiver).hamBand;
    DSPMode mode = m_receiverDataList.at(m_currentReceiver).dspModeList[band];
    locker.unlock();

    setMercuryAttenuator(m_receiverDataList.at(m_currentReceiver).mercuryAttenuators.at(band));
    setFramesPerSecond(m_currentReceiver, m_receiverDataList.at(m_currentReceiver).framesPerSecond);

    SETTINGS_DEBUG << "switch to receiver: " << m_currentReceiver;
    emit currentReceiverChanged(value);
    //emit frequencyChanged(true, value, m_receiverDataList.at(m_currentReceiver).frequency);
    long vfoF = m_receiverDataList.at(m_currentReceiver).vfoFrequency;
    long ctrF = m_receiverDataList.at(m_currentReceiver).ctrFrequency;

    emit ctrFrequencyChanged(true, value, ctrF);
    emit vfoFrequencyChanged(true, value, vfoF);
    emit ncoFrequencyChanged(m_currentReceiver, vfoF - ctrF);
    emit hamBandChanged(m_currentReceiver, false, band);
    emit dspModeChanged(m_currentReceiver, mode);

    // Sync the panadapter filter lines to the saved filter for this receiver.
    // Without this, setDSPMode only updates the mode label; the filter shading
    // stays at whatever filterLo/Hi the panel read at construction time.
    emit filterFrequenciesChanged(m_currentReceiver,
        m_receiverDataList.at(m_currentReceiver).filterLo,
        m_receiverDataList.at(m_currentReceiver).filterHi);

    emit mouseWheelFreqStepChanged(m_currentReceiver,
    m_receiverDataList.at(m_currentReceiver).mouseWheelFreqStep);


}

void Settings::setSampleRate(int value) {

    QMutexLocker locker(&settingsMutex);

    int speed = 0;
    int outputIncrement = 0;
    if (!sampleRateToParams(value, speed, outputIncrement)) {
        SETTINGS_DEBUG << "Invalid sample rate (must be 48, 96, 192, 384, 768 or 1536 kHz)!\n";
        return;
    }

    m_sampleRate = value;
    m_mercurySpeed = speed;
    m_outputSampleIncrement = outputIncrement;

    for (int i = 0; i < MAX_RECEIVERS; i++)
        m_receiverDataList[i].sampleRate = m_sampleRate;

    emit sampleRateChanged(m_sampleRate);
}

void Settings::setMercuryAttenuator(int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(m_currentReceiver).mercuryAttenuators.length() != MAX_BANDS)
        return;

    HamBand band = m_receiverDataList[m_currentReceiver].hamBand;
    m_receiverDataList[m_currentReceiver].mercuryAttenuators[band] = value;

    emit mercuryAttenuatorChanged(band, value);
}

QList<int> Settings::getMercuryAttenuators(int rx) {

    return m_receiverDataList[rx].mercuryAttenuators;
}

void Settings::setDither(int value) {

    QMutexLocker locker(&settingsMutex);
    m_mercuryDither = value;

    emit ditherChanged(value);
}

void Settings::setRandom(int value) {

    QMutexLocker locker(&settingsMutex);
    m_mercuryRandom = value;

    emit randomChanged(value);
}

void Settings::set10MhzSource(int source) {

    QMutexLocker locker(&settingsMutex);

    m_10MHzSource = source;
    emit src10MhzChanged(source);
}

void Settings::set122_88MhzSource(int source) {

    QMutexLocker locker(&settingsMutex);

    m_122_8MHzSource = source;
    emit src122_88MhzChanged(source);
}

void Settings::setMicSource(int source) {

    QMutexLocker locker(&settingsMutex);

    m_micSource = source;
    emit micSourceChanged(source);
}

void Settings::setMicInputDev(int index) {

    QMutexLocker locker(&settingsMutex);

    m_micInputDev = index;
    emit micInputChanged(index);
}

void Settings::setMicInputSourceName(const QString &name) {

	QMutexLocker locker(&settingsMutex);
	m_micInputSourceName = name;
}

void Settings::setDigitalInputSourceName(const QString &name) {

    QMutexLocker locker(&settingsMutex);
    m_digitalInputSourceName = name;
}

void Settings::setDigitalAudioInputDev(int index) {

    QMutexLocker locker(&settingsMutex);

    m_digitalAudioInputDev = index;
    emit digitalAudioInputChanged(index);
}

void Settings::setMicInputLevel(int level) {

    QMutexLocker locker(&settingsMutex);

    m_micGain = level;
    emit micInputLevelChanged(level);
}

void Settings::setDriveLevel(int level) {

    QMutexLocker locker(&settingsMutex);

    m_drivelevel = level;
    emit driveLevelChanged(level);
}





void Settings::setClass(int value) {

    QMutexLocker locker(&settingsMutex);

    m_RxClass = value;
    emit classChanged(value);
}

void Settings::setTiming(int value) {

    QMutexLocker locker(&settingsMutex);

    m_RxTiming = value;
    emit timingChanged(value);
}

void Settings::setMouseWheelFreqStep(int rx, qreal value) {

    QMutexLocker locker(&settingsMutex);

    //m_mouseWheelFreqStep = value;
    m_receiverDataList[rx].mouseWheelFreqStep = value;
    emit mouseWheelFreqStepChanged(rx, value);
}

double Settings::getMouseWheelFreqStep(int rx) {

    return m_receiverDataList[rx].mouseWheelFreqStep;
}

qreal Settings::getMainVolume(int rx) {

    return m_receiverDataList[rx].audioVolume;
}

void Settings::setMainVolume(int rx, float volume) {

    if (volume < 0) volume = 0.0f;
    if (volume > 1) volume = 1.0f;

    QMutexLocker locker(&settingsMutex);

    //if (m_receiverDataList[rx].audioVolume == volume) return;
    m_receiverDataList[rx].audioVolume = volume;

    emit mainVolumeChanged(rx, volume);
}

void Settings::setMainVolumeMute(int rx, bool value) {

    Q_UNUSED(value)

    qreal vol = getMainVolume(rx);
    if (value)
        setMainVolume(rx, 0.0f);
    else
        setMainVolume(rx, vol);
}

void Settings::setCtrFrequency(int rx, long frequency) {

    QMutexLocker locker(&settingsMutex);

    HamBand band = getBandFromFrequency(m_bandList, frequency);

    m_receiverDataList[rx].ctrFrequency = frequency;
    //m_receiverDataList[rx].hamBand = band;
    //m_receiverDataList[rx].lastHamBand = band;
    m_receiverDataList[rx].lastCenterFrequencyList[(int) band] = frequency;
}

void Settings::setVfoFrequency(int rx, long frequency) {

    QMutexLocker locker(&settingsMutex);

    HamBand band = getBandFromFrequency(m_bandList, frequency);

    m_receiverDataList[rx].vfoFrequency = frequency;
    m_receiverDataList[rx].hamBand = band;
    m_receiverDataList[rx].lastHamBand = band;
    m_receiverDataList[rx].lastVfoFrequencyList[(int) band] = frequency;

    m_receiverDataList[rx].ncoFrequency = frequency - m_receiverDataList.at(rx).ctrFrequency;
 //   setDSPMode(rx,m_receiverDataList[rx].dspModeList[band]);
    SETTINGS_DEBUG << "set vfo freq (Rx " << rx << ") " << m_receiverDataList[rx].ctrFrequency;
}

void Settings::setCtrFrequency(int mode, int rx, long frequency) {

    QMutexLocker locker(&settingsMutex);
    m_receiverDataList[rx].ctrFrequency = frequency;

    HamBand band = getBandFromFrequency(m_bandList, frequency);
    m_receiverDataList[rx].lastCenterFrequencyList[(int) band] = frequency;
    locker.unlock();

    switch (mode) {

        case 0:
            break;

        case 1:

            setVFOFrequency(0, rx, frequency);
            break;
    }

    SETTINGS_DEBUG << "ctr freq (Rx " << rx << ") " << m_receiverDataList[rx].ctrFrequency;

    const DSPMode currentMode = m_receiverDataList.at(rx).dspModeList.at(m_receiverDataList.at(rx).hamBand);
    if (currentMode == FDV) {
        const DSPMode sideband = resolveWDSPMode(FDV, frequency);
        setRXFilter(rx,
            m_defaultFilterList.at((int) sideband).filterLo,
            m_defaultFilterList.at((int) sideband).filterHi);
    }

    emit ctrFrequencyChanged(mode, rx, frequency);
}

long Settings::getCtrFrequency(int rx) {

    return m_receiverDataList.at(rx).ctrFrequency;
}

void Settings::setVFOFrequency(int mode, int rx, long frequency) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).vfoFrequency == frequency) return;
    m_receiverDataList[rx].vfoFrequency = frequency;
    SETTINGS_DEBUG << "vfo freq (Rx " << rx << ") " << m_receiverDataList[rx].vfoFrequency;
    if (frequency > 60000000)
        SETTINGS_DEBUG << "frequency out of expected range: " << frequency;

    HamBand band = getBandFromFrequency(m_bandList, frequency);
    m_receiverDataList[rx].lastVfoFrequencyList[(int) band] = frequency;

    locker.unlock();
    if (m_receiverDataList.at(rx).hamBand != band) {

        //m_receiverDataList[rx].ctrFrequency = m_receiverDataList[rx].vfoFrequency;
        setHamBand(rx, false, band);
    }

    switch (mode) {

        case 0: // change only VFO

            m_receiverDataList[rx].ncoFrequency = frequency - m_receiverDataList.at(rx).ctrFrequency;
            SETTINGS_DEBUG << "nco freq = " << m_receiverDataList[rx].ncoFrequency << "rx frequency = " << frequency << "Ctr Frequnecy =" << m_receiverDataList.at(rx).ctrFrequency;
            break;

        case 1: // change VFO and center freq; keep NCO frequency

            setCtrFrequency(0, rx, frequency - m_receiverDataList.at(rx).ncoFrequency);
            break;

        case 2: // change VFO, set center frequency from lastCenterFrequencyList

            setCtrFrequency(0, rx, m_receiverDataList.at(rx).lastCenterFrequencyList.at((int) band));
            m_receiverDataList[rx].ncoFrequency = frequency - m_receiverDataList.at(rx).ctrFrequency;
            break;
    }

    emit vfoFrequencyChanged(mode, rx, frequency);

    SETTINGS_DEBUG << "nco freq (Rx " << rx << ")" << m_receiverDataList[rx].ncoFrequency ;
    emit ncoFrequencyChanged(rx, m_receiverDataList[rx].ncoFrequency);

}

long Settings::getVfoFrequency(int rx) {

    return m_receiverDataList.at(rx).vfoFrequency;
}

void Settings::setNCOFrequency(bool value, int rx, long frequency) {

    Q_UNUSED(value)

    SETTINGS_DEBUG << "nco freq (Rx " << rx << ") " << m_receiverDataList[rx].ncoFrequency << "(direct)";
    m_receiverDataList[rx].ncoFrequency = frequency;

    emit ncoFrequencyChanged(rx, frequency);
}

void Settings::setHamBand(int rx, bool byButton, HamBand band) {

    QMutexLocker locker(&settingsMutex);
    qDebug() << "ham band" << m_receiverDataList[rx].hamBand << "band" << band;
    if (m_receiverDataList[rx].hamBand == band)
        return;

    m_receiverDataList[rx].lastHamBand = m_receiverDataList[rx].hamBand;
    m_receiverDataList[rx].hamBand = band;

    SETTINGS_DEBUG << "last Ham band:  " << m_receiverDataList[rx].lastHamBand;
    SETTINGS_DEBUG << "Ham band:  " << m_receiverDataList[rx].hamBand;

    if (m_receiverDataList[rx].hamBand == (HamBand) gen)
        setTxAllowed(false);
    else
        setTxAllowed(true);

    locker.unlock();

    setMercuryAttenuator(m_receiverDataList[rx].mercuryAttenuators[band]);

    emit hamBandChanged(rx, byButton, band);
}

HamBand Settings::getCurrentHamBand(int rx) {

    return m_receiverDataList[rx].hamBand;
}

void Settings::setDSPMode(int rx, DSPMode mode) {

    SETTINGS_DEBUG << "DSP mode change " << mode << rx;
    if (rx < 0 || rx >= m_receiverDataList.size())
        return;

    HamBand band = m_receiverDataList[rx].hamBand;
    m_receiverDataList[rx].dspModeList[band] = mode;


    const DSPMode wdspMode = resolveWDSPMode(mode, m_receiverDataList[rx].ctrFrequency);
    setRXFilter(rx, m_defaultFilterList.at((int) wdspMode).filterLo, m_defaultFilterList.at((int) wdspMode).filterHi);
    emit dspModeChanged(rx, mode);
}

int Settings::getFreeDVMode(int rx) {

	if (rx < 0 || rx >= m_freeDVModeList.size()) return 0;
	return m_freeDVModeList.at(rx);
}

QString Settings::getCodec2ModeString(int mode) {
	// Return Codec2/FreeDV mode name and bitrate
	switch(mode) {
		case 0:  return "1600 bps (FREEDV_MODE_1600)";
		case 1:  return "1400 bps (FREEDV_MODE_1400)";
		case 2:  return "1300 bps (FREEDV_MODE_1300)";
		case 3:  return "700C bps (FREEDV_MODE_700C)";
		case 4:  return "2400 bps (FREEDV_MODE_2400)";
		case 5:  return "3200 bps (FREEDV_MODE_3200)";
		case 6:  return "700D bps (FREEDV_MODE_700D)";
		case 8:  return "2020 (FREEDV_MODE_2020)";
		case 16: return "2020B (FREEDV_MODE_2020B)";
		default: return "1600 bps (FREEDV_MODE_1600)";
	}
}

QList<int> Settings::availableCodec2Modes() {
	// Return list of available Codec2 modes
	return {0, 1, 2, 3, 4, 5, 6, 8, 16};
}

AGCMode Settings::getAGCMode(int rx) {

    return m_receiverDataList.at(rx).agcMode;
}

ADCMode Settings::getADCMode(int rx) {

    return m_receiverDataList.at(rx).adcMode;
}

QString Settings::getADCModeString(int rx) {

    ADCMode mode = getADCMode(rx);
    QString str;
    if (mode == adc1)
        str = "ADC1";
    else
        str = "ADC2";

    return str;
}

QString Settings::getAGCModeString(int rx) {

    AGCMode mode = getAGCMode(rx);
    QString str;
    switch (mode) {

        case (AGCMode) agcOFF:
            str = "Off";
            break;

        case (AGCMode) agcLONG:
            str = "Long";
            break;

        case (AGCMode) agcSLOW:
            str = "Slow";
            break;

        case (AGCMode) agcMED:
            str = "Med";
            break;

        case (AGCMode) agcFAST:
            str = "Fast";
            break;

        case (AGCMode) agcUser:
            str = "User";
            break;
    }
    return str;
}

void Settings::setADCMode(int rx, ADCMode mode) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].adcMode == mode) return;
    m_receiverDataList[rx].adcMode = mode;

    emit adcModeChanged(rx, mode);
}

void Settings::setAGCMode(int rx, AGCMode mode) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcMode == mode) return;
    m_receiverDataList[rx].agcMode = mode;

    bool hang;
    if (mode == (AGCMode) agcOFF || mode == (AGCMode) agcMED || mode == (AGCMode) agcFAST) {

        m_receiverDataList[rx].hangEnabled = false;
        hang = false;
        if (mode == (AGCMode) agcOFF)
                emit agcFixedGainChanged_dB(rx, m_receiverDataList[rx].agcFixedGain_dB);
    } else {

        m_receiverDataList[rx].hangEnabled = true;
        hang = true;
    }

    emit agcModeChanged(rx, mode, hang);
    emit agcHangEnabledChanged(rx, hang);
}

void Settings::setAGCShowLines(int rx, bool value) {

    if (m_receiverDataList[rx].agcLines == value) return;
    m_receiverDataList[rx].agcLines = value;

    emit showAGCLinesStatusChanged(m_receiverDataList[rx].agcLines, rx);
}

qreal Settings::getAGCGain(int rx) {

    return m_receiverDataList[rx].acgGain;
}

void Settings::setAGCGain(int rx, int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].acgGain == value) return;
    m_receiverDataList[rx].acgGain = value;
    //SETTINGS_DEBUG << "acgGain " << value;
    emit agcGainChanged(rx, value);
}

void Settings::setAGCMaximumGain_dB(int rx, qreal value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcMaximumGain_dB == value) return;
    m_receiverDataList[rx].agcMaximumGain_dB = value;

    SETTINGS_DEBUG << "set agcMaximumGain_dB = " << m_receiverDataList[rx].agcMaximumGain_dB;
    emit agcMaximumGainChanged_dB(rx, value);
}

int Settings::getAGCMaximumGain_dB(int rx) {

    return m_receiverDataList[rx].agcMaximumGain_dB;
}

void Settings::setAGCFixedGain_dB(int rx, qreal value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcFixedGain_dB == value) return;
    m_receiverDataList[rx].agcFixedGain_dB = value;

    SETTINGS_DEBUG << "m_receiverDataList[rx].agcFixedGain_dB = " << m_receiverDataList[rx].agcFixedGain_dB;
    emit agcFixedGainChanged_dB(rx, value);
}

qreal Settings::getAGCFixedGain_dB(int rx) {

    return m_receiverDataList[rx].agcFixedGain_dB;
}

void Settings::setAGCThreshold_dB(int rx, qreal value) {

    QMutexLocker locker(&settingsMutex);
    SETTINGS_DEBUG << "acgThreshold = " << value;
    if (m_receiverDataList[rx].acgThreshold_dB == value) return;
    m_receiverDataList[rx].acgThreshold_dB = value;

    SETTINGS_DEBUG << "acgThreshold = " << m_receiverDataList[rx].acgThreshold_dB;
    emit agcThresholdChanged_dB(rx, value);
}

void Settings::setAGCHangThresholdSlider(int rx, qreal value) {

    emit agcHangThresholdSliderChanged(rx, value);
}

int Settings::getAGCHangThreshold(int rx) {

    return m_receiverDataList[rx].agcHangThreshold;
}

void Settings::setAGCHangThreshold(int rx, int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcHangThreshold == value) return;
    m_receiverDataList[rx].agcHangThreshold = value;

    //SETTINGS_DEBUG << "agcHangThreshold = " << m_receiverDataList[rx].agcHangThreshold;
    emit agcHangThresholdChanged(rx, value);
}

int Settings::getAGCHangLeveldB(int rx) {

    return m_receiverDataList[rx].agcHangThreshold;
}

void Settings::setAGCHangLevel_dB(int rx, qreal value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcHangLevel == value) return;
    m_receiverDataList[rx].agcHangLevel = value;

    //SETTINGS_DEBUG << "agcHangLevel = " << m_receiverDataList[rx].agcHangLevel;
    emit agcHangLevelChanged_dB(rx, value);
}

void Settings::setAGCLineLevels(int rx, qreal thresh, qreal hang) {

    if (m_currentReceiver != rx) return;
    if ((m_receiverDataList[rx].agcHangLevel == hang) && (m_receiverDataList[rx].acgThreshold_dB == thresh)) return;

    m_receiverDataList[rx].agcHangLevel = hang;
    m_receiverDataList[rx].acgThreshold_dB = thresh;
    SETTINGS_DEBUG << "SET agcHangLevel = " << m_receiverDataList[rx].agcHangLevel;
    emit agcLineLevelsChanged(rx, thresh, hang);
}

void Settings::setAGCVariableGain_dB(int rx, qreal value) {

    if (m_currentReceiver != rx) return;

    if (m_receiverDataList[rx].agcSlope == value) return;
    m_receiverDataList[rx].agcSlope = value;

    SETTINGS_DEBUG << "agcSlope = " << m_receiverDataList[rx].agcSlope;
    emit agcVariableGainChanged_dB(rx, value);
}

void Settings::setAGCAttackTime(int rx, qreal value) {

    if (m_currentReceiver != rx) return;

    if (m_receiverDataList[rx].agcAttackTime == value) return;
    m_receiverDataList[rx].agcAttackTime = value;

    SETTINGS_DEBUG << "agcAttackTime = " << m_receiverDataList[rx].agcAttackTime;
    emit agcAttackTimeChanged(rx, value);
}

void Settings::setAGCDecayTime(int rx, qreal value) {

    if (m_currentReceiver != rx) return;

    if (m_receiverDataList[rx].agcDecayTime == value) return;
    m_receiverDataList[rx].agcDecayTime = value;

    SETTINGS_DEBUG << "agcDecayTime = " << m_receiverDataList[rx].agcDecayTime;
    emit agcDecayTimeChanged(rx, value);
}

void Settings::setAGCHangTime(int rx, qreal value) {

    if (m_currentReceiver != rx) return;

    if (m_receiverDataList[rx].agcHangTime == value) return;
    m_receiverDataList[rx].agcHangTime = value;

    SETTINGS_DEBUG << "agcHangTime = " << m_receiverDataList[rx].agcHangTime;
    emit agcHangTimeChanged(rx, value);
}

void Settings::setRXFilter(int rx, qreal low, qreal high) {

    QMutexLocker locker(&settingsMutex);

    // Guard per-receiver: only suppress if THIS receiver's values haven't changed.
    // The old global m_filterFrequencyLow/High guard incorrectly suppressed updates
    // for rx1 when rx0 had already set the same filter values.
    if (m_receiverDataList[rx].filterLo == low && m_receiverDataList[rx].filterHi == high) return;

    m_receiverDataList[rx].filterLo = m_filterFrequencyLow = low;
    m_receiverDataList[rx].filterHi = m_filterFrequencyHigh = high;

    SETTINGS_DEBUG << "filter freq changed" << low << high;
    emit filterFrequenciesChanged(rx, low, high);
}



void Settings::setIQPort(int rx, int port) {

    emit iqPortChanged(rx, port);
}

void Settings::setSpectrumBuffer(int rx, const QList<float> &buffer) {

    emit spectrumBufferChanged(rx, buffer);
}

void Settings::setPostSpectrumBuffer(int rx, const float *buffer) {

    emit postSpectrumBufferChanged(rx, buffer);
}

void Settings::setSMeterValue(int rx, double value) {

    emit sMeterValueChanged(rx, value);
}

void Settings::setFreeDVStatus(int rx, bool sync, float snr, quint64 rxFrames) {

    if (rx < 0 || rx >= m_freeDVSyncList.size()) return;

    m_freeDVSyncList[rx] = sync;
    m_freeDVSnrList[rx] = snr;
    m_freeDVRxFramesList[rx] = rxFrames;

    emit freeDVStatusChanged(
        rx,
        m_freeDVSyncList[rx],
        m_freeDVSnrList[rx],
        m_freeDVRxFramesList[rx],
        m_freeDVTxFramesList[rx]);
}

void Settings::addFreeDVTxFrames(int rx, quint64 txFrames) {

    if (rx < 0 || rx >= m_freeDVTxFramesList.size()) return;

    m_freeDVTxFramesList[rx] += txFrames;

    emit freeDVStatusChanged(
        rx,
        m_freeDVSyncList[rx],
        m_freeDVSnrList[rx],
        m_freeDVRxFramesList[rx],
        m_freeDVTxFramesList[rx]);
}

void Settings::setFreeDVMode(int rx, int mode) {

    if (rx < 0 || rx >= m_freeDVModeList.size()) return;
    if (m_freeDVModeList[rx] == mode) return;

    m_freeDVModeList[rx] = mode;
    m_freeDVSyncList[rx] = false;
    m_freeDVSnrList[rx] = 0.0f;
    m_freeDVRxFramesList[rx] = 0;
    m_freeDVTxFramesList[rx] = 0;

    emit freeDVModeChanged(rx, mode);
    emit freeDVStatusChanged(rx, false, 0.0f, 0, 0);
}

void Settings::setReceiverDataReady() {

    emit receiverDataReady();
}

void Settings::setSampleSize(int rx, int size) {

    SETTINGS_DEBUG << "set sample size to: " << size << " for Rx " << rx;
    switch (size) {

        case 4096:
            m_receiverDataList[rx].fftFactor = 1;
            break;

        case 8192:
            m_receiverDataList[rx].fftFactor = 2;
            break;

        case 16384:
            m_receiverDataList[rx].fftFactor = 4;
            break;

        case 32768:
            m_receiverDataList[rx].fftFactor = 8;
            break;

        case 65536:
            m_receiverDataList[rx].fftFactor = 16;
            break;

        case 131072:
            m_receiverDataList[rx].fftFactor = 32;
            break;

        case 262144:
            m_receiverDataList[rx].fftFactor = 64;
            break;
    }

    emit sampleSizeChanged(rx, size);
}

int Settings::getFFTMultiplicator(int rx) {

    return m_receiverDataList.at(rx).fftFactor;
}

// Alex configuration:
//
// manual 		  0
// bypassAll 	  1
// amp6m		  2
// hpf1_5MHz	  3
// hpf6_5MHz	  4
// hpf9_5MHz	  5
// hpf13MHz		  6
// hpf20MHz		  7
// lpf160m		  8
// lpf80m		  9
// lpf60_40m	 10
// lpf30_20m	 11
// lpf17_15m	 12
// lpf12_10m	 13
// lpf6m		 14

// m_alexConfig (qint16)
//
// 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
//   | | | | | | | | | | | | | | |
//   | | | | | | | | | | | | | | +-----Alex   - manual HPF/LPF filter select (0 = disable, 1 = enable)
//   | | | | | | | | | | | | | +------ Alex   -	Bypass all HPFs   (0 = disable, 1 = enable)*
//   | | | | | | | | | | | | +-------- Alex   -	6M low noise amplifier (0 = disable, 1 = enable)*
//   | | | | | | | | | | | +---------- Alex   -	select 1.5MHz HPF (0 = disable, 1 = enable)*
//   | | | | | | | | | | +------------ Alex   -	select 6.5MHz HPF (0 = disable, 1 = enable)*
//   | | | | | | | | | +-------------- Alex   -	select 9.5MHz HPF (0 = disable, 1 = enable)*
//   | | | | | | | | +---------------- Alex   -	select 13MHz  HPF (0 = disable, 1 = enable)*
//   | | | | | | | +------------------ Alex   -	select 20MHz  HPF (0 = disable, 1 = enable)*
//   | | | | | | +-------------------- Alex   - select 160m   LPF (0 = disable, 1 = enable)*
//   | | | | | +---------------------- Alex   - select 80m    LPF (0 = disable, 1 = enable)*
//   | | | | +------------------------ Alex   - select 60/40m LPF (0 = disable, 1 = enable)*
//   | | | +-------------------------- Alex   - select 30/20m LPF (0 = disable, 1 = enable)*
//   | | +---------------------------- Alex   - select 17/15m LPF (0 = disable, 1 = enable)*
//   | +------------------------------ Alex   - select 12/10m LPF (0 = disable, 1 = enable)*
//   +-------------------------------- Alex   - select 6m     LPF (0 = disable, 1 = enable)*

void Settings::setAlexConfiguration(quint16 conf) {

    QMutexLocker locker(&settingsMutex);
    m_alexConfig = conf;

    emit alexConfigurationChanged(m_alexConfig);
}

void Settings::setAlexHPFLoFrequencies(int filter, long value) {

    m_HPFLoFrequencyList[filter] = value;
}

void Settings::setAlexHPFHiFrequencies(int filter, long value) {

    m_HPFHiFrequencyList[filter] = value;
}

void Settings::setAlexLPFLoFrequencies(int filter, long value) {

    m_LPFLoFrequencyList[filter] = value;
}

void Settings::setAlexLPFHiFrequencies(int filter, long value) {

    m_LPFHiFrequencyList[filter] = value;
}

/*
 * Alex state encoding
 * We use the same encoding as in the KISS Konsole by George Byrkit, K9TRV
 * AA.TT.BBB.RR
 * AA is RX Attenuator
 * TT is TX antenna selection (0 is NOT valid!)
 * BBB is RX special selection
 * RR is TX antenna to use for RX (0 is NOT valid!)
 *
 */
void Settings::setAlexState(int pos, int value) {

    if (m_alexStates.length() != MAX_BANDS)
        return;
    else {

        if (m_alexStates.at(pos) == value)
            return;

        int state = checkAlexState(value);
        //qDebug() << "alex state at pos: " << pos <<" = " << state;

        m_alexStates.replace(pos, state);

        emit alexStateChanged((HamBand) pos, m_alexStates);
    }
}

void Settings::setAlexState(int value) {

    HamBand band = m_receiverDataList[m_currentReceiver].hamBand;

    setAlexState(band, value);
}

void Settings::setAlexStates(const QList<int> &states) {

    if (m_alexStates == states) return;

    m_alexStates = states;

    emit alexStatesChanged(m_alexStates);
}

// check Alex state - adapted from KISS Konsole, (c) George Byrkit, K9TRV
int Settings::checkAlexState(int state) {

    if ((state & 0x3) == 0) {

        // if rx antenna selector is 0, set it to 1
        state |= 1;
    }

    if (((state >> 5) & 0x3) == 0) {

        // if tx antenna selector is 0, set it to 1
        state |= 33;
    }
    return state;
}

void Settings::setAlexToManual(bool value) {

    QMutexLocker locker(&settingsMutex);

    emit alexManualStateChanged(value);
}

void Settings::setRxJ6Pin(HamBand band, int value) {

    if (m_rxJ6pinList.length() != MAX_BANDS - 1) return;
    if (m_rxJ6pinList[band] == value) return;

    m_rxJ6pinList[band] = value;

    emit rxJ6PinsChanged(m_rxJ6pinList);
}

void Settings::setRxJ6Pins(const QList<int> &states) {

    //if (m_rxJ6pinList == states) return;

    m_rxJ6pinList = states;

    emit rxJ6PinsChanged(m_rxJ6pinList);
}

void Settings::setTxJ6Pin(HamBand band, int value) {

    if (m_txJ6pinList.length() != MAX_BANDS - 1) return;
    if (m_txJ6pinList[band] == value) return;

    m_txJ6pinList[band] = value;

    emit txJ6PinsChanged(m_txJ6pinList);
}

void Settings::setTxJ6Pins(const QList<int> &states) {

    //if (m_txJ6pinList == states) return;

    m_txJ6pinList = states;

    emit txJ6PinsChanged(m_txJ6pinList);
}

void Settings::setPennyOCEnabled(bool value) {

    if (m_pennyOCEnabled == value) return;

    m_pennyOCEnabled = value;

    emit pennyOCEnabledChanged(m_pennyOCEnabled);
}

//**************************************
//**************************************
// OpenCL stuff

//void Settings::setOpenCLDevices(QList<QCLDevice> devices) {
//
//	m_clDevices = devices;
//}

void Settings::setFreqRulerPosition(int rx, float position) {

    if (position < 0) position = 0;
    if (position > 1) position = 1;

    m_receiverDataList[rx].freqRulerPosition = position;
    emit freqRulerPositionChanged(rx, position);
}

//**********************************************************************************
// audio settings

void Settings::setAudioFormat(const QAudioFormat &format) {

    QMutexLocker locker(&settingsMutex);

    //if (m_format == format) return;
    m_format = format;

    emit audioFormatChanged(m_format);
}

void Settings::setAudioPosition(qint64 position) {

    emit audioPositionChanged(position);
}

void Settings::setAudioBuffer(qint64 position, qint64 length, const QByteArray &buffer) {

    emit audioBufferChanged(position, length, buffer);
}


//**********************************************************************************
// wideband data & options

void Settings::setWidebandSpectrumBuffer(const qVectorFloat &buffer) {

    emit widebandSpectrumBufferChanged(buffer);
}

void Settings::resetWidebandSpectrumBuffer() {

    emit widebandSpectrumBufferReset();
}

void Settings::setWidebandOptions(TWideband options) {

    QMutexLocker locker(&settingsMutex);

    m_widebandOptions = options;
    locker.unlock();
    emit widebandOptionsChanged(m_widebandOptions);
}

void Settings::setWidebandStatus(bool value) {

    QMutexLocker locker(&settingsMutex);

    if (m_widebandOptions.wideBandDisplayStatus == value) return;
    m_widebandOptions.wideBandDisplayStatus = value;
    locker.unlock();
    emit widebandStatusChanged(m_widebandOptions.wideBandDisplayStatus);
}

void Settings::setWidebandData(bool value) {

    QMutexLocker locker(&settingsMutex);

    if (m_widebandOptions.wideBandData == value) return;
    m_widebandOptions.wideBandData = value;
    locker.unlock();
    emit widebandDataChanged(m_widebandOptions.wideBandData);
}

void Settings::setWidebandBuffers(int value) {

    SETTINGS_DEBUG << "Set WidebandBuffers)! " << value ;
    QMutexLocker locker(&settingsMutex);
    m_widebandOptions.numberOfBuffers = value;
    locker.unlock();
}

void Settings::setWidebanddBmScaleMin(qreal value) {

    QMutexLocker locker(&settingsMutex);

    if (m_widebandOptions.dBmWBScaleMin == value) return;
    m_widebandOptions.dBmWBScaleMin = value;

    locker.unlock();
    emit widebanddBmScaleMinChanged(m_widebandOptions.dBmWBScaleMin);
}

void Settings::setWidebanddBmScaleMax(qreal value) {

    QMutexLocker locker(&settingsMutex);

    if (m_widebandOptions.dBmWBScaleMax == value) return;
    m_widebandOptions.dBmWBScaleMax = value;

    locker.unlock();
    emit widebanddBmScaleMaxChanged(m_widebandOptions.dBmWBScaleMax);
}

void Settings::setWideBandRulerPosition(float position) {

    if (m_widebandOptions.scalePosition == position) return;
    if (position < 0) position = 0;
    if (position > 1) position = 1;
    m_widebandOptions.scalePosition = position;

    emit wideBandScalePositionChanged(m_widebandOptions.scalePosition);
}


void Settings::setSpectrumSize(int value) {

    if (m_spectrumSize == value) return;

    m_spectrumSize = value;
    emit spectrumSizeChanged(m_spectrumSize);
}

void Settings::moveDisplayWidget(int value) {

    emit displayWidgetHeightChanged(value);
}


//*********************************
// color stuff

void Settings::setPanadapterColors(TPanadapterColors type) {

    if (type.panBackgroundColor != m_panadapterColors.panBackgroundColor)
        m_panadapterColors.panBackgroundColor = type.panBackgroundColor;

    if (type.waterfallColor != m_panadapterColors.waterfallColor)
        m_panadapterColors.waterfallColor = type.waterfallColor;

    if (type.panLineColor != m_panadapterColors.panLineColor)
        m_panadapterColors.panLineColor = type.panLineColor;

    if (type.panLineFilledColor != m_panadapterColors.panLineFilledColor)
        m_panadapterColors.panLineFilledColor = type.panLineFilledColor;

    if (type.panSolidTopColor != m_panadapterColors.panSolidTopColor)
        m_panadapterColors.panSolidTopColor = type.panSolidTopColor;

    if (type.panSolidBottomColor != m_panadapterColors.panSolidBottomColor)
        m_panadapterColors.panSolidBottomColor = type.panSolidBottomColor;

    if (type.wideBandLineColor != m_panadapterColors.wideBandLineColor)
        m_panadapterColors.wideBandLineColor = type.wideBandLineColor;

    if (type.wideBandFilledColor != m_panadapterColors.wideBandFilledColor)
        m_panadapterColors.wideBandFilledColor = type.wideBandFilledColor;

    if (type.wideBandSolidTopColor != m_panadapterColors.wideBandSolidTopColor)
        m_panadapterColors.wideBandSolidTopColor = type.wideBandSolidTopColor;

    if (type.wideBandSolidBottomColor != m_panadapterColors.wideBandSolidBottomColor)
        m_panadapterColors.wideBandSolidBottomColor = type.wideBandSolidBottomColor;

    if (type.distanceLineColor != m_panadapterColors.distanceLineColor)
        m_panadapterColors.distanceLineColor = type.distanceLineColor;

    if (type.distanceLineFilledColor != m_panadapterColors.distanceLineFilledColor)
        m_panadapterColors.distanceLineFilledColor = type.distanceLineFilledColor;

    if (type.panCenterLineColor != m_panadapterColors.panCenterLineColor)
        m_panadapterColors.panCenterLineColor = type.panCenterLineColor;

    if (type.gridLineColor != m_panadapterColors.gridLineColor)
        m_panadapterColors.gridLineColor = type.gridLineColor;

    emit panadapterColorChanged();
}

void Settings::setFramesPerSecond(int rx, int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).framesPerSecond != value)
        m_receiverDataList[rx].framesPerSecond = value;

    emit framesPerSecondChanged(rx, m_receiverDataList[rx].framesPerSecond);
}

int Settings::getFramesPerSecond(int rx) {

    return m_receiverDataList.at(rx).framesPerSecond;
}

void Settings::setSpectrumAveraging(int rx, bool value) {

    if (rx == -1) {
        m_widebandOptions.averagingCnt = value;
    } else {
        m_receiverDataList[rx].spectrumAveraging = value;
    }

    SETTINGS_DEBUG << "Averaging for Rx " << rx << " : " << value;
   // emit spectrumAveragingChanged(rx, value);
    emit spectrumAveragingCntChanged(rx, 100);
}

bool Settings::getSpectrumAveraging(int rx) {

    if (rx == -1)
        return m_widebandOptions.averaging;
    else
        return m_receiverDataList[rx].spectrumAveraging;
}

int Settings::getSpectrumAveragingCnt(int rx) {

    if (rx == -1)
        return m_widebandOptions.averagingCnt;
    else
        return m_receiverDataList[rx].averagingCnt;
}

void Settings::setSpectrumAveragingCnt(int rx, int value) {

 //   QMutexLocker locker(&settingsMutex);

    //if (m_specAveragingCnt == value) return
    if (rx == -1)
        m_widebandOptions.averagingCnt = value;
    else
        m_receiverDataList[rx].averagingCnt = value;
    qDebug() << "spec av" << value;
    emit spectrumAveragingCntChanged(rx, value);
}


void Settings::setPanGrid(bool value, int rx) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).panGrid == value) return;
    m_receiverDataList[rx].panGrid = value;

    emit panGridStatusChanged(m_receiverDataList.at(rx).panGrid, rx);
}

bool Settings::getPanGridStatus(int rx) {

    return m_receiverDataList[rx].panGrid;
}

void Settings::setPeakHold(bool value, int rx) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).peakHold == value) return;
    m_receiverDataList[rx].peakHold = value;

    emit peakHoldStatusChanged(m_receiverDataList.at(rx).peakHold, rx);
}

bool Settings::getPeakHoldStatus(int rx) {

    return m_receiverDataList.at(rx).peakHold;
}

void Settings::setPanLocked(bool value, int rx) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).panLocked == value) return;
    m_receiverDataList[rx].panLocked = value;

    emit panLockedStatusChanged(m_receiverDataList.at(rx).panLocked, rx);
}

bool Settings::getPanLockedStatus(int rx) {

    return m_receiverDataList[rx].panLocked;
}

void Settings::setClickVFO(bool value, int rx) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).clickVFO == value) return;
    m_receiverDataList[rx].clickVFO = value;

    emit clickVFOStatusChanged(m_receiverDataList.at(rx).clickVFO, rx);
}

bool Settings::getClickVFOStatus(int rx) {

    return m_receiverDataList[rx].clickVFO;
}

void Settings::setHairCross(bool value, int rx) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).hairCross == value) return;
    m_receiverDataList[rx].hairCross = value;

    emit hairCrossStatusChanged(m_receiverDataList.at(rx).hairCross, rx);
}

bool Settings::getHairCrossStatus(int rx) {

    return m_receiverDataList[rx].hairCross;
}


void Settings::setWaterfallTime(int rx, int value) {

    Q_UNUSED(rx)
    Q_UNUSED(value)
}

void Settings::setWaterfallOffesetLo(int rx, int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].waterfallOffsetLo == value) return;
    m_receiverDataList[rx].waterfallOffsetLo = value;

    emit waterfallOffesetLoChanged(rx, value);
}

void Settings::setWaterfallOffesetHi(int rx, int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].waterfallOffsetHi == value) return;
    m_receiverDataList[rx].waterfallOffsetHi = value;

    emit waterfallOffesetHiChanged(rx, value);
}

void Settings::setSMeterHoldTime(int value) {

    QMutexLocker locker(&settingsMutex);

    if (m_sMeterHoldTime == value) return;
    m_sMeterHoldTime = value;

    emit sMeterHoldTimeChanged(m_sMeterHoldTime);
}

void Settings::setdBmPanScaleMin(int rx, qreal value) {

    QMutexLocker locker(&settingsMutex);

    HamBand band = m_receiverDataList.at(m_currentReceiver).hamBand;
    m_receiverDataList[rx].dBmPanScaleMinList[band] = value;

    emit dBmScaleMinChanged(rx, value);
}

void Settings::setdBmPanScaleMax(int rx, qreal value) {

    QMutexLocker locker(&settingsMutex);

    HamBand band = m_receiverDataList.at(m_currentReceiver).hamBand;
    m_receiverDataList[rx].dBmPanScaleMaxList[band] = value;

    emit dBmScaleMaxChanged(rx, value);
}

void Settings::setdBmDistScaleMin(qreal value) {

    Q_UNUSED(value)
}

void Settings::setdBmDistScaleMax(qreal value) {

    Q_UNUSED(value)
}

// **********************************************************************

void Settings::showRadioPopupWidget() {

    if (m_radioPopupVisible)
        m_radioPopupVisible = false;
    else
        m_radioPopupVisible = true;


}

void Settings::setPanAveragingMode(int rx, PanAveragingMode mode) {

    if (m_receiverDataList.at(rx).panAvMode == mode) return;

    m_receiverDataList[rx].panAvMode = mode;

    qDebug() << "Pan average mode set to " << mode;

    emit panAveragingModeChanged(rx, mode);

}

void Settings::setPanDetectorMode(int rx, PanDetectorMode mode) {

    if (m_receiverDataList.at(rx).panDetMode == mode) return;

    m_receiverDataList[rx].panDetMode = mode;

    qDebug() << "Pan detector mode set to " << m_receiverDataList[rx].panDetMode;

    emit panDetectorModeChanged(rx, mode);
};


int Settings::getAGCSlope(int rx) {
    return m_receiverDataList[rx].agcSlope;

}

void Settings::setfftSize(int rx, int size) {
    if (m_receiverDataList[rx].fftsize == size) return;
    m_receiverDataList[rx].fftsize = size;
    qDebug() << "fftsize set to " << size;
    emit fftSizeChanged(rx, size);
}

void Settings::setfmsqLevel(int rx, int level) {

    if (m_receiverDataList[rx].fmsqLevel == level) return;
    m_receiverDataList[rx].fmsqLevel = level;
    qDebug() << "fm sq level set to " << level;
    emit fmsqLevelChanged(rx, level);

}


int Settings::getfftSize(int rx) {
    return m_receiverDataList[rx].fftsize;
}

int Settings::getNrAGC(int rx) {
    return m_receiverDataList[rx].nr_agc;
}


int Settings::getNr2GainMethod(int rx) {
    return m_receiverDataList[rx].nr2_gain_method;
}

int Settings::getNr2NpeMethod(int rx) {
    return m_receiverDataList[rx].nr2_npe_method;
}

bool Settings::getSnb(int rx) {
    return m_receiverDataList[rx].snb;
}

bool Settings::getAnf(int rx) {
    return m_receiverDataList[rx].anf;
}

bool Settings::getNr2ae(int rx) {
    return m_receiverDataList[rx].nr2_ae;
}


int Settings::getnbMode(int rx) {
    return m_receiverDataList[rx].nbMode;
}

int Settings::getnrMode(int rx) {
    return m_receiverDataList[rx].nr;
}


void Settings::setNoiseBlankerMode(int rx, int nb) {
    if (m_receiverDataList[rx].nbMode == nb) return;
    m_receiverDataList[rx].nbMode = nb;
    emit (noiseBlankerChanged(rx, nb));
}


void Settings::setNoiseFilterMode(int rx, int nr) {
    if (m_receiverDataList[rx].nr == nr) return;
    m_receiverDataList[rx].nr = nr;
    emit (noiseFilterChanged(rx, nr));
}

void Settings::setNR2Ae(int rx, bool value) {
    if (m_receiverDataList[rx].nr2_ae == value) return;
    m_receiverDataList[rx].nr2_ae = value;
    emit(nr2AeChanged(rx, value));
}

void Settings::setNR2GainMethod(int rx, int value) {
    if (m_receiverDataList[rx].nr2_gain_method == value) return;
    m_receiverDataList[rx].nr2_gain_method = value;
    emit(nr2GainMethodChanged(rx, value));
}

void Settings::setNR2NpeMethod(int rx, int value) {
    if (m_receiverDataList[rx].nr2_npe_method == value) return;
    m_receiverDataList[rx].nr2_npe_method = value;
    emit(nr2NpeMethodChanged(rx, value));
}

void Settings::setNRAgc(int rx, int value) {
    if (m_receiverDataList[rx].nr_agc == value) return;
    m_receiverDataList[rx].nr_agc = value;
    emit(nrAgcChanged(rx, value));
}


void Settings::setSnb(int rx, bool value) {
    if (m_receiverDataList[rx].snb == value) return;
    m_receiverDataList[rx].snb = value;
    emit(snbChanged(rx, value));
}


void Settings::setAnf(int rx, bool value) {
    if (m_receiverDataList[rx].anf == value) return;
    m_receiverDataList[rx].anf = value;
    emit(anfChanged(rx, value));
}


void Settings::getConfigPath() {
    cfg_dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).append("/.cudaSDR");
    qDebug() << "cfg dir " << cfg_dir;

    QDir dir(cfg_dir);
    if (!dir.exists()) {
        dir.mkpath(".'");
        qDebug() << "cfg dir does not exist - creating" << cfg_dir;

    }
    qDebug() << cfg_dir;
}





DSPMode Settings::getDSPMode(int rx){
    if (rx < 0 || rx >= m_receiverDataList.size())
        return LSB;

    HamBand band = m_receiverDataList[rx].hamBand;

    return m_receiverDataList[rx].dspModeList[band];
}




RadioState Settings::setRadioState(RadioState mode) {

    if (m_radioState != mode) {
         m_radioState = mode;
         qDebug() << " Radio state changed" << m_radioState;
         emit radioStateChanged(m_radioState);
    }
    return m_radioState;

}

void Settings::setRepeaterMode(bool mode){
    m_repeaterMode = mode;

   emit repeaterModeChanged(mode);
}

void Settings::setRepeaterOffset(int offset)
{
m_repeaterOffset = (double)offset;
 emit repeaterOffsetchanged(offset);
}

void Settings::setFMPreEmphasize(int value)
{

    m_fmPremphasize=(double)value;
    emit fmPremphasizechanged(value);

}

void Settings::setFmDeveation(int value)
{
    m_fmDeveation = (double)value;
    emit fmdeveationchanged(value);
}

void Settings::setAMCarrierLevel(int level)
{
    qDebug() << "set Am carrier level" << level;
    m_amCarrierLevel = (double) level;
 emit amCarrierlevelchanged(level);
}

void Settings::setAudioCompression(int level){

m_audioCompression = level;
emit audioCompressionchanged(level);
}

bool Settings::isInternalCw() const {
    return (m_internal_cw > 0);
}

void Settings::setInternalCw(int InternalCw) {
     m_internal_cw = InternalCw;
     emit(InternalCwChanged (InternalCw));
}

int Settings::getCwKeyerSpeed() const {
    return m_cw_keyer_speed;

}

int Settings::getCwKeyerMode() const {
    return m_cw_keyer_mode;
}

void Settings::setCwKeyerMode(int mCwKeyerMode) {
    m_cw_keyer_mode = mCwKeyerMode;
    emit(CwKeyerModeChanged(m_cw_keyer_mode));
}

int  Settings::isCwKeyReversed() const {
      return m_cw_key_reversed;
}

void Settings::setCwKeyReversed(int mCwKeyReversed) {
    m_cw_key_reversed = mCwKeyReversed;
    emit(CwKeyReversedChanged(mCwKeyReversed));
}


void Settings::setCwKeyerSpeed(int mCwKeyerSpeed) {
    m_cw_keyer_speed = mCwKeyerSpeed;
    emit(CwKeyerSpeedChanged(mCwKeyerSpeed));
}

int Settings::getCwSidetoneVolume() const {
    return m_cw_sidetone_volume;
}

void Settings::setCwSidetoneVolume(int mCwSidetoneVolume) {
    m_cw_sidetone_volume = mCwSidetoneVolume;
    emit(CwSidetoneVolumeChanged(mCwSidetoneVolume));
}


int Settings::getCwPttDelay() const {
    return m_cw_ptt_delay;
}

void Settings::setCwPttDelay(int mCwPttDelay) {
    m_cw_ptt_delay = mCwPttDelay;
    emit(CwPttDelayChanged(mCwPttDelay));
}

int Settings::getCwHangTime() const {
    return m_cw_hang_time;
}

void Settings::setCwHangTime(int mCwHangTime) {
    m_cw_hang_time = mCwHangTime;
    emit(CwHangTimeChanged(mCwHangTime));
}


int Settings::getCwSidetoneFreq() const {
    return m_cw_sidetone_freq;
}


int Settings::getCwKeyerWeight() const {
    return m_cw_keyer_weight;
}


int Settings::getCwKeyerSpacing() const {
    return m_cw_keyer_spacing;
}




void Settings::setCwSidetoneFreq(int mCwSidetoneFreq) {
    m_cw_sidetone_freq = mCwSidetoneFreq;
    emit(CwSidetoneFreqChanged(mCwSidetoneFreq));
}


void Settings::setCwKeyerWeight(int val){
    m_cw_keyer_weight= val;
    qDebug() << "CW weight" << val;
    emit(CwKeyerWeightChanged(val));

}


void Settings::setCwKeyerSpacing(int val) {
    m_cw_keyer_spacing = val;
    qDebug() << "CW spacing" << val;
    emit(CwKeyerSpacingChanged(val));
}
