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
#include "Models/RadioModel.h"
#include "Models/RadioTelemetry.h"
#include "Models/SliceModel.h"
#include "Models/TransmitModel.h"
#include "Util/settings_utils.h"

using namespace SettingsUtils;

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

    connect(m_displayConfig, &DisplayConfig::spectrumSizeChanged, this, &Settings::spectrumSizeChanged);
    connect(m_displayConfig, &DisplayConfig::sMeterHoldTimeChanged, this, &Settings::sMeterHoldTimeChanged);


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

    m_versionString = "v6.2.0 - ZL2BRG";

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
        m_receiverDataList[i].nr = 0;
        m_receiverDataList[i].nbMode = 0;
        m_receiverDataList[i].anf = false;
        m_receiverDataList[i].snb = false;
        m_receiverDataList[i].nr2_gain_method = 0;
        m_receiverDataList[i].nr2_npe_method = 0;
        m_receiverDataList[i].nr_agc = 0;
        m_receiverDataList[i].nr2_ae = false;

        for (int j = 0; j < MAX_BANDS; j++) {

            m_receiverDataList[i].mercuryAttenuators << 0;
            m_receiverDataList[i].dBmPanScaleMinList << 0.0;
            m_receiverDataList[i].dBmPanScaleMaxList << 0.0;
            m_receiverDataList[i].dspModeList << (DSPMode) LSB;
            m_receiverDataList[i].lastCenterFrequencyList << (qint64)7050000;
            m_receiverDataList[i].lastVfoFrequencyList << (qint64)7050000;
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
    m_minimumWidgetWidth = clampMinimumWidgetWidth(value);

    value = settings->value("window/minimumGroupBoxWidth", 250).toInt();
    m_minimumGroupBoxWidth = clampMinimumGroupBoxWidth(value, m_minimumWidgetWidth);

    value = settings->value("window/multiRxView", 0).toInt();
    m_multiRxView = clampMultiRxView(value);


    // network settings
    m_networkConfig->loadIni(settings);
    m_serverAddress = m_networkConfig->serverAddress();
    m_hpsdrDeviceLocalAddr = m_networkConfig->localAddress();
    m_serverPort = m_networkConfig->serverPort();
    m_listenerPort = m_networkConfig->listenPort();
    m_audioPort = m_networkConfig->audioPort();
    m_metisPort = m_networkConfig->metisPort();
    m_socketBufferSize = m_networkConfig->socketBufferSize();

    m_tciServerEnabled = settings->value("network/tci_enabled", true).toBool();
    m_tciRxGain = qBound(0.0f, settings->value("network/tci_rx_gain", 1.0).toFloat(), 2.0f);
    m_tciTxGain = qBound(0.0f, settings->value("network/tci_tx_gain", 1.0).toFloat(), 2.0f);

    m_lastConnectedDevice.deviceClass = static_cast<DeviceClass>(settings->value("network/lastDeviceClass", DeviceClass_None).toInt());
    m_lastConnectedDevice.deviceType = settings->value("network/lastDeviceType", "").toString();
    m_lastConnectedDevice.serialNumber = settings->value("network/lastDeviceSerial", "").toString();
    m_lastConnectedDevice.label = settings->value("network/lastDeviceLabel", "").toString();

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

#ifdef HAVE_SOAPYSDR
    // Always load SoapySDR settings regardless of active hardware mode.
    m_soapyRxAntenna   = settings->value("SoapySDR/rxAntenna", "").toString();
    m_soapyTxAntenna   = settings->value("SoapySDR/txAntenna", "").toString();
    m_soapyLnaGain     = settings->value("SoapySDR/lnaGain", 25).toInt();
    m_soapyTiaGain     = settings->value("SoapySDR/tiaGain", 12).toInt();
    m_soapyPgaGain     = settings->value("SoapySDR/pgaGain", 12).toInt();
    m_soapyOverallGain = settings->value("SoapySDR/overallGain", 60).toInt();
    m_soapyAutoCalibrate = settings->value("SoapySDR/autoCalibrate", false).toBool();
    m_soapyIQBalance     = settings->value("SoapySDR/iqBalance",      true).toBool();
    m_soapyHardwareKey = "";
    m_soapyAntennaList.clear();
    m_soapyTxAntennaList.clear();
    // Pluto / network Soapy devices need uri or hostname beyond driver+serial.
    {
        const QString uri = settings->value("SoapySDR/uri", "").toString();
        const QString hostname = settings->value("SoapySDR/hostname", "").toString();
        if (!uri.isEmpty())
            m_currentSoapyDevice.args.insert(QStringLiteral("uri"), uri);
        if (!hostname.isEmpty())
            m_currentSoapyDevice.args.insert(QStringLiteral("hostname"), hostname);
    }
#endif

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

    m_hardwareConfig->loadIni(settings);

    m_audioConfig->loadIni(settings);

    m_repeaterOffset =  settings->value("repeater_offset",0).toDouble();
    m_txFullDuplex = settings->value("radio/txFullDuplex", true).toBool();

    m_cwConfig->loadIni(settings);

    str = settings->value("server/class", 0).toString();
    m_RxClass = (str.toLower() == "E");
    if (m_RxClass)
        m_RxClass = 1;
    else
        m_RxClass = 0;

    value = settings->value("server/timing", 0).toInt();
    if (value < 0 || value > 1) value = 0;
    m_RxTiming = value;



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
    if ((value < 1) || (value > 1000)) value = 5;
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

        m_receiverConfigs[i]->loadIni(settings);
        m_receiverDataList[i].dspCore = m_receiverConfigs[i]->dspCore();
        if (m_receiverDataList[i].dspCore == QSDR::QtDSP) {
            setSpectrumSize(4096);
        }
        m_receiverDataList[i].ctrFrequency = m_receiverConfigs[i]->ctrFrequency();
        m_receiverDataList[i].vfoFrequency = m_receiverConfigs[i]->vfoFrequency();
        m_receiverDataList[i].vfoAFrequency = m_receiverConfigs[i]->vfoAFrequency();
        m_receiverDataList[i].vfoBFrequency = m_receiverConfigs[i]->vfoBFrequency();
        m_receiverDataList[i].activeVfo = m_receiverConfigs[i]->activeVfo();

        QString cstr = m_rxStringList.at(i);
        cstr.append("/nr");
        m_receiverDataList[i].nr = settings->value(cstr, 0).toInt();
        cstr = m_rxStringList.at(i);
        cstr.append("/nbMode");
        m_receiverDataList[i].nbMode = settings->value(cstr, 0).toInt();

        cstr = m_rxStringList.at(i);
        cstr.append("/anf");
        m_receiverDataList[i].anf = settings->value(cstr, false).toBool();

        cstr = m_rxStringList.at(i);
        cstr.append("/snb");
        m_receiverDataList[i].snb = settings->value(cstr, false).toBool();

        cstr = m_rxStringList.at(i);
        cstr.append("/nr2_gain_method");
        m_receiverDataList[i].nr2_gain_method = settings->value(cstr, 0).toInt();

        cstr = m_rxStringList.at(i);
        cstr.append("/nr2_npe_method");
        m_receiverDataList[i].nr2_npe_method = settings->value(cstr, 0).toInt();

        cstr = m_rxStringList.at(i);
        cstr.append("/nr_agc");
        value = settings->value(cstr, 0).toInt();
        m_receiverDataList[i].nr_agc = value;

        cstr = m_rxStringList.at(i);
        cstr.append("/nr2_ae");
        value = settings->value(cstr, false).toBool();
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
        cstr.append("/peakHold");
        str = settings->value(cstr, "off").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].peakHold = true;
        else
            m_receiverDataList[i].peakHold = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/hairCross");
        str = settings->value(cstr, "off").toString();
        if (str.toLower() == "on")
            m_receiverDataList[i].hairCross = true;
        else
            m_receiverDataList[i].hairCross = false;

        cstr = m_rxStringList.at(i);
        cstr.append("/cwDecode");
        str = settings->value(cstr, "off").toString();
        if (str.toLower() == "on" || str.toLower() == "true")
            m_receiverDataList[i].cwDecode = true;
        else
            m_receiverDataList[i].cwDecode = false;

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

        m_receiverDataList[i].lastCenterFrequencyList.clear();
        m_receiverDataList[i].lastVfoFrequencyList.clear();

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency2200m");
        lvalue = settings->value(cstr, 135700).toLongLong();
        if ((lvalue < 135700) || (lvalue > 137800)) lvalue = 135700;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency2200m");
        lvalue = settings->value(cstr, 135700).toLongLong();
        if ((lvalue < 135700) || (lvalue > 137800)) lvalue = 135700;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency630m");
        lvalue = settings->value(cstr, 472000).toLongLong();
        if ((lvalue < 472000) || (lvalue > 479000)) lvalue = 472000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency630m");
        lvalue = settings->value(cstr, 472000).toLongLong();
        if ((lvalue < 472000) || (lvalue > 479000)) lvalue = 472000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency160m");
        lvalue = settings->value(cstr, 1810000).toLongLong();
        if ((lvalue < 1810000) || (lvalue > 2000000)) lvalue = 1810000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency160m");
        lvalue = settings->value(cstr, 1800000).toLongLong();
        if ((lvalue < 1810000) || (lvalue > 2000000)) lvalue = 1810000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency80m");
        lvalue = settings->value(cstr, 3500000).toLongLong();
        if ((lvalue < 3500000) || (lvalue > 3800000)) lvalue = 3500000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency80m");
        lvalue = settings->value(cstr, 3500000).toLongLong();
        if ((lvalue < 3500000) || (lvalue > 3800000)) lvalue = 3500000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency60m");
        lvalue = settings->value(cstr, 5260000).toLongLong();
        if ((lvalue < 5260000) || (lvalue > 5410000)) lvalue = 5260000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency60m");
        lvalue = settings->value(cstr, 5260000).toLongLong();
        if ((lvalue < 5260000) || (lvalue > 5410000)) lvalue = 5260000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency40m");
        lvalue = settings->value(cstr, 7000000).toLongLong();
        if ((lvalue < 7000000) || (lvalue > 7200000)) lvalue = 7000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency40m");
        lvalue = settings->value(cstr, 7000000).toLongLong();
        if ((lvalue < 7000000) || (lvalue > 7200000)) lvalue = 7000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency30m");
        lvalue = settings->value(cstr, 10100000).toLongLong();
        if ((lvalue < 10100000) || (lvalue > 10150000)) lvalue = 10100000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency30m");
        lvalue = settings->value(cstr, 10100000).toLongLong();
        if ((lvalue < 10100000) || (lvalue > 10150000)) lvalue = 10100000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency20m");
        lvalue = settings->value(cstr, 14000000).toLongLong();
        if ((lvalue < 14000000) || (lvalue > 14350000)) lvalue = 14000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency20m");
        lvalue = settings->value(cstr, 14000000).toLongLong();
        if ((lvalue < 14000000) || (lvalue > 14350000)) lvalue = 14000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency17m");
        lvalue = settings->value(cstr, 18068000).toLongLong();
        if ((lvalue < 18068000) || (lvalue > 18168000)) lvalue = 18068000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency17m");
        lvalue = settings->value(cstr, 18068000).toLongLong();
        if ((lvalue < 18068000) || (lvalue > 18168000)) lvalue = 18068000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency15m");
        lvalue = settings->value(cstr, 21000000).toLongLong();
        if ((lvalue < 21000000) || (lvalue > 21450000)) lvalue = 21000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency15m");
        lvalue = settings->value(cstr, 21000000).toLongLong();
        if ((lvalue < 21000000) || (lvalue > 21450000)) lvalue = 21000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency12m");
        lvalue = settings->value(cstr, 24890000).toLongLong();
        if ((lvalue < 24890000) || (lvalue > 24990000)) lvalue = 24890000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency12m");
        lvalue = settings->value(cstr, 24890000).toLongLong();
        if ((lvalue < 24890000) || (lvalue > 24990000)) lvalue = 24890000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency10m");
        lvalue = settings->value(cstr, 28000000).toLongLong();
        if ((lvalue < 28000000) || (lvalue > 29700000)) lvalue = 28000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency10m");
        lvalue = settings->value(cstr, 28000000).toLongLong();
        if ((lvalue < 28000000) || (lvalue > 29700000)) lvalue = 28000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency6m");
        lvalue = settings->value(cstr, 50000000).toLongLong();
        if ((lvalue < 50000000) || (lvalue > 54000000)) lvalue = 50000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency6m");
        lvalue = settings->value(cstr, 50000000).toLongLong();
        if ((lvalue < 50000000) || (lvalue > 54000000)) lvalue = 50000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency2m");
        lvalue = settings->value(cstr, 144000000).toLongLong();
        if ((lvalue < 144000000) || (lvalue > 148000000)) lvalue = 144000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency2m");
        lvalue = settings->value(cstr, 144000000).toLongLong();
        if ((lvalue < 144000000) || (lvalue > 148000000)) lvalue = 144000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency125cm");
        lvalue = settings->value(cstr, 222000000).toLongLong();
        if ((lvalue < 222000000) || (lvalue > 225000000)) lvalue = 222000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency125cm");
        lvalue = settings->value(cstr, 222000000).toLongLong();
        if ((lvalue < 222000000) || (lvalue > 225000000)) lvalue = 222000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency70cm");
        lvalue = settings->value(cstr, 420000000).toLongLong();
        if ((lvalue < 420000000) || (lvalue > 450000000)) lvalue = 420000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency70cm");
        lvalue = settings->value(cstr, 420000000).toLongLong();
        if ((lvalue < 420000000) || (lvalue > 450000000)) lvalue = 420000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency33cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

//RRK TODO FIX
        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency33cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency23cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency23cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency13cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency13cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency10cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency10cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequency5cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequency5cm");
        lvalue = settings->value(cstr, 902000000).toLongLong();
        if ((lvalue < 902000000) || (lvalue > 928000000)) lvalue = 902000000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastCenterFrequencyGen");
        lvalue = settings->value(cstr, 1800000).toLongLong();
        if ((lvalue < 0) || (lvalue > 50000000)) lvalue = 3500000;
        m_receiverDataList[i].lastCenterFrequencyList << lvalue;

        cstr = m_rxStringList.at(i);
        cstr.append("/lastVfoFrequencyGen");
        lvalue = settings->value(cstr, 1800000).toLongLong();
        if ((lvalue < 0) || (lvalue > MAXFREQUENCY)) lvalue = 1800000;
        m_receiverDataList[i].lastVfoFrequencyList << lvalue;

        // Prefer keys under receiverN/; fall back to values already loaded via ReceiverConfig
        // (including legacy rxN/ migration). Do not hard-default to 3672000 — that wiped
        // restored frequencies after ReceiverConfig started owning persistence.
        cstr = m_rxStringList.at(i);
        cstr.append("/centerFrequency");
        lvalue = settings->value(cstr, m_receiverDataList[i].ctrFrequency).toLongLong();
        if ((lvalue < 0) || (lvalue > MAXFREQUENCY)) lvalue = m_receiverDataList[i].ctrFrequency;
        m_receiverDataList[i].ctrFrequency = lvalue;

        setCtrFrequency(i, lvalue);

        cstr = m_rxStringList.at(i);
        cstr.append("/vfoFrequency");
        lvalue = settings->value(cstr, m_receiverDataList[i].vfoFrequency).toLongLong();
        if ((lvalue < 0) || (lvalue > MAXFREQUENCY)) lvalue = m_receiverDataList[i].vfoFrequency;
        m_receiverDataList[i].vfoFrequency = lvalue;

        setVfoFrequency(i, lvalue);

        cstr = m_rxStringList.at(i);
        cstr.append("/vfoAFrequency");
        if (settings->contains(cstr)) {
            lvalue = settings->value(cstr).toLongLong();
            if ((lvalue < 0) || (lvalue > MAXFREQUENCY))
                lvalue = m_receiverDataList[i].vfoFrequency;
            m_receiverDataList[i].vfoAFrequency = lvalue;
        } else {
            m_receiverDataList[i].vfoAFrequency = m_receiverDataList[i].vfoFrequency;
        }

        cstr = m_rxStringList.at(i);
        cstr.append("/vfoBFrequency");
        if (settings->contains(cstr)) {
            lvalue = settings->value(cstr).toLongLong();
            if ((lvalue < 0) || (lvalue > MAXFREQUENCY))
                lvalue = m_receiverDataList[i].vfoFrequency;
            m_receiverDataList[i].vfoBFrequency = lvalue;
        } else {
            m_receiverDataList[i].vfoBFrequency = m_receiverDataList[i].vfoFrequency;
        }

        cstr = m_rxStringList.at(i);
        cstr.append("/activeVfo");
        {
            const int active = settings->value(cstr, 0).toInt();
            m_receiverDataList[i].activeVfo = (active == 1) ? 1 : 0;
            // Live dial follows the active VFO memory.
            if (m_receiverDataList[i].activeVfo == 1)
                m_receiverDataList[i].vfoFrequency = m_receiverDataList[i].vfoBFrequency;
            else
                m_receiverDataList[i].vfoFrequency = m_receiverDataList[i].vfoAFrequency;
        }


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
                else if (str == "WBFM" || str == "WFM")
                    m_receiverDataList[i].dspModeList[j] = FMN; // legacy; WBFM removed
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

        if (m_receiverDataList[i].dBmPanScaleMinList.length() == MAX_BANDS &&
            m_receiverDataList[i].dBmPanScaleMaxList.length() == MAX_BANDS &&
            m_bandList.length() == MAX_BANDS
                ) {
            for (int j = 0; j < MAX_BANDS; j++) {

                cstr = m_rxStringList.at(i);
                cstr.append("/dBmPanScaleMin");
                cstr.append(m_bandList.at(j).bandString);

                qreal loadedMin = settings->value(cstr, -120.0).toDouble();
                if ((loadedMin < MINDBM) || (loadedMin > MAXDBM)) loadedMin = -120.0;

                cstr = m_rxStringList.at(i);
                cstr.append("/dBmPanScaleMax");
                cstr.append(m_bandList.at(j).bandString);

                qreal loadedMax = settings->value(cstr, -10.0).toDouble();
                if ((loadedMax < MINDBM) || (loadedMax > MAXDBM)) loadedMax = -10.0;

                if (loadedMax <= loadedMin) {

                    loadedMin = -120.0;
                    loadedMax = -10.0;
                }

                m_receiverDataList[i].dBmPanScaleMinList[j] = loadedMin;
                m_receiverDataList[i].dBmPanScaleMaxList[j] = loadedMax;
            }
        }
    }

     //******************************************************************
    // graphics and color settings
    m_displayConfig->loadIni(settings);


    SETTINGS_DEBUG << "reading done.";

    return 0;
}

int Settings::saveSettings() {
    syncSettingsWithSlices();
    syncSettingsWithTransmit();

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

    m_networkConfig->saveIni(settings);
    settings->setValue("network/tci_enabled", m_tciServerEnabled);
    settings->setValue("network/tci_rx_gain", m_tciRxGain);
    settings->setValue("network/tci_tx_gain", m_tciTxGain);
    settings->setValue("network/lastDeviceClass", static_cast<int>(m_lastConnectedDevice.deviceClass));
    settings->setValue("network/lastDeviceType", m_lastConnectedDevice.deviceType);
    settings->setValue("network/lastDeviceSerial", m_lastConnectedDevice.serialNumber);
    settings->setValue("network/lastDeviceLabel", m_lastConnectedDevice.label);
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

        // Cyclops / SoapySDR
        case 2:
            break;
    }

#ifdef HAVE_SOAPYSDR
    // Always save SoapySDR settings regardless of active hardware mode,
    // so they persist when switching back to SoapySDR.
    settings->setValue("SoapySDR/label",       m_currentSoapyDevice.label);
    settings->setValue("SoapySDR/driver",      m_currentSoapyDevice.driver);
    settings->setValue("SoapySDR/serial",      m_currentSoapyDevice.serial);
    settings->setValue("SoapySDR/uri",         m_currentSoapyDevice.args.value(QStringLiteral("uri")));
    settings->setValue("SoapySDR/hostname",    m_currentSoapyDevice.args.value(QStringLiteral("hostname")));
    settings->setValue("SoapySDR/rxAntenna",   m_soapyRxAntenna);
    settings->setValue("SoapySDR/txAntenna",   m_soapyTxAntenna);
    settings->setValue("SoapySDR/lnaGain",     m_soapyLnaGain);
    settings->setValue("SoapySDR/tiaGain",     m_soapyTiaGain);
    settings->setValue("SoapySDR/pgaGain",     m_soapyPgaGain);
    settings->setValue("SoapySDR/overallGain", m_soapyOverallGain);
    settings->setValue("SoapySDR/autoCalibrate", m_soapyAutoCalibrate);
    settings->setValue("SoapySDR/iqBalance",   m_soapyIQBalance);
#endif

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

            // Cyclops / SoapySDR
        case 2:
#ifdef HAVE_SOAPYSDR
            if (m_hwInterface == QSDR::SoapySDR)
                settings->setValue("hpsdr/interface", "soapy");
#endif
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

    m_hardwareConfig->saveIni(settings);

    m_audioConfig->saveIni(settings);


    settings->setValue("server/class", m_RxClass);
    settings->setValue("server/timing", m_RxTiming);

    settings->setValue("repeater_offset",m_repeaterOffset);
    settings->setValue("radio/txFullDuplex", m_txFullDuplex);
    // CW settings

    m_cwConfig->saveIni(settings);




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
        str.append("/peakHold");
        if (m_receiverDataList[i].peakHold)
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
        str.append("/cwDecode");
        if (m_receiverDataList[i].cwDecode)
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

            settings->setValue(str,  m_receiverDataList[i].lastCenterFrequencyList.at(j));
        }

        // vfo frequencies
        for (int j = 0; j < MAX_BANDS; j++) {

            str = m_rxStringList.at(i);
            str.append("/lastVfoFrequency");
            str.append(m_bandList.at(j).bandString);

            settings->setValue(str,  m_receiverDataList[i].lastVfoFrequencyList.at(j));
        }

        m_receiverConfigs[i]->setDspCore(m_receiverDataList[i].dspCore);
        m_receiverConfigs[i]->setCtrFrequency(m_receiverDataList[i].ctrFrequency);
        m_receiverConfigs[i]->setVfoFrequency(m_receiverDataList[i].vfoFrequency);
        m_receiverConfigs[i]->setVfoAFrequency(m_receiverDataList[i].vfoAFrequency);
        m_receiverConfigs[i]->setVfoBFrequency(m_receiverDataList[i].vfoBFrequency);
        m_receiverConfigs[i]->setActiveVfo(m_receiverDataList[i].activeVfo);
        m_receiverConfigs[i]->saveIni(settings);

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

            settings->setValue(str,  m_receiverDataList[i].dBmPanScaleMinList[j]);

            str = m_rxStringList.at(i);
            str.append("/dBmPanScaleMax");
            str.append(m_bandList.at(j).bandString);

            settings->setValue(str,  m_receiverDataList[i].dBmPanScaleMaxList[j]);
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

    m_displayConfig->saveIni(settings);

    SETTINGS_DEBUG << "save settings done.";
    return 0;
}

void Settings::reopenSettingsStorage(const QString &absoluteIniPath)
{
    if (settings) {
        delete settings;
        settings = nullptr;
    }
    settingsFilename = QFileInfo(absoluteIniPath).fileName();
    settings = new QSettings(absoluteIniPath, QSettings::IniFormat);
}

//void Settings::setMainWindowsState() {
//
//	settings->setValue("geometry", .saveGeometry());
//	settings->setValue("windowState", saveState());
//}

//*******************************************************

QList<qint64> Settings::getCtrFrequencies() {

    QList<qint64> frequencies;

    for (int i = 0; i < MAX_RECEIVERS; i++) {
        if (m_radioModel && i < m_radioModel->slices().size() && m_radioModel->slices()[i])
            frequencies << m_radioModel->slices()[i]->centerFrequency();
        else if (i < m_receiverDataList.size())
            frequencies << m_receiverDataList[i].ctrFrequency;
        else
            frequencies << 7000000;
    }

    return frequencies;
}

QList<qint64> Settings::getVfoFrequencies() {

    QList<qint64> frequencies;

    for (int i = 0; i < MAX_RECEIVERS; i++) {
        if (m_radioModel && i < m_radioModel->slices().size() && m_radioModel->slices()[i])
            frequencies << m_radioModel->slices()[i]->frequency();
        else if (i < m_receiverDataList.size())
            frequencies << m_receiverDataList[i].vfoFrequency;
        else
            frequencies << 7000000;
    }

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

    saveSettings();
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
    if (rx >= 0 && m_radioModel && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setPanMode(panMode);
            slice->setWaterfallMode(waterfallColorMode);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].panMode = panMode;
            m_receiverDataList[rx].waterfallMode = waterfallColorMode;
            return;
        }
    }

    QMutexLocker locker(&settingsMutex);

    if (rx == -1) {
        m_widebandOptions.panMode = panMode;
    } else {
        m_receiverDataList[rx].panMode = panMode;
        m_receiverDataList[rx].waterfallMode = waterfallColorMode;
    }

    emit graphicModeChanged(rx, panMode, waterfallColorMode);
}

PanGraphicsMode Settings::getPanadapterMode(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->panMode();

    return m_receiverDataList[rx].panMode;
}

PanAveragingMode Settings::getPanAveragingMode(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->panAveragingMode();

    return m_receiverDataList[rx].panAvMode;
}


PanDetectorMode Settings::getPanDetectorMode(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->panDetectorMode();

    return m_receiverDataList[rx].panDetMode;
}


WaterfallColorMode Settings::getWaterfallColorMode(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->waterfallMode();

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

void Settings::setRxList(QList<SliceProcessor *> rxList) {

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
    emit clearDiscoveredDevicesSignal();
    emit searchMetisSignal();
#ifdef HAVE_SOAPYSDR
    emit searchSoapySignal();
#endif
}

void Settings::clearMetisCardList() {

    m_metisCards.clear();

    //emit metisCardListChanged(m_metisCards);
}

void Settings::setMaxFrequency(qint64 value) {

    if (m_maxFrequency == value) return;
    m_maxFrequency = value;
    emit maxFrequencyChanged(m_maxFrequency);
}

void Settings::setMinFrequency(qint64 value) {

    if (m_minFrequency == value) return;
    m_minFrequency = value;
    emit minFrequencyChanged(m_minFrequency);
}

void Settings::setCurrentHPSDRDevice(TNetworkDevicecard card) {

    m_currentHPSDRDevice = card;
    m_lastConnectedDevice.deviceClass = DeviceClass_HPSDR;
    m_lastConnectedDevice.deviceType = card.boardName;
    m_lastConnectedDevice.serialNumber = QString::fromLatin1(card.mac_address);
    m_lastConnectedDevice.label = card.boardName;

    if (card.frequency_max > 0) {
        m_maxFrequency = static_cast<qint64>(card.frequency_max);
        m_minFrequency = static_cast<qint64>(card.frequency_min);
        emit maxFrequencyChanged(m_maxFrequency);
    }

    emit hpsdrNetworkDeviceChanged(m_currentHPSDRDevice);
    saveSettings();
}

#ifdef HAVE_SOAPYSDR
void Settings::setSoapyDeviceList(QList<TSoapyDevice> list) {
    m_soapyDevices = list;
    emit soapyDeviceListChanged(m_soapyDevices);
}

void Settings::setCurrentSoapyDevice(TSoapyDevice device) {
    m_currentSoapyDevice = device;
    m_lastConnectedDevice.deviceClass = DeviceClass_SoapySDR;
    m_lastConnectedDevice.deviceType = device.driver;
    m_lastConnectedDevice.serialNumber = device.serial;
    m_lastConnectedDevice.label = device.label;
    emit soapyDeviceChanged(m_currentSoapyDevice);
    saveSettings();
}

void Settings::setSoapyMessage(QString message) {
    emit soapyMessageEvent(message);
}

void Settings::setSoapyAntennaList(const QStringList &list) {
    m_soapyAntennaList = list;
    emit soapyAntennaListChanged(list);
}

void Settings::setSoapyTxAntennaList(const QStringList &list) {
    m_soapyTxAntennaList = list;
    emit soapyTxAntennaListChanged(list);
}

void Settings::setSoapyHardwareKey(const QString &key) {
    m_soapyHardwareKey = key;
    emit soapyHardwareKeyChanged(key);
}

void Settings::setSoapyRxAntenna(const QString &antenna) {
    if (m_soapyRxAntenna != antenna) {
        m_soapyRxAntenna = antenna;
        emit soapyRxAntennaChanged(antenna);
    }
}

void Settings::setSoapyTxAntenna(const QString &antenna) {
    if (m_soapyTxAntenna != antenna) {
        m_soapyTxAntenna = antenna;
        emit soapyTxAntennaChanged(antenna);
    }
}

void Settings::setSoapyLnaGain(int gain) {
    if (m_soapyLnaGain != gain) {
        m_soapyLnaGain = gain;
        emit soapyLnaGainChanged(gain);
    }
}

void Settings::setSoapyTiaGain(int gain) {
    if (m_soapyTiaGain != gain) {
        m_soapyTiaGain = gain;
        emit soapyTiaGainChanged(gain);
    }
}

void Settings::setSoapyPgaGain(int gain) {
    if (m_soapyPgaGain != gain) {
        m_soapyPgaGain = gain;
        emit soapyPgaGainChanged(gain);
    }
}

void Settings::setSoapyOverallGain(int gain) {
    if (m_soapyOverallGain != gain) {
        m_soapyOverallGain = gain;
        emit soapyOverallGainChanged(gain);
    }
}

void Settings::setSoapyOverallGainRange(int minGain, int maxGain) {
    if (maxGain < minGain)
        qSwap(minGain, maxGain);
    if (m_soapyOverallGainMin == minGain && m_soapyOverallGainMax == maxGain)
        return;
    m_soapyOverallGainMin = minGain;
    m_soapyOverallGainMax = maxGain;
    if (m_soapyOverallGain < minGain || m_soapyOverallGain > maxGain) {
        m_soapyOverallGain = qBound(minGain, m_soapyOverallGain, maxGain);
        emit soapyOverallGainChanged(m_soapyOverallGain);
    }
    emit soapyOverallGainRangeChanged(minGain, maxGain);
}

void Settings::setSoapyAutoCalibrate(bool enabled) {
    if (m_soapyAutoCalibrate != enabled) {
        m_soapyAutoCalibrate = enabled;
        emit soapyAutoCalibrateChanged(enabled);
        saveSettings();
    }
}

void Settings::setSoapyIQBalance(bool enabled) {
    if (m_soapyIQBalance != enabled) {
        m_soapyIQBalance = enabled;
        emit soapyIQBalanceChanged(enabled);
    }
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

void Settings::setTciServerEnabled(bool enabled) {

    if (m_tciServerEnabled == enabled) return;
    m_tciServerEnabled = enabled;
    settings->setValue("network/tci_enabled", m_tciServerEnabled);
    emit tciServerEnabledChanged(m_tciServerEnabled);
}

void Settings::setTciRxGain(float gain) {
    const float clamped = qBound(0.0f, gain, 2.0f);
    if (qAbs(m_tciRxGain - clamped) < 1e-6f)
        return;
    m_tciRxGain = clamped;
    settings->setValue("network/tci_rx_gain", m_tciRxGain);
    emit tciRxGainChanged(m_tciRxGain);
}

void Settings::setTciTxGain(float gain) {
    const float clamped = qBound(0.0f, gain, 2.0f);
    if (qAbs(m_tciTxGain - clamped) < 1e-6f)
        return;
    m_tciTxGain = clamped;
    settings->setValue("network/tci_tx_gain", m_tciTxGain);
    emit tciTxGainChanged(m_tciTxGain);
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
    if (rx < 0 || rx >= m_receiverDataList.size())
        return {};
    return m_receiverDataList.at(rx).mercuryAttenuators;
}

QSDR::_DSPCore Settings::getReceiverDspCore(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return QSDR::QtDSP;
    return m_receiverDataList.at(rx).dspCore;
}

SliceModel* Settings::sliceModel(int rx) const {

    if (!m_radioModel || rx < 0 || rx >= m_radioModel->slices().size())
        return nullptr;
    return m_radioModel->slices().at(rx);
}

QList<DSPMode> Settings::getDSPModeList(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return {};
    return m_receiverDataList.at(rx).dspModeList;
}

QList<qint64> Settings::getLastCenterFrequencyList(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return {};
    return m_receiverDataList.at(rx).lastCenterFrequencyList;
}

QList<qint64> Settings::getLastVfoFrequencyList(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return {};
    return m_receiverDataList.at(rx).lastVfoFrequencyList;
}

qreal Settings::getdBmPanScaleMin(int rx, HamBand band) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return -140.0;
    if (SliceModel* slice = sliceModel(rx)) {
        if (band == m_receiverDataList.at(rx).hamBand)
            return slice->dBmPanScaleMin();
    }
    if (band < 0 || band >= m_receiverDataList.at(rx).dBmPanScaleMinList.size())
        return -140.0;
    return m_receiverDataList.at(rx).dBmPanScaleMinList.at(band);
}

qreal Settings::getdBmPanScaleMax(int rx, HamBand band) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return -20.0;
    if (SliceModel* slice = sliceModel(rx)) {
        if (band == m_receiverDataList.at(rx).hamBand)
            return slice->dBmPanScaleMax();
    }
    if (band < 0 || band >= m_receiverDataList.at(rx).dBmPanScaleMaxList.size())
        return -20.0;
    return m_receiverDataList.at(rx).dBmPanScaleMaxList.at(band);
}

float Settings::getFreqRulerPosition(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return 0.5f;
    return m_receiverDataList.at(rx).freqRulerPosition;
}

qreal Settings::getFilterLo(int rx) const {

    if (SliceModel* slice = sliceModel(rx))
        return slice->filterLow();
    if (rx < 0 || rx >= m_receiverDataList.size())
        return -3050.0;
    return m_receiverDataList.at(rx).filterLo;
}

qreal Settings::getFilterHi(int rx) const {

    if (SliceModel* slice = sliceModel(rx))
        return slice->filterHigh();
    if (rx < 0 || rx >= m_receiverDataList.size())
        return -150.0;
    return m_receiverDataList.at(rx).filterHi;
}

TDefaultFilterMode Settings::getDefaultFilterMode(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return (TDefaultFilterMode)0;
    return m_receiverDataList.at(rx).defaultFilterMode;
}

qreal Settings::getAGCAttackTime(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return 0.002;
    return m_receiverDataList.at(rx).agcAttackTime;
}

qreal Settings::getAGCDecayTime(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return 0.25;
    return m_receiverDataList.at(rx).agcDecayTime;
}

qreal Settings::getAGCHangTime(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return 0.0;
    return m_receiverDataList.at(rx).agcHangTime;
}

bool Settings::getHangEnabled(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return false;
    return m_receiverDataList.at(rx).hangEnabled;
}

bool Settings::getAgcLines(int rx) const {

    if (rx < 0 || rx >= m_receiverDataList.size())
        return false;
    return m_receiverDataList.at(rx).agcLines;
}

int Settings::getWaterfallOffsetLo(int rx) const {

    if (SliceModel* slice = sliceModel(rx))
        return slice->waterfallOffsetLo();
    if (rx < 0 || rx >= m_receiverDataList.size())
        return -120;
    return m_receiverDataList.at(rx).waterfallOffsetLo;
}

int Settings::getWaterfallOffsetHi(int rx) const {

    if (SliceModel* slice = sliceModel(rx))
        return slice->waterfallOffsetHi();
    if (rx < 0 || rx >= m_receiverDataList.size())
        return -60;
    return m_receiverDataList.at(rx).waterfallOffsetHi;
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
    m_hardwareConfig->setSource10Mhz(source);
    emit src10MhzChanged(source);
}

void Settings::set122_88MhzSource(int source) {
    m_hardwareConfig->setSource122_88Mhz(source);
    emit src122_88MhzChanged(source);
}

void Settings::setMicSource(int source) {
    m_audioConfig->setMicSource(source);
    emit micSourceChanged(source);
}

void Settings::setMicInputDev(int index) {
    m_audioConfig->setMicInputDev(index);
    emit micInputChanged(index);
}

void Settings::setMicInputSourceName(const QString &name) {
    m_audioConfig->setMicInputSourceName(name);
}

void Settings::setDigitalInputSourceName(const QString &name) {
    m_audioConfig->setDigitalInputSourceName(name);
}

void Settings::setDigitalAudioInputDev(int index) {
    m_audioConfig->setDigitalAudioInputDev(index);
    emit digitalAudioInputChanged(index);
}

void Settings::setMicInputLevel(int level) {
    m_audioConfig->setMicGain(level);
    emit micInputLevelChanged(level);
}

void Settings::setDriveLevel(int level) {
    m_audioConfig->setDriveLevel(level);
    emit driveLevelChanged(clampDriveLevel(level));
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
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return (qreal)m_radioModel->slices()[rx]->volume();

    return m_receiverDataList[rx].audioVolume;
}

void Settings::setMainVolume(int rx, float volume) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setVolume(volume); return; }

    if (volume < 0) volume = 0.0f;
    if (volume > 1) volume = 1.0f;

    QMutexLocker locker(&settingsMutex);

    //if (m_receiverDataList[rx].audioVolume == volume) return;
    m_receiverDataList[rx].audioVolume = volume;

    emit mainVolumeChanged(rx, volume);
}

void Settings::setMainVolumeMute(int rx, bool value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setMute(value);
            return;
        }
    }

    qreal vol = getMainVolume(rx);
    if (value)
        setMainVolume(rx, 0.0f);
    else
        setMainVolume(rx, vol);
}

void Settings::setCtrFrequency(int rx, qint64 frequency) {

    QMutexLocker locker(&settingsMutex);

    HamBand band = getBandFromFrequency(m_bandList, frequency);

    m_receiverDataList[rx].ctrFrequency = frequency;
    //m_receiverDataList[rx].hamBand = band;
    //m_receiverDataList[rx].lastHamBand = band;
    m_receiverDataList[rx].lastCenterFrequencyList[(int) band] = frequency;
}

void Settings::setVfoFrequency(int rx, qint64 frequency) {

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

void Settings::setCtrFrequency(int mode, int rx, qint64 frequency) {
    if (rx < 0 || rx >= m_receiverDataList.size())
        return;

    // Mirror into MVC slice model; pre-MVC legacy path continues below and remains authoritative
    if (m_radioModel && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx])
        m_radioModel->slices()[rx]->setCenterFrequency(frequency);

    QMutexLocker locker(&settingsMutex);
    m_receiverDataList[rx].ctrFrequency = frequency;

    HamBand band = getBandFromFrequency(m_bandList, frequency);
    m_receiverDataList[rx].lastCenterFrequencyList[(int) band] = frequency;
    locker.unlock();

    switch (mode) {

        case 0:
            break;

        case 1:
            // Keep VFO on the LO (NCO 0). setVFOFrequency early-returns when
            // VFO already equals frequency, which would leave a stale NCO if
            // CTR was corrected after a mode-0 band hop — clear it explicitly.
            setVFOFrequency(0, rx, frequency);
            if (m_receiverDataList.at(rx).ncoFrequency != 0)
                setNCOFrequency(false, rx, 0);
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

qint64 Settings::getCtrFrequency(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->centerFrequency();

    return m_receiverDataList.at(rx).ctrFrequency;
}

void Settings::setVFOFrequency(int mode, int rx, qint64 frequency) {
    if (rx < 0 || rx >= m_receiverDataList.size())
        return;

    // Mirror into MVC slice model; pre-MVC legacy path continues below and remains authoritative
    if (m_radioModel && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
        SliceModel *slice = m_radioModel->slices()[rx];
        slice->setFrequency(frequency);
        m_receiverDataList[rx].vfoAFrequency = slice->vfoAFrequency();
        m_receiverDataList[rx].vfoBFrequency = slice->vfoBFrequency();
        m_receiverDataList[rx].activeVfo =
            (slice->activeVfo() == SliceModel::VfoB) ? 1 : 0;
    }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).vfoFrequency == frequency) return;
    m_receiverDataList[rx].vfoFrequency = frequency;
    SETTINGS_DEBUG << "vfo freq (Rx " << rx << ") " << m_receiverDataList[rx].vfoFrequency;
    HamBand band = getBandFromFrequency(m_bandList, frequency);
    m_receiverDataList[rx].lastVfoFrequencyList[(int) band] = frequency;

    locker.unlock();
    if (m_receiverDataList.at(rx).hamBand != band) {

        // Capture the live operating mode before switching bands. Per-band
        // dspModeList entries can be stale (e.g. AM on 15m while DIGU is live);
        // WSJT-X band hops must not silently report/apply the wrong mode.
        DSPMode liveMode = m_receiverDataList[rx].dspModeList[m_receiverDataList[rx].hamBand];
        if (m_radioModel && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx])
            liveMode = m_radioModel->slices()[rx]->dspMode();

        setHamBand(rx, false, band);
        setDSPMode(rx, liveMode);
    }

    switch (mode) {

        case 0: // change only VFO

#ifdef HAVE_SOAPYSDR
            if (m_hwInterface == QSDR::SoapySDR) {
                // SoapySDR hardware tunes directly to VFO; keep center = VFO so NCO stays 0.
                // Update SliceModel as well: the panadapter follows
                // SliceModel::centerFrequencyChanged, not Settings::ctrFrequencyChanged.
                // Without that, click-VFO retunes the LO while the frequency ruler stays
                // on the old centre and signals appear shifted by the click offset.
                const bool ctrChanged = (m_receiverDataList[rx].ctrFrequency != frequency);
                m_receiverDataList[rx].ctrFrequency = frequency;
                m_receiverDataList[rx].ncoFrequency = 0;
                m_receiverDataList[rx].lastCenterFrequencyList[(int) band] = frequency;
                if (m_radioModel && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx])
                    m_radioModel->slices()[rx]->setCenterFrequency(frequency);
                if (ctrChanged)
                    emit ctrFrequencyChanged(0, rx, frequency);
            } else
#endif
            {
                m_receiverDataList[rx].ncoFrequency = frequency - m_receiverDataList.at(rx).ctrFrequency;
                SETTINGS_DEBUG << "nco freq = " << m_receiverDataList[rx].ncoFrequency << "rx frequency = " << frequency << "Ctr Frequnecy =" << m_receiverDataList.at(rx).ctrFrequency;
            }
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

void Settings::setVfoFrequencyVisible(int rx, qint64 frequency) {
    if (rx < 0 || rx >= m_receiverDataList.size())
        return;

    // The panadapter only covers ctr +/- sampleRate/2 and the panel clamps the VFO
    // cursor to that window, so a jump beyond it would hide the RX filter. Stay on
    // the current centre while the target is comfortably inside the span.
    const qint64 halfSpan = getSampleRate() / 2;
    const qint64 margin = halfSpan - halfSpan / 10;
    if (halfSpan > 0 && qAbs(frequency - getCtrFrequency(rx)) <= margin) {
        setVFOFrequency(0, rx, frequency);
        return;
    }

    // Out of span: move the LO with the dial (NCO back to zero), as a band change does.
    setCtrFrequency(1, rx, frequency);
}

qint64 Settings::getVfoFrequency(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->frequency();

    return m_receiverDataList.at(rx).vfoFrequency;
}

void Settings::setNCOFrequency(bool value, int rx, qint64 frequency) {

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
    // Mirror into MVC slice model; pre-MVC legacy path continues below and remains authoritative
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
        auto slice = m_radioModel->slices()[rx];
        slice->setDspMode(mode);
        const DSPMode wdspMode = resolveWDSPMode(mode, slice->centerFrequency());
        auto filter = getFilterFromDSPMode(m_defaultFilterList, wdspMode);
        slice->setFilterLow((float)filter.filterLo);
        slice->setFilterHigh((float)filter.filterHi);
    }

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
		case 100: return "RADE v1 (Neural Codec)";
		default: return "1600 bps (FREEDV_MODE_1600)";
	}
}

QList<int> Settings::availableCodec2Modes() {
	// Return list of available Codec2 modes
	return {0, 1, 2, 3, 4, 5, 6, 8, 16, 100};
}

AGCMode Settings::getAGCMode(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->agcMode();

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
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setAgcMode(mode);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].agcMode = mode;
            m_receiverDataList[rx].hangEnabled = agcHangEnabledForMode(mode);
            locker.unlock();
            return;
        }
    }

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
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return (qreal)m_radioModel->slices()[rx]->agcGain();

    return m_receiverDataList[rx].acgGain;
}

void Settings::setAGCGain(int rx, int value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setAgcGain(value);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].acgGain = value;
            return;
        }
    }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].acgGain == value) return;
    m_receiverDataList[rx].acgGain = value;
    emit agcGainChanged(rx, value);
}

void Settings::setAGCMaximumGain_dB(int rx, qreal value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setAgcMaxGain(static_cast<int>(value));
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].agcMaximumGain_dB = static_cast<int>(value);
            return;
        }
    }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcMaximumGain_dB == value) return;
    m_receiverDataList[rx].agcMaximumGain_dB = value;

    SETTINGS_DEBUG << "set agcMaximumGain_dB = " << m_receiverDataList[rx].agcMaximumGain_dB;
    emit agcMaximumGainChanged_dB(rx, value);
}

int Settings::getAGCMaximumGain_dB(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx])
        return m_radioModel->slices()[rx]->agcMaxGain();

    return m_receiverDataList[rx].agcMaximumGain_dB;
}

void Settings::setAGCFixedGain_dB(int rx, qreal value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setAgcFixedGain(static_cast<int>(value));
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].agcFixedGain_dB = value;
            return;
        }
    }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcFixedGain_dB == value) return;
    m_receiverDataList[rx].agcFixedGain_dB = value;

    SETTINGS_DEBUG << "m_receiverDataList[rx].agcFixedGain_dB = " << m_receiverDataList[rx].agcFixedGain_dB;
    emit agcFixedGainChanged_dB(rx, value);
}

qreal Settings::getAGCFixedGain_dB(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx])
        return static_cast<qreal>(m_radioModel->slices()[rx]->agcFixedGain());

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
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setAgcHangThreshold(value);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].agcHangThreshold = value;
            return;
        }
    }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].agcHangThreshold == value) return;
    m_receiverDataList[rx].agcHangThreshold = value;

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
    if (rx < 0 || rx >= m_receiverDataList.size())
        return;

    // Mirror into MVC slice model; legacy path continues below and remains authoritative
    if (m_radioModel && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
        auto slice = m_radioModel->slices()[rx];
        slice->setFilterLow(static_cast<float>(low));
        slice->setFilterHigh(static_cast<float>(high));
    }

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

    quint16 newAlexConfig = 0;
    {
        QMutexLocker locker(&settingsMutex);
        if (value)
            m_alexConfig |= 0x01;
        else
            m_alexConfig &= 0xFFFE;
        newAlexConfig = m_alexConfig;
    }

    // Emit after releasing settingsMutex to avoid re-entrant deadlocks/crashes
    // when connected slots call back into Settings mutators.
    emit alexManualStateChanged(value);
    emit alexConfigurationChanged(newAlexConfig);
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
    m_displayConfig->setSpectrumSize(value);
}

void Settings::setSpectrumBuffer(int rx, const qVectorFloat& buffer)
{
    emit spectrumBufferChanged(rx, buffer);
    if (m_radioModel && m_radioModel->telemetry()) {
        m_radioModel->telemetry()->setSpectrumBuffer(rx, buffer);
    }
}

void Settings::moveDisplayWidget(int value) {

    emit displayWidgetHeightChanged(value);
}


//*********************************
// color stuff

TPanadapterColors Settings::getPanadapterColors() {
    if (m_radioModel)
        return m_radioModel->panadapterColors();
    return m_displayConfig->panadapterColors();
}

void Settings::setPanadapterColors(TPanadapterColors type) {
    m_displayConfig->setPanadapterColors(type);
    if (m_radioModel) {
        m_radioModel->setPanadapterColors(type);
    }
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
    if (rx >= 0 && m_radioModel && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setSpectrumAveraging(value);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].spectrumAveraging = value;
            return;
        }
    }

    if (rx == -1) {
        m_widebandOptions.averaging = value;
    } else {
        m_receiverDataList[rx].spectrumAveraging = value;
    }

    SETTINGS_DEBUG << "Averaging for Rx " << rx << " : " << value;
    emit spectrumAveragingChanged(rx, value);
}

bool Settings::getSpectrumAveraging(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->spectrumAveraging();

    if (rx == -1)
        return m_widebandOptions.averaging;
    else
        return m_receiverDataList[rx].spectrumAveraging;
}

int Settings::getSpectrumAveragingCnt(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->spectrumAveragingCnt();

    if (rx == -1)
        return m_widebandOptions.averagingCnt;
    else
        return m_receiverDataList[rx].averagingCnt;
}

void Settings::setSpectrumAveragingCnt(int rx, int value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setSpectrumAveragingCnt(value); return; }

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
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setPanGrid(value); return; }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).panGrid == value) return;
    m_receiverDataList[rx].panGrid = value;

    emit panGridStatusChanged(m_receiverDataList.at(rx).panGrid, rx);
}

bool Settings::getPanGridStatus(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->panGrid();

    return m_receiverDataList[rx].panGrid;
}

void Settings::setPeakHold(bool value, int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
        m_radioModel->slices()[rx]->setPeakHold(value);
        QMutexLocker locker(&settingsMutex);
        m_receiverDataList[rx].peakHold = value;
        return;
    }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList.at(rx).peakHold == value) return;
    m_receiverDataList[rx].peakHold = value;

    emit peakHoldStatusChanged(m_receiverDataList.at(rx).peakHold, rx);
}

bool Settings::getPeakHoldStatus(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->peakHold();

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
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setWaterfallOffsetLo(value); return; }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].waterfallOffsetLo == value) return;
    m_receiverDataList[rx].waterfallOffsetLo = value;

    emit waterfallOffesetLoChanged(rx, value);
}

void Settings::setWaterfallOffesetHi(int rx, int value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setWaterfallOffsetHi(value); return; }

    QMutexLocker locker(&settingsMutex);

    if (m_receiverDataList[rx].waterfallOffsetHi == value) return;
    m_receiverDataList[rx].waterfallOffsetHi = value;

    emit waterfallOffesetHiChanged(rx, value);
}

void Settings::setSMeterHoldTime(int value) {
    if (m_radioModel) { for (auto slice : m_radioModel->slices()) if (slice) slice->setSMeterHoldTime(value); }
    m_displayConfig->setSMeterHoldTime(value);
}

void Settings::setdBmPanScaleMin(int rx, qreal value) {
    if (rx < 0 || rx >= m_receiverDataList.size())
        return;

    if (m_radioModel && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx])
        m_radioModel->slices()[rx]->setDBmPanScaleMin(value);

    QMutexLocker locker(&settingsMutex);

    HamBand band = m_receiverDataList.at(rx).hamBand;
    if (band < 0 || band >= m_receiverDataList[rx].dBmPanScaleMinList.size())
        return;
    m_receiverDataList[rx].dBmPanScaleMinList[band] = value;

    locker.unlock();
    emit dBmScaleMinChanged(rx, value);
}

void Settings::setdBmPanScaleMax(int rx, qreal value) {
    if (rx < 0 || rx >= m_receiverDataList.size())
        return;

    if (m_radioModel && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx])
        m_radioModel->slices()[rx]->setDBmPanScaleMax(value);

    QMutexLocker locker(&settingsMutex);

    HamBand band = m_receiverDataList.at(rx).hamBand;
    if (band < 0 || band >= m_receiverDataList[rx].dBmPanScaleMaxList.size())
        return;
    m_receiverDataList[rx].dBmPanScaleMaxList[band] = value;

    locker.unlock();
    emit dBmScaleMaxChanged(rx, value);
}

void Settings::setdBmDistScaleMin(qreal value) {
    m_displayConfig->setdBmDistScaleMin(value);
}

void Settings::setdBmDistScaleMax(qreal value) {
    m_displayConfig->setdBmDistScaleMax(value);
}

// **********************************************************************

void Settings::showRadioPopupWidget() {

    if (m_radioPopupVisible)
        m_radioPopupVisible = false;
    else
        m_radioPopupVisible = true;


}

void Settings::setPanAveragingMode(int rx, PanAveragingMode mode) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
        m_radioModel->slices()[rx]->setPanAveragingMode(mode);
        QMutexLocker locker(&settingsMutex);
        m_receiverDataList[rx].panAvMode = mode;
        return;
    }

    if (m_receiverDataList.at(rx).panAvMode == mode) return;

    m_receiverDataList[rx].panAvMode = mode;

    qDebug() << "Pan average mode set to " << mode;

    emit panAveragingModeChanged(rx, mode);

}

void Settings::setPanDetectorMode(int rx, PanDetectorMode mode) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setPanDetectorMode(mode);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].panDetMode = mode;
            return;
        }
    }

    if (m_receiverDataList.at(rx).panDetMode == mode) return;

    m_receiverDataList[rx].panDetMode = mode;

    qDebug() << "Pan detector mode set to " << m_receiverDataList[rx].panDetMode;

    emit panDetectorModeChanged(rx, mode);
};


int Settings::getAGCSlope(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->agcSlope();
    return m_receiverDataList[rx].agcSlope;

}

void Settings::setfftSize(int rx, int size) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setFftSize(size); return; }
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
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->fftSize();
    return m_receiverDataList[rx].fftsize;
}

int Settings::getNrAGC(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->nrAgc();
    return m_receiverDataList[rx].nr_agc;
}


int Settings::getNr2GainMethod(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->nr2GainMethod();
    return m_receiverDataList[rx].nr2_gain_method;
}

int Settings::getNr2NpeMethod(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->nr2NpeMethod();
    return m_receiverDataList[rx].nr2_npe_method;
}

bool Settings::getSnb(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->snb();
    return m_receiverDataList[rx].snb;
}

bool Settings::getAnf(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->anf();
    return m_receiverDataList[rx].anf;
}

bool Settings::getNr2ae(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->nr2Ae();
    return m_receiverDataList[rx].nr2_ae;
}


int Settings::getnbMode(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->nbMode();
    return m_receiverDataList[rx].nbMode;
}

int Settings::getnrMode(int rx) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) return m_radioModel->slices()[rx]->nrMode();
    return m_receiverDataList[rx].nr;
}


void Settings::setNoiseBlankerMode(int rx, int nb) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setNbMode(nb);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].nbMode = nb;
            return;
        }
    }
    if (m_receiverDataList[rx].nbMode == nb) return;
    m_receiverDataList[rx].nbMode = nb;
    emit noiseBlankerChanged(rx, nb);
}


void Settings::setNoiseFilterMode(int rx, int nr) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setNrMode(nr);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].nr = nr;
            return;
        }
    }
    if (m_receiverDataList[rx].nr == nr) return;
    m_receiverDataList[rx].nr = nr;
    emit noiseFilterChanged(rx, nr);
}

void Settings::setNR2Ae(int rx, bool value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setNr2Ae(value);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].nr2_ae = value;
            return;
        }
    }
    if (m_receiverDataList[rx].nr2_ae == value) return;
    m_receiverDataList[rx].nr2_ae = value;
    emit nr2AeChanged(rx, value);
}

void Settings::setNR2GainMethod(int rx, int value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setNr2GainMethod(value);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].nr2_gain_method = value;
            return;
        }
    }
    if (m_receiverDataList[rx].nr2_gain_method == value) return;
    m_receiverDataList[rx].nr2_gain_method = value;
    emit nr2GainMethodChanged(rx, value);
}

void Settings::setNR2NpeMethod(int rx, int value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setNr2NpeMethod(value);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].nr2_npe_method = value;
            return;
        }
    }
    if (m_receiverDataList[rx].nr2_npe_method == value) return;
    m_receiverDataList[rx].nr2_npe_method = value;
    emit nr2NpeMethodChanged(rx, value);
}

void Settings::setNRAgc(int rx, int value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size()) {
        SliceModel* slice = m_radioModel->slices().at(rx);
        if (slice) {
            slice->setNrAgc(value);
            QMutexLocker locker(&settingsMutex);
            m_receiverDataList[rx].nr_agc = value;
            return;
        }
    }
    if (m_receiverDataList[rx].nr_agc == value) return;
    m_receiverDataList[rx].nr_agc = value;
    emit nrAgcChanged(rx, value);
}


void Settings::setSnb(int rx, bool value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setSnb(value); return; }
    if (m_receiverDataList[rx].snb == value) return;
    m_receiverDataList[rx].snb = value;
    emit(snbChanged(rx, value));
}


void Settings::setAnf(int rx, bool value) {
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) { m_radioModel->slices()[rx]->setAnf(value); return; }
    if (m_receiverDataList[rx].anf == value) return;
    m_receiverDataList[rx].anf = value;
    emit(anfChanged(rx, value));
}

bool Settings::getCwDecode(int rx) {
    if (SliceModel* slice = sliceModel(rx))
        return slice->cwDecodeEnabled();
    if (rx < 0 || rx >= m_receiverDataList.size())
        return false;
    return m_receiverDataList[rx].cwDecode;
}

void Settings::setCwDecode(int rx, bool value) {
    if (rx >= 0 && rx < m_receiverDataList.size()) {
        m_receiverDataList[rx].cwDecode = value;
    }
    if (m_radioModel && rx >= 0 && rx < m_radioModel->slices().size() && m_radioModel->slices()[rx]) {
        m_radioModel->slices()[rx]->setCwDecodeEnabled(value);
    }
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
    if (SliceModel* slice = sliceModel(rx))
        return slice->dspMode();
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

void Settings::setTxFullDuplex(bool fullDuplex) {
    if (m_txFullDuplex != fullDuplex) {
        m_txFullDuplex = fullDuplex;
        emit txFullDuplexChanged(fullDuplex);
    }
}

void Settings::setRepeaterOffset(int offset)
{
m_repeaterOffset = (double)offset;
 emit repeaterOffsetchanged(offset);
}

void Settings::setFMPreEmphasize(int value)
{
    m_audioConfig->setFmPreemphasis(value);
    emit fmPremphasizechanged(value);
}

void Settings::setPhaseRotator(int value)
{
    m_audioConfig->setPhaseRotator(value);
    emit phaseRotatorChanged(value);
}

void Settings::setPhaseRotatorAuto(bool enabled)
{
    m_audioConfig->setPhaseRotatorAuto(enabled);
    emit phaseRotatorAutoChanged(m_audioConfig->phaseRotatorAuto());
}

void Settings::requestPhaseRotatorAutoReset()
{
    emit phaseRotatorAutoResetRequested();
}

void Settings::setPhaseRotatorStatus(const QString &status)
{
    emit phaseRotatorStatusChanged(status);
}

void Settings::setCtcssToneHz(int hz)
{
    m_audioConfig->setCtcssToneHz(hz);
    emit ctcssToneHzChanged(m_audioConfig->ctcssToneHz());
}

void Settings::setRxEqEnabled(bool enabled)
{
    m_audioConfig->setRxEqEnabled(enabled);
    emit rxEqChanged();
}

void Settings::setRxEqBands(const QVector<int> &bands)
{
    m_audioConfig->setRxEqBands(bands);
    emit rxEqChanged();
}

void Settings::setRxEqBand(int index, int gainDb)
{
    m_audioConfig->setRxEqBand(index, gainDb);
    emit rxEqChanged();
}

void Settings::setTxEqEnabled(bool enabled)
{
    m_audioConfig->setTxEqEnabled(enabled);
    emit txEqChanged();
}

void Settings::setTxEqBands(const QVector<int> &bands)
{
    m_audioConfig->setTxEqBands(bands);
    emit txEqChanged();
}

void Settings::setTxEqBand(int index, int gainDb)
{
    m_audioConfig->setTxEqBand(index, gainDb);
    emit txEqChanged();
}

void Settings::setRxEqCurveDeg(int deg)
{
    m_audioConfig->setRxEqCurveDeg(deg);
    emit rxEqChanged();
}

void Settings::setTxEqCurveDeg(int deg)
{
    m_audioConfig->setTxEqCurveDeg(deg);
    emit txEqChanged();
}

void Settings::setCfcEnabled(bool enabled)
{
    m_audioConfig->setCfcEnabled(enabled);
    emit cfcChanged();
}

void Settings::setCfcPeqEnabled(bool enabled)
{
    m_audioConfig->setCfcPeqEnabled(enabled);
    emit cfcChanged();
}

void Settings::setCfcPrecomp(double db)
{
    m_audioConfig->setCfcPrecomp(db);
    emit cfcChanged();
}

void Settings::setCfcPrePeq(double db)
{
    m_audioConfig->setCfcPrePeq(db);
    emit cfcChanged();
}

void Settings::setCfcCurveDeg(int deg)
{
    m_audioConfig->setCfcCurveDeg(deg);
    emit cfcChanged();
}

void Settings::setCfcLevel(int index, double db)
{
    m_audioConfig->setCfcLevel(index, db);
    emit cfcChanged();
}

void Settings::setCfcPostBand(int index, double db)
{
    m_audioConfig->setCfcPost(index, db);
    emit cfcChanged();
}

void Settings::setEmnrPost2Enabled(bool enabled)
{
    m_audioConfig->setEmnrPost2Enabled(enabled);
    emit emnrPost2Changed();
}

void Settings::setEmnrPost2Factor(double pct)
{
    m_audioConfig->setEmnrPost2Factor(pct);
    emit emnrPost2Changed();
}

void Settings::setEmnrPost2Nlevel(double pct)
{
    m_audioConfig->setEmnrPost2Nlevel(pct);
    emit emnrPost2Changed();
}

void Settings::setEmnrPost2Taper(double pct)
{
    m_audioConfig->setEmnrPost2Taper(pct);
    emit emnrPost2Changed();
}

void Settings::setEmnrPost2Rate(double seconds)
{
    m_audioConfig->setEmnrPost2Rate(seconds);
    emit emnrPost2Changed();
}

void Settings::setFmDeveation(int value)
{
    m_audioConfig->setFmDeviation(value);
    emit fmdeveationchanged(value);
}

void Settings::setAMCarrierLevel(int level)
{
    qDebug() << "set Am carrier level" << level;
    m_audioConfig->setAmCarrierLevel(level);
    emit amCarrierlevelchanged(level);
}

void Settings::setAudioCompression(int level){
    m_audioConfig->setAudioCompression(level);
    emit audioCompressionchanged(level);
}

void Settings::setInternalCw(int InternalCw) {
    m_cwConfig->setInternalCw(InternalCw);
    emit(InternalCwChanged(InternalCw));
}

void Settings::setCwKeyerMode(int mCwKeyerMode) {
    m_cwConfig->setKeyerMode(mCwKeyerMode);
    emit(CwKeyerModeChanged(mCwKeyerMode));
}

void Settings::setCwKeyReversed(int mCwKeyReversed) {
    m_cwConfig->setKeyReversed(mCwKeyReversed);
    emit(CwKeyReversedChanged(mCwKeyReversed));
}

void Settings::setCwKeyerSpeed(int mCwKeyerSpeed) {
    m_cwConfig->setKeyerSpeed(mCwKeyerSpeed);
    emit(CwKeyerSpeedChanged(mCwKeyerSpeed));
}

void Settings::setCwSidetoneVolume(int mCwSidetoneVolume) {
    m_cwConfig->setSidetoneVolume(mCwSidetoneVolume);
    emit(CwSidetoneVolumeChanged(mCwSidetoneVolume));
}

void Settings::setCwPttDelay(int mCwPttDelay) {
    m_cwConfig->setPttDelay(mCwPttDelay);
    emit(CwPttDelayChanged(mCwPttDelay));
}

void Settings::setCwHangTime(int mCwHangTime) {
    m_cwConfig->setHangTime(mCwHangTime);
    emit(CwHangTimeChanged(mCwHangTime));
}

void Settings::setCwSidetoneFreq(int mCwSidetoneFreq) {
    m_cwConfig->setSidetoneFreq(mCwSidetoneFreq);
    emit(CwSidetoneFreqChanged(mCwSidetoneFreq));
}

void Settings::setCwKeyerWeight(int val){
    m_cwConfig->setKeyerWeight(val);
    emit(CwKeyerWeightChanged(val));
}

void Settings::setCwKeyerSpacing(int val) {
    m_cwConfig->setKeyerSpacing(val);
    emit(CwKeyerSpacingChanged(val));
}

void Settings::syncSlicesWithSettings() {
    if (!m_radioModel) return;
    m_radioModel->setPanadapterColors(m_displayConfig->panadapterColors());
    connect(m_radioModel, &RadioModel::colorsChanged, this, &Settings::panadapterColorChanged);

    for (int i = 0; i < m_receiverDataList.size() && i < m_radioModel->slices().size(); ++i) {
        auto slice = m_radioModel->slices().at(i);
        if (!slice) continue;

        // Frequency / mode / filters / volume / AGC / DSP-display: SliceModel -> runtime (not relayed via Settings).

        const qint64 vfoA = (m_receiverDataList[i].vfoAFrequency > 0)
                                ? m_receiverDataList[i].vfoAFrequency
                                : m_receiverDataList[i].vfoFrequency;
        const qint64 vfoB = (m_receiverDataList[i].vfoBFrequency > 0)
                                ? m_receiverDataList[i].vfoBFrequency
                                : vfoA;
        const SliceModel::ActiveVfo active =
            (m_receiverDataList[i].activeVfo == 1) ? SliceModel::VfoB : SliceModel::VfoA;
        slice->setVfoMemories(vfoA, vfoB, active);
        slice->setCenterFrequency(m_receiverDataList[i].ctrFrequency);
        const HamBand band = m_receiverDataList[i].hamBand;
        DSPMode startupMode = m_receiverDataList[i].dspMode;
        if (band >= 0 && band < m_receiverDataList[i].dspModeList.size())
            startupMode = m_receiverDataList[i].dspModeList[band];
        slice->setDspMode(startupMode);
        slice->setFilterLow((float)m_receiverDataList[i].filterLo);
        slice->setFilterHigh((float)m_receiverDataList[i].filterHi);
        slice->setAgcMode(m_receiverDataList[i].agcMode);
        slice->setAgcGain((int)m_receiverDataList[i].acgGain);
        slice->setAgcMaxGain(m_receiverDataList[i].agcMaximumGain_dB);
        slice->setAgcFixedGain((int)m_receiverDataList[i].agcFixedGain_dB);
        slice->setAgcHangThreshold(m_receiverDataList[i].agcHangThreshold);
        slice->setAgcSlope(m_receiverDataList[i].agcSlope);
        slice->setAnf(m_receiverDataList[i].anf);
        slice->setSnb(m_receiverDataList[i].snb);
        slice->setNbMode(m_receiverDataList[i].nbMode);
        slice->setNrMode(m_receiverDataList[i].nr);
        slice->setNr2GainMethod(m_receiverDataList[i].nr2_gain_method);
        slice->setNr2NpeMethod(m_receiverDataList[i].nr2_npe_method);
        slice->setNr2Ae(m_receiverDataList[i].nr2_ae);
        slice->setNrAgc(m_receiverDataList[i].nr_agc);
        slice->setSMeterHoldTime(m_displayConfig->sMeterHoldTime());
        slice->setFftSize(m_receiverDataList[i].fftsize);
        slice->setSpectrumAveraging(m_receiverDataList[i].spectrumAveraging);
        slice->setSpectrumAveragingCnt(m_receiverDataList[i].averagingCnt);
        slice->setPanAveragingMode(m_receiverDataList[i].panAvMode);
        slice->setPanDetectorMode(m_receiverDataList[i].panDetMode);
        slice->setVolume(m_receiverDataList[i].audioVolume);
        slice->setPanMode(m_receiverDataList[i].panMode);
        slice->setWaterfallMode(m_receiverDataList[i].waterfallMode);
        slice->setWaterfallOffsetLo(m_receiverDataList[i].waterfallOffsetLo);
        slice->setWaterfallOffsetHi(m_receiverDataList[i].waterfallOffsetHi);
        slice->setPanGrid(m_receiverDataList[i].panGrid);
        slice->setPeakHold(m_receiverDataList[i].peakHold);
        const qreal dBmMin = (band >= 0 && band < m_receiverDataList[i].dBmPanScaleMinList.size())
            ? m_receiverDataList[i].dBmPanScaleMinList.at(band)
            : -120.0;
        const qreal dBmMax = (band >= 0 && band < m_receiverDataList[i].dBmPanScaleMaxList.size())
            ? m_receiverDataList[i].dBmPanScaleMaxList.at(band)
            : -10.0;
        slice->setDBmPanScaleMin(dBmMin);
        slice->setDBmPanScaleMax(dBmMax);
        slice->setWaterfallOffsetLo(m_receiverDataList[i].waterfallOffsetLo);
        slice->setWaterfallOffsetHi(m_receiverDataList[i].waterfallOffsetHi);
        slice->setPanGrid(m_receiverDataList[i].panGrid);
        slice->setPeakHold(m_receiverDataList[i].peakHold);
        slice->setCwDecodeEnabled(m_receiverDataList[i].cwDecode);
    }
}

void Settings::syncSettingsWithSlices() {
    if (!m_radioModel) return;
    m_displayConfig->setPanadapterColors(m_radioModel->panadapterColors());
    for (int i = 0; i < m_receiverDataList.size() && i < m_radioModel->slices().size(); ++i) {
        auto slice = m_radioModel->slices().at(i);
        if (!slice) continue;
        m_receiverDataList[i].vfoAFrequency = slice->vfoAFrequency();
        m_receiverDataList[i].vfoBFrequency = slice->vfoBFrequency();
        m_receiverDataList[i].activeVfo = (slice->activeVfo() == SliceModel::VfoB) ? 1 : 0;
        m_receiverDataList[i].vfoFrequency = slice->frequency();
        m_receiverDataList[i].ctrFrequency = slice->centerFrequency();
        m_receiverDataList[i].dspMode = slice->dspMode();
        const HamBand band = m_receiverDataList[i].hamBand;
        if (band >= 0 && band < m_receiverDataList[i].dspModeList.size())
            m_receiverDataList[i].dspModeList[band] = slice->dspMode();
        if (band >= 0 && band < m_receiverDataList[i].dBmPanScaleMinList.size())
            m_receiverDataList[i].dBmPanScaleMinList[band] = slice->dBmPanScaleMin();
        if (band >= 0 && band < m_receiverDataList[i].dBmPanScaleMaxList.size())
            m_receiverDataList[i].dBmPanScaleMaxList[band] = slice->dBmPanScaleMax();
        m_receiverDataList[i].filterLo = (qreal)slice->filterLow();
        m_receiverDataList[i].filterHi = (qreal)slice->filterHigh();
        m_receiverDataList[i].agcMode = slice->agcMode();
        m_receiverDataList[i].acgGain = (qreal)slice->agcGain();
        m_receiverDataList[i].agcMaximumGain_dB = slice->agcMaxGain();
        m_receiverDataList[i].agcFixedGain_dB = (qreal)slice->agcFixedGain();
        m_receiverDataList[i].agcHangThreshold = slice->agcHangThreshold();
        m_receiverDataList[i].agcSlope = slice->agcSlope();
        m_receiverDataList[i].anf = slice->anf();
        m_receiverDataList[i].snb = slice->snb();
        m_receiverDataList[i].nbMode = slice->nbMode();
        m_receiverDataList[i].nr = slice->nrMode();
        m_receiverDataList[i].nr2_gain_method = slice->nr2GainMethod();
        m_receiverDataList[i].nr2_npe_method = slice->nr2NpeMethod();
        m_receiverDataList[i].nr2_ae = slice->nr2Ae();
        m_receiverDataList[i].nr_agc = slice->nrAgc();
        m_receiverDataList[i].fftsize = slice->fftSize();
        m_receiverDataList[i].spectrumAveraging = slice->spectrumAveraging();
        m_receiverDataList[i].averagingCnt = slice->spectrumAveragingCnt();
        m_receiverDataList[i].panAvMode = slice->panAveragingMode();
        m_receiverDataList[i].panDetMode = slice->panDetectorMode();
        m_receiverDataList[i].audioVolume = slice->volume();
        m_receiverDataList[i].panMode = slice->panMode();
        m_receiverDataList[i].waterfallMode = slice->waterfallMode();
        m_receiverDataList[i].waterfallOffsetLo = slice->waterfallOffsetLo();
        m_receiverDataList[i].waterfallOffsetHi = slice->waterfallOffsetHi();
        m_receiverDataList[i].panGrid = slice->panGrid();
        m_receiverDataList[i].peakHold = slice->peakHold();
        m_receiverDataList[i].cwDecode = slice->cwDecodeEnabled();
    }
}

void Settings::syncTransmitWithSettings() {
    if (!m_radioModel)
        return;

    TransmitModel* tx = m_radioModel->transmit();
    if (!tx)
        return;

    tx->setAmCarrierLevel(static_cast<int>(m_audioConfig->amCarrierLevel()));
    tx->setAudioCompression(m_audioConfig->audioCompression());
    tx->setFmDeviation(static_cast<int>(m_audioConfig->fmDeviation()));
    tx->setFmPreEmphasis(m_audioConfig->fmPreemphasis() != 0);
    tx->setPhaseRotator(m_audioConfig->phaseRotator() != 0);
    tx->setPhaseRotatorAuto(m_audioConfig->phaseRotatorAuto());
    tx->setCtcssToneHz(m_audioConfig->ctcssToneHz());
    tx->setTxEqEnabled(m_audioConfig->txEqEnabled());
    tx->setTxEqBands(m_audioConfig->txEqBands());
    tx->setTxEqCurveDeg(m_audioConfig->txEqCurveDeg());
    tx->setCfcEnabled(m_audioConfig->cfcEnabled());
    tx->setCfcPeqEnabled(m_audioConfig->cfcPeqEnabled());
    tx->setCfcPrecomp(m_audioConfig->cfcPrecomp());
    tx->setCfcPrePeq(m_audioConfig->cfcPrePeq());
    tx->setCfcCurveDeg(m_audioConfig->cfcCurveDeg());
    tx->setCfcLevels(m_audioConfig->cfcLevels());
    tx->setCfcPost(m_audioConfig->cfcPost());
    tx->setMicInputDev(m_audioConfig->micInputDev());
    tx->setMicInputSourceName(m_audioConfig->micInputSourceName());
    tx->setDigitalAudioInputDev(m_audioConfig->digitalAudioInputDev());
    tx->setDigitalInputSourceName(m_audioConfig->digitalInputSourceName());
    tx->setCwKeyerMode(m_cwConfig->keyerMode());
    tx->setInternalCw(m_cwConfig->internalCw() > 0);
    tx->setCwKeyReversed(m_cwConfig->keyReversed() > 0);
    tx->setCwKeyerSpacing(m_cwConfig->keyerSpacing() > 0);
    tx->setCwKeyerSpeed(m_cwConfig->keyerSpeed());
    tx->setCwPttDelay(m_cwConfig->pttDelay());
    tx->setCwSidetoneFreq(m_cwConfig->sidetoneFreq());
    tx->setCwSidetoneVolume(m_cwConfig->sidetoneVolume());
    tx->setCwHangTime(m_cwConfig->hangTime());
    tx->setCwKeyerWeight(m_cwConfig->keyerWeight());
}

void Settings::syncSettingsWithTransmit() {
    if (!m_radioModel)
        return;

    const TransmitModel* tx = m_radioModel->transmit();
    if (!tx)
        return;

    m_audioConfig->setAmCarrierLevel(tx->amCarrierLevel());
    m_audioConfig->setAudioCompression(tx->audioCompression());
    m_audioConfig->setFmDeviation(tx->fmDeviation());
    m_audioConfig->setFmPreemphasis(tx->fmPreEmphasis() ? 1 : 0);
    m_audioConfig->setPhaseRotator(tx->phaseRotator() ? 1 : 0);
    m_audioConfig->setPhaseRotatorAuto(tx->phaseRotatorAuto());
    m_audioConfig->setCtcssToneHz(tx->ctcssToneHz());
    m_audioConfig->setTxEqEnabled(tx->txEqEnabled());
    m_audioConfig->setTxEqBands(tx->txEqBands());
    m_audioConfig->setTxEqCurveDeg(tx->txEqCurveDeg());
    m_audioConfig->setCfcEnabled(tx->cfcEnabled());
    m_audioConfig->setCfcPeqEnabled(tx->cfcPeqEnabled());
    m_audioConfig->setCfcPrecomp(tx->cfcPrecomp());
    m_audioConfig->setCfcPrePeq(tx->cfcPrePeq());
    m_audioConfig->setCfcCurveDeg(tx->cfcCurveDeg());
    for (int i = 0; i < tx->cfcLevels().size(); ++i)
        m_audioConfig->setCfcLevel(i, tx->cfcLevels().at(i));
    for (int i = 0; i < tx->cfcPost().size(); ++i)
        m_audioConfig->setCfcPost(i, tx->cfcPost().at(i));
    m_audioConfig->setMicInputDev(tx->micInputDev());
    m_audioConfig->setMicInputSourceName(tx->micInputSourceName());
    m_audioConfig->setDigitalAudioInputDev(tx->digitalAudioInputDev());
    m_audioConfig->setDigitalInputSourceName(tx->digitalInputSourceName());
    m_cwConfig->setKeyerMode(tx->cwKeyerMode());
    m_cwConfig->setInternalCw(tx->internalCw() ? 1 : 0);
    m_cwConfig->setKeyReversed(tx->cwKeyReversed() ? 1 : 0);
    m_cwConfig->setKeyerSpacing(tx->cwKeyerSpacing() ? 1 : 0);
    m_cwConfig->setKeyerSpeed(tx->cwKeyerSpeed());
    m_cwConfig->setPttDelay(tx->cwPttDelay());
    m_cwConfig->setSidetoneFreq(tx->cwSidetoneFreq());
    m_cwConfig->setSidetoneVolume(tx->cwSidetoneVolume());
    m_cwConfig->setHangTime(tx->cwHangTime());
    m_cwConfig->setKeyerWeight(tx->cwKeyerWeight());
}
