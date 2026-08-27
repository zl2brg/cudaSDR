/**
* @file  cusdr_settings.h
* @brief settings header file for cuSDR
* @author by Hermann von Hasseln, DL3HVH
* @version 0.1
* @date 2010-11-18
*/

/*   
 *   Copyright 2010 - 2015 Hermann von Hasseln, DL3HVH
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
 
#ifndef CUSDR_SETTINGS_H
#define CUSDR_SETTINGS_H
#define USE_INTERNAL_AUDIO
//#define DEBUG

#include <QObject>
#include <QErrorMessage>
#include <QMutex>
#include <QtNetwork>
#include <QString>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#include <qaudiodevice.h>
#include <QMap>
#include <QReadWriteLock>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <atomic>

#include "cusdr_hamDatabase.h"
#include "fftw3.h"
#include "portaudio.h"

#include "Settings/SettingsTypes.h"
#include "Settings/NetworkConfig.h"
#include "Settings/DisplayConfig.h"
#include "Settings/HardwareConfig.h"
#include "Settings/AudioConfig.h"
#include "Settings/CWConfig.h"
#include "Settings/ReceiverConfig.h"
#include "Settings/AlexConfig.h"
#include "Settings/TransmitConfig.h"
#include "Settings/FreeDVConfig.h"
#include "Settings/WindowConfig.h"
#include "Settings/TciConfig.h"
#include "Settings/SoapyConfig.h"
#include "Settings/WidebandConfig.h"
#include "Settings/PennyConfig.h"
#include "Util/cusdr_queue.h"
#include "Util/display_utils.h"


// test for OpenCL
//#include "CL/qclcontext.h"


// **************************************
// messages

#define BANDSCOPE_IN_USE "Error: bandscope in use"
#define BANDSCOPE_NOT_OWNER "Error: Not owner of bandscope"

#define RECEIVER_INVALID "Error: Invalid Receiver"
#define RECEIVER_IN_USE "Error: Receiver in use"
#define RECEIVER_NOT_OWNER "Error: Not owner of receiver"

#define CLIENT_ATTACHED "Error: Client is already attached to receiver"
#define CLIENT_DETACHED "Error: Client is not attached to receiver"

#define INVALID_COMMAND "Error: Invalid Command"

#define OK "OK"

// **************************************
// constants

#define ONEPI 3.14159265358979323846264338328
#define TWOPI 6.28318530717958647692528676656
//#define AGCOFFSET 33.0

#define MAXFREQUENCY 61440000
#ifdef HL
#define MAXHPFREQUENCY 30720000
#else
#define MAXHPFREQUENCY 61440000
#endif
#define MINDBM -180
#define MAXDBM 10
#define MINDISTDBM -150
#define MAXDISTDBM 150
#define MAX_FFTSIZE	262144
#define TX_ID 10


// **************************************
// receiver settings

#define MAX_RECEIVERS				8
#define MAX_BANDS					22
#define BUFFER_SIZE					1024
#define SAMPLE_BUFFER_SIZE			4096
#define DSP_SAMPLE_SIZE             1024
#define BANDSCOPE_BUFFER_SIZE		4096

#define								SMALL_PACKETS
#define BIGWIDEBANDSIZE				16384
//#define BIGWIDEBANDSIZE				32768
#define SMALLWIDEBANDSIZE			4096


// **************************************
// IO buffer, command & control settings

#define IO_BUFFERS					16
#define IO_BUFFER_SIZE				512
#define IO_HEADER_SIZE				8
#define IO_AUDIOBUFFER_SIZE			8192

#define SYNC						0x7F

#define IQ_DATAGRAM_BUFFERS			512
#define AUDIO_DATAGRAM_BUFFERS		512

#define METIS_HEADER_SIZE			8
#define METIS_DATA_SIZE				1032

#define ALEX_PARAMETERS				15

// uncomment to compile code that allows for SYNC error recovery
#define RESYNC

#define MOX_DISABLED				0x00
#define MOX_ENABLED					0x01

#define ATLAS_10MHZ_SOURCE			0x00
#define PENELOPE_10MHZ_SOURCE		0x04
#define MERCURY_10MHZ_SOURCE		0x08
#define PENELOPE_122_88MHZ_SOURCE	0x00
#define MERCURY_122_88MHZ_SOURCE	0x10
#define PENELOPE_PRESENT			0x20
#define MERCURY_PRESENT				0x40
#define MIC_SOURCE_PENELOPE			0x80

//#define MIC_SOURCE_JANUS			0x00
//#define CONFIG_NONE					0x00
//#define CONFIG_BOTH					0x60


//#define SPEED_48KHZ					0x00
//#define SPEED_96KHZ					0x01
//#define SPEED_192KHZ				0x02

#define MODE_CLASS_E				0x01
#define MODE_OTHERS					0x00

#define ALEX_ATTENUATION_0DB		0x00
#define ALEX_ATTENUATION_10DB		0x01
#define ALEX_ATTENUATION_20DB		0x02
#define ALEX_ATTENUATION_30DB		0x03
#define LT2208_GAIN_OFF				0x00
#define LT2208_GAIN_ON				0x04
#define LT2208_DITHER_OFF			0x00
#define LT2208_DITHER_ON			0x08
#define LT2208_RANDOM_OFF			0x00
#define LT2208_RANDOM_ON			0x10

#define DV_ENGINE_FREEDV			0x00

namespace QSDR {

	enum _Error { 
		
		NoError,
		NotImplemented,
		HwIOError,
		ServerModeError,
		OpenError,
		DataReceiverThreadError,
		DataProcessThreadError,
		WideBandDataProcessThreadError,
		AudioThreadError,
		UnderrunError, 
		FirmwareError,
		FatalError
	};

	enum _DataEngineState {

		DataEngineDown,
		DataEngineUp
	};

	enum _ServerMode {

		NoServerMode,
		SDRMode,
	};

	enum _HWInterfaceMode { 
		
		NoInterfaceMode, 
		Metis,
        Hermes,
        SoapySDR
	};
}

Q_DECLARE_METATYPE(QSDR::_Error)
Q_DECLARE_METATYPE(QSDR::_DataEngineState)
Q_DECLARE_METATYPE(QSDR::_ServerMode)
Q_DECLARE_METATYPE(QSDR::_HWInterfaceMode)


// **************************************
enum {
	prefixNothing = 0,	/*!< No prefix. */

	/* SI units. */
	prefixKilo = 1,		/*!< Kilo prefix 1000^1 = 10^3. */
	prefixMega = 2,		/*!< Mega prefix 1000^2 = 10^6. */
	prefixGiga = 3,		/*!< Giga prefix 1000^3 = 10^9. */
	prefixTera = 4,		/*!< Tera prefix 1000^4 = 10^12. */
	prefixPeta = 5,		/*!< Peta prefix 1000^5 = 10^15. */
	prefixExa = 6,		/*!< Exa prefix 1000^6 = 10^18. */
	prefixZetta = 7,	/*!< Zetta prefix 1000^7 = 10^21. */
	prefixYotta = 8,	/*!< Yotta prefix 1000^8 = 10^24. */
	prefixSiMax = prefixYotta,

	/* IEC 60027 units. */
	prefixKibi = 1,		/*!< Kibi prefix 1024^1 = 2^10. */
	prefixMebi = 2,		/*!< Mebi prefix 1024^2 = 2^20. */
	prefixGibi = 3,		/*!< Gibi prefix 1024^3 = 2^30. */
	prefixTebi = 4,		/*!< Tebi prefix 1024^4 = 2^40. */
	prefixPebi = 5,		/*!< Pebi prefix 1024^5 = 2^50. */
	prefixExbi = 6,		/*!< Exbi prefix 1024^6 = 2^60. */
	prefixZebi = 7,		/*!< Zebi prefix 1024^7 = 2^70. */
	prefixYobi = 8,		/*!< Yobi prefix 1024^8 = 2^80. */
	prefixIecMax = prefixYobi,
};
  

#define DEVICE_PORT 1024
#define DATA_PORT 8886

// **************************************
// Audio definitions


typedef QVector<float> qVectorFloat;

// WDSP TX analyzer levels run hot vs RX; offset panadapter display (dB).
constexpr float kTxPanadapterDisplayDbOffset = DisplayUtils::kTxPanadapterDisplayDbOffset;
inline void applyTxPanadapterDisplayOffset(qVectorFloat &spectrum) {
    DisplayUtils::applyTxPanadapterDisplayOffset(spectrum);
}
inline void prepareTxPanadapterSpectrum(qVectorFloat &spectrum, int panSampleRate,
                                        int txAnalyzerRate = DisplayUtils::kTxAnalyzerSampleRate) {
    DisplayUtils::prepareTxPanadapterSpectrum(spectrum, panSampleRate, txAnalyzerRate);
}
typedef QVector<double> qVectorDouble;

typedef struct _frequency {

	int	freqMHz;
	int	freqkHz;

	qint64 frequency;

} TFrequency;


typedef struct _ccParameterRx {

	THPSDRDevices	devices;

	uchar	roundRobin;	// roundRobin is varied in a round-robin fashion in order to decode
						// all values which are sent in sequence. 

	bool	ptt;		// PTT  (1 = active, 0 = inactive), GPIO[23]= Ozy J8-8, Hermes J16-1
	int 	dash;		// DASH (1 = active, 0 = inactive), GPIO[21]= Ozy J8-6, Hermes J6-2
	int 	dot;		// DOT  (1 = active, 0 = inactive), GPIO[22]= Ozy J8-7, Hermes J6-3
    int     previous_dash;//
    int     previous_dot;//
    bool	lt2208;		// LT2208 Overflow (1 = active, 0 = inactive)
	bool	hermesI01;	// Hermes I01 (0 = active, 1 = inactive)
	bool	hermesI02;	// Hermes I02 (0 = active, 1 = inactive)
	bool	hermesI03;	// Hermes I03 (0 = active, 1 = inactive)
	bool	hermesI04;	// Hermes I04 (0 = active, 1 = inactive)
	bool	cyclopsPLL;	// Cyclops PLL locked (0 = unlocked, 1 = locked)
	bool	cyclops;	// Cyclops - Mercury frequency changed, bit toggles 

	//int		mercuryFirmwareVersion;			// Mercury firmware version
	//int		penelopeFirmwareVersion;		// Penelope firmware version
	//int		networkDeviceFirmwareVersion;	// Metis/Hermes firmware version

	quint16	ain1;		// Forward Power from Alex or Apollo
	quint16	ain2;		// Reverse Power from Alex or Apollo
	quint16	ain3;		// AIN3 from Penny or Hermes
	quint16	ain4;		// AIN4 from Penny or Hermes
	quint16	ain5;		// Forward Power from Penelope or Hermes
	quint16	ain6;		// AIN6,13.8v supply on Hermes

	bool	mercury1_LT2208;	//Mercury 1 LT2208 Overflow (1 = active, 0 = inactive)
	bool	mercury2_LT2208;	//Mercury 2 LT2208 Overflow (1 = active, 0 = inactive)
	bool	mercury3_LT2208;	//Mercury 3 LT2208 Overflow (1 = active, 0 = inactive)
	bool	mercury4_LT2208;	//Mercury 4 LT2208 Overflow (1 = active, 0 = inactive)


} TCCParameterRx;

typedef struct _ccParameterTx {

	bool	mox;
	bool	ptt;
	bool	lineIn;
	bool	micGain20dB;
	bool	pennyOCenabled;
	bool	vnaMode;

	uchar	clockByte;
	uchar	timeStamp;
	uchar	commonMercuryFrequencies;

	int		hpsdr10MhzSource;
	int		hpsdr122_88MhzSource;
	int		hpsdrConfig;
	int		duplex;
	int		mercuryAttenuator;
	int		dither;
	int		random;
	int		currentAlexState;
    uchar   drivelevel;
    double  repeaterOffset;
    bool    use_repeaterOffset;
    int     fmPremphasize;
    double  amCarrierLevel;
    int     audioCompression;
    double  fmDeveation;
    double  txFrequency;
    DSPMode mode;

	HamBand		currentBand;

	QList<int>					mercuryAttenuators;
	QList<int>					alexStates;
	//QList<TAlexConfiguration>	alexConfiguration;
	quint16						alexConfig;
	QList<int>					rxJ6pinList;
	QList<int>					txJ6pinList;

} TCCParameterTx;

class RigCtlServer;
class TciServer;

typedef struct _iqPacket {
	QByteArray	payload;
	quint16		sourcePort;

	_iqPacket()
		: sourcePort(0)
	{}

	_iqPacket(const QByteArray &data, quint16 port)
		: payload(data)
		, sourcePort(port)
	{}

} TIQPacket;

typedef struct _networkDeviceCard {
    int             protocol;
    int             device;
    int             sw_version;
    int             status;
    int             max_receivers;
    int             max_transmitters;
    int             adcs;
    int             dacs;
    double          frequency_min;
    double          frequency_max;

	QHostAddress	ip_address;
	char			mac_address[18];
	int				boardID;
	QString			boardName;
#ifdef SOAPYSDR
      struct soapy {
        char version[128];
        char hardware_key[64];
        char driver_key[64];
        int rtlsdr_count;
        int sdrplay_count;
        int sample_rate;
        size_t rx_channels;
        size_t rx_gains;
        char **rx_gain;
        SoapySDRRange *rx_range;
        gboolean rx_has_automatic_gain;
        gboolean rx_has_automatic_dc_offset_correction;
        size_t rx_antennas;
        char **rx_antenna;
        size_t tx_channels;
        size_t tx_gains;
        char **tx_gain;
        SoapySDRRange *tx_range;
        size_t tx_antennas;
        char **tx_antenna;
    size_t sensors;
        char **sensor;
        gboolean has_temp;
        char address[64];
      } soapy;
#endif

} TNetworkDevicecard;

Q_DECLARE_METATYPE (TNetworkDevicecard)
Q_DECLARE_METATYPE (QList<TNetworkDevicecard>)

// Every scalar carries a default: unset members used to reach the INI via
// syncSettingsWithSlices()/saveSettings() and persist as garbage.
typedef struct _receiver {

	QSDR::_DSPCore		dspCore = QSDR::QtDSP;

	HamBand				hamBand = gen;
	HamBand				lastHamBand = gen;
	DSPMode				dspMode = LSB;
	ADCMode				adcMode = adc1;
	AGCMode				agcMode = agcMED;
	TDefaultFilterMode	defaultFilterMode = filterLSB;
	PanGraphicsMode		panMode = Line;
	WaterfallColorMode	waterfallMode = Enhanced;
	PanAveragingMode    panAvMode = AV_MODE_RECURSIVE;
	PanDetectorMode     panDetMode = DET_MODE_ROSENFELL;
	int 				fftsize = 1;
	int					fmsqLevel = 0;

	QList<qint64>		lastCenterFrequencyList;
	QList<qint64>		lastVfoFrequencyList;
	QList<int>			mercuryAttenuators;
	QList<qreal>		dBmPanScaleMinList;
	QList<qreal>		dBmPanScaleMaxList;
	QList<DSPMode>		dspModeList;

	bool	hangEnabled = false;
	bool	agcLines = false;
	bool	panLocked = false;
	bool	spectrumAveraging = true;
	bool	hairCross = false;
	bool	panGrid = true;
	bool	peakHold = false;
	bool	clickVFO = false;

	qint64	ctrFrequency = 7050000;
	qint64	vfoFrequency = 7050000;
	qint64	vfoAFrequency = 7050000;
	qint64	vfoBFrequency = 7050000;
	int		activeVfo = 0; // 0 = A, 1 = B (SliceModel::ActiveVfo)
	int		filterSlope = 1; // SliceModel filter slope (0 Soft …); residual until Slice exists
	qint64	ncoFrequency = 0;

	float	freqRulerPosition = 0.5f;
	float	audioVolume = 0.25f;

	qreal	mouseWheelFreqStep = 100.0;
	qreal	filterLo = -3050.0;
	qreal	filterHi = -150.0;


    int     m_filterIndex = 0;
	qreal	acgGain = 100.0;
	qreal	acgThreshold_dB = 0.0;
	int		agcHangThreshold = 0;
	qreal	agcHangLevel = 0.0;
	int 	agcMaximumGain_dB = 30;
	qreal	agcAttackTime = 2.0;
	qreal	agcDecayTime = 250.0;
	qreal	agcHangTime = 100.0;
	qreal	agcFixedGain_dB = 30.0;
	int 	agcSlope = 0;

	int		sampleRate = 48000;
	int		framesPerSecond = 25;
	int		waterfallOffsetLo = -5;
	int		waterfallOffsetHi = 20;
	int		averagingCnt = 5;
	int		fftFactor = 1;
	int		nr = 0;
    int     nr_agc = 0;
    int		nbMode = 0;
    int     nr2_gain_method = 0;
    int     nr2_npe_method = 0;
    bool     nr2_ae = false;
	bool	anf = false;
	bool	snb = false;
	bool	cwDecode = false;
} TReceiver;

typedef struct _wideband {

	PanGraphicsMode	panMode = Line;

	bool	wideBandData = true;
	bool	wideBandDisplayStatus = false;
	bool	averaging = true;

	int	numberOfBuffers = 0;
	int	averagingCnt = 5;

	float	scalePosition = 0.0f;

	qreal	dBmWBScaleMin = -140.0;
	qreal	dBmWBScaleMax = -10.0;

} TWideband;

typedef struct _transmitter {

	QSDR::_DSPCore		dspCore;

	HamBand				hamBand;
	DSPMode				dspMode;
	AGCMode				agcMode;
	TDefaultFilterMode	defaultFilterMode;

	bool	txAllowed;
	qint64	frequency;

	float	audioVolume;

} TTransmitter;


typedef enum _smeterType {

	SIGNAL_STRENGTH,
	AVG_SIGNAL_STRENGTH,
	ADC_REAL,
	ADC_IMAG,
	AGC_GAIN,
	MIC,
	PWR,
	ALC,
	EQtap,
	LEVELER,
	COMP,
	CPDR,
	ALC_G,
	LVL_G,
	MIC_PK,
	ALC_PK,
	EQ_PK,
	LEVELER_PK,
	COMP_PK,
	CPDR_PK

} TMeterType;

typedef enum _windowtype {

  RECTANGULAR_WINDOW,
  HANNING_WINDOW,
  WELCH_WINDOW,
  PARZEN_WINDOW,
  BARTLETT_WINDOW,
  HAMMING_WINDOW,
  BLACKMAN2_WINDOW,
  BLACKMAN3_WINDOW,
  BLACKMAN4_WINDOW,
  EXPONENTIAL_WINDOW,
  RIEMANN_WINDOW,
  BLACKMANHARRIS_WINDOW,
  NUTTALL_WINDOW

} TWindowtype;



class SliceProcessor;

// *********************************************************************
// thread class

class QThreadEx : public QThread {

protected:
    void run() { exec(); }

};




// **************************************
// Settings class

class RadioModel;
class TransmitModel;
class SliceModel;
class Settings : public QObject {

	Q_OBJECT

public:
    void setRadioModel(RadioModel* model) { m_radioModel = model; }
    RadioModel* radioModel() const { return m_radioModel; }
    TransmitModel* transmitModel() const;
    SliceModel* sliceModel(int rx) const;
    void syncSlicesWithSettings();
    void syncSettingsWithSlices();
    void syncTransmitWithSettings();
    void syncSettingsWithTransmit();
public:
	static Settings *instance(QObject *parent = nullptr) {

		if (Settings::m_instance)
			return Settings::m_instance;

		Settings::m_instance = new Settings(parent);
		
		return Settings::m_instance;
	}

	static void delete_instance() {
	
		if (Settings::m_instance) {
		
			disconnect(Settings::m_instance, nullptr, nullptr, nullptr);
			delete Settings::m_instance;
			Settings::m_instance = nullptr;
		}
	}

    NetworkConfig *networkConfig() const { return m_networkConfig; }
    DisplayConfig *displayConfig() const { return m_displayConfig; }
    HardwareConfig *hardwareConfig() const { return m_hardwareConfig; }
    AudioConfig *audioConfig() const { return m_audioConfig; }
    CWConfig *cwConfig() const { return m_cwConfig; }
    AlexConfig *alexConfig() const { return m_alexConfigObj; }
    TransmitConfig *transmitConfig() const { return m_transmitConfig; }
    FreeDVConfig *freeDVConfig() const { return m_freeDVConfig; }
    WindowConfig *windowConfig() const { return m_windowConfig; }
    TciConfig *tciConfig() const { return m_tciConfig; }
    SoapyConfig *soapyConfig() const { return m_soapyConfig; }
    WidebandConfig *widebandConfig() const { return m_widebandConfig; }
    PennyConfig *pennyConfig() const { return m_pennyConfig; }
    QList<ReceiverConfig*> receiverConfigs() const { return m_receiverConfigs; }

	virtual ~Settings() override;

	QMutex 			settingsMutex;

private:
	explicit Settings(QObject *parent = nullptr);

	static Settings		*m_instance;

	QSettings			*settings;
	QSettings			*debugLog;
	QErrorMessage		*error;

    NetworkConfig       *m_networkConfig;
    DisplayConfig       *m_displayConfig;
    HardwareConfig      *m_hardwareConfig;
    AudioConfig         *m_audioConfig;
    CWConfig            *m_cwConfig;
    AlexConfig          *m_alexConfigObj;
    TransmitConfig      *m_transmitConfig;
    FreeDVConfig        *m_freeDVConfig;
    WindowConfig        *m_windowConfig;
    TciConfig           *m_tciConfig;
    SoapyConfig         *m_soapyConfig;
    WidebandConfig      *m_widebandConfig;
    PennyConfig         *m_pennyConfig;
    QList<ReceiverConfig*> m_receiverConfigs;

signals:
	void systemMessageEvent(const QString &msg, int);

	void masterSwitchChanged(bool power);

	void radioStateChanged(RadioState state);


	void systemStateChanged(
				QSDR::_Error err, 
				QSDR::_HWInterfaceMode hwmode, 
				QSDR::_ServerMode mode, 
				QSDR::_DataEngineState state);

	void graphicModeChanged(
				int rx,
				PanGraphicsMode panMode,
				WaterfallColorMode waterfallColorMode);

	void moxStateChanged(RadioState);
	void tuneStateChanged(RadioState);
	void cpuLoadChanged(short load);
	void txAllowedChanged(bool value);
	void multiRxViewChanged(int view);
	void freeDVStatusChanged(int rx, bool sync, float snr, quint64 rxFrames, quint64 txFrames);
	void freeDVModeChanged(int rx, int mode);

	void sampleSizeChanged(int rx, int size);
    void rxListChanged(QList<SliceProcessor *> rxList);
    void clientConnectedChanged(bool connect);
	void clientNoConnectedChanged(int client);
	void audioRxChanged(int rx);
	void receiverChanged(int value);
	void currentReceiverChanged(int rx);
	void connectedChanged(bool connect);

	void clientConnectedEvent(int client);
	void clientDisconnectedEvent(int client);
	void rxConnectedStatusChanged(int rx, bool value);
	void framesPerSecondChanged(int rx, int value);
	
	void settingsFilenameChanged(QString filename);
	void settingsLoadedChanged(bool loaded);

	void newServerNetworkInterface(QString nicName, QString ipAddress);
	void newHPSDRDeviceNIC(QString nicName, QString ipAddress);
	void serverNICChanged(int);
	void tciServerEnabledChanged(bool enabled);
	void tciRxGainChanged(float gain);
	void tciTxGainChanged(float gain);
	void hpsdrDeviceNICChanged(int);
	void socketBufferSizeChanged(int value);
	void manualSocketBufferChanged(bool value);
	//void metisCardListChanged(QList<TMetiscard> list);
	void metisCardListChanged(const QList<TNetworkDevicecard> &list);
	void hpsdrDevicesChanged(THPSDRDevices devices);
	void hpsdrNetworkDeviceChanged(TNetworkDevicecard card);
	void networkDeviceNumberChanged(int value);
	void maxFrequencyChanged(qint64 value);
	void minFrequencyChanged(qint64 value);
	void networkIOComboBoxEntryAdded(QString str);
	void clearNetworkIOComboBoxEntrySignal();
    void clearDiscoveredDevicesSignal();
	void searchMetisSignal();
    void searchSoapySignal();
    void soapyDeviceListChanged(const QList<TSoapyDevice> &list);
    void soapyDeviceChanged(TSoapyDevice device);
    void soapyMessageEvent(QString message);
    void soapyAntennaListChanged(QStringList list);
    void soapyTxAntennaListChanged(QStringList list);
    void soapyHardwareKeyChanged(QString key);
    void soapyRxAntennaChanged(QString antenna);
    void soapyTxAntennaChanged(QString antenna);
    void soapyLnaGainChanged(int gain);
    void soapyTiaGainChanged(int gain);
    void soapyPgaGainChanged(int gain);
    void soapyOverallGainChanged(int gain);
    void soapyOverallGainRangeChanged(int minGain, int maxGain);
    void soapyAutoCalibrateChanged(bool enabled);
    void soapyIQBalanceChanged(bool enabled);
	void serverAddrChanged(QString addr);
	void hpsdrDeviceLocalAddrChanged(QString addr);
	void serverPortChanged(quint16 port);
	void listenPortChanged(quint16 port);
	void audioPortChanged(quint16 port);
	void metisPortChanged(quint16 port);
	
	void showNetworkIO();
	void showWarning(const QString &msg);

	void callsignChanged();

	void mouseWheelFreqStepChanged(int rx, qreal value);
	void mainVolumeChanged(int rx, float volume );

	//void hermesPresenceChanged(bool value);
	void hpsdrHardwareChanged(int value);
	void hermesVersionChanged(int value);
	void mercuryPresenceChanged(bool value);
	void mercuryVersionChanged(int value);
	void penelopePresenceChanged(bool value);
	void penelopeVersionChanged(int value);
	void pennyLanePresenceChanged(bool value);
	void pennyLaneVersionChanged(int value);
	void alexPresenceChanged(bool value);
	void excaliburPresenceChanged(bool value);
	void metisVersionChanged(int value);
	//void alexConfigurationChanged(const QList<TAlexConfiguration> &conf);
	void alexConfigurationChanged(quint16 config);
	//void alexParametersChanged(TAlexParameters p);
	void alexStatesChanged(const QList<int> &states);
	void alexStateChanged(HamBand band, const QList<int> &states);
//	void alexStateChanged(int pos, int value);
	void alexManualStateChanged(bool value);
	void checkFirmwareVersionChanged(bool value);
	void pennyOCEnabledChanged(bool value);
	void rxJ6PinsChanged(const QList<int> &states);
	void txJ6PinsChanged(const QList<int> &states);

	void numberOfRXChanged(int value);
	void sampleRateChanged(int value);
	void mercuryAttenuatorChanged(HamBand band, int value);
	//void mercuryAttenuatorsChanged(const QList<int> &values);
	void ditherChanged(int value);
	void randomChanged(int value);
	void src10MhzChanged(int source);
	void src122_88MhzChanged(int source);
    void micSourceChanged(int source);
    void micInputChanged(int source);
	void digitalAudioInputChanged(int index);
	void classChanged(int value);
	void timingChanged(int value);
	void controlBytesOutChanged(unsigned char *values);
	//void ctrFrequencyChanged(bool value, int rx, long frequency);
	void ctrFrequencyChanged(int mode, int rx, qint64 frequency);
	//void vfoFrequencyChanged(bool value, int rx, long frequency);
	void vfoFrequencyChanged(int mode, int rx, qint64 frequency);
	void ncoFrequencyChanged(int rx, qint64 frequency);

	// wideband data
	void widebandOptionsChanged(TWideband options);
	void widebandStatusChanged(bool value);
	void widebandDataChanged(bool value);
	void widebanddBmScaleMinChanged(qreal value);
	void widebanddBmScaleMaxChanged(qreal value);
	void wideBandScalePositionChanged(float position);
	//void widebandAveragingChanged(bool value);
	//void widebandAveragingCntChanged(int value);
    void panAveragingModeChanged(int rx, int mode);
    void panDetectorModeChanged(int rx, int mode);
    void fftSizeChanged(int rx, int size);
    void fmsqLevelChanged(int rx, int level);


	void iqPortChanged(int rx, int port);

	void hamBandChanged(int rx, bool byButton, HamBand band);
	void dspModeChanged(int rx, DSPMode mode);
	void adcModeChanged(int rx, ADCMode mode);
	void agcModeChanged(int rx, AGCMode mode, bool hangEnabled);
	void agcHangEnabledChanged(int rx, bool hang);
	void agcGainChanged(int rx, int value);
	void agcThresholdChanged_dB(int rx, qreal value);
	void agcFixedGainChanged_dB(int rx, qreal value);
	void agcMaximumGainChanged_dB(int rx,  qreal value);
	void agcHangThresholdChanged(int rx, int value);
	void agcHangThresholdSliderChanged(int rx, qreal value);
	void agcHangLevelChanged_dB(int rx, qreal value);
	void agcLineLevelsChanged(int rx, qreal thresh, qreal hang);
	void agcVariableGainChanged_dB(int rx, qreal value);
	void agcAttackTimeChanged(int rx, qreal value);
	void agcDecayTimeChanged(int rx, qreal value);
	void agcHangTimeChanged(int rx, qreal value);
	void filterFrequenciesChanged(int rx, qreal low, qreal high);
	
	void freqRulerPositionChanged(int rx, float position);
	

	void audioFormatChanged(const QAudioFormat &format);
	void audioPositionChanged(qint64 position);
	void audioBufferChanged(qint64 position, qint64 length, const QByteArray &buffer);
	//void audioBufferChanged(const QByteArray &buffer);


	void displayWidgetHeightChanged(int value);
	void spectrumSizeChanged(int value);
	void panadapterColorChanged();
	void panGridStatusChanged(bool value, int rx);
	void peakHoldStatusChanged(bool value, int rx);
	void panLockedStatusChanged(bool value, int rx);
	void clickVFOStatusChanged(bool value, int rx);
	void hairCrossStatusChanged(bool value, int rx);
	void showAGCLinesStatusChanged(bool value, int rx);

	void spectrumAveragingChanged(int rx, bool value);
	void spectrumAveragingCntChanged(int rx, int value);
	void spectrumBufferChanged(int rx, const qVectorFloat& buffer);

	void waterfallTimeChanged(int rx, int value);
	void waterfallOffesetLoChanged(int rx, int value);
	void waterfallOffesetHiChanged(int rx, int value);

	void sMeterHoldTimeChanged(int value);
	void dBmScaleMinChanged(int rx, qreal value);
	void dBmScaleMaxChanged(int rx, qreal value);
    void noiseBlankerChanged(int rx, int mode);
	void noiseFilterChanged(int rx, int mode);
    void nr2GainMethodChanged(int rx, int value);
    void nr2NpeMethodChanged(int rx, int value);
    void nrAgcChanged(int rx, int value);
    void nr2AeChanged(int rx, bool value);
    void snbChanged(int rx, bool value);
    void anfChanged(int rx, bool value);
    void micInputLevelChanged(int level);
    void driveLevelChanged(int level);
	void repeaterModeChanged(bool mode);
    void txFullDuplexChanged(bool fullDuplex);
    void repeaterOffsetchanged(double value);
    void fmPremphasizechanged(double value);
    void phaseRotatorChanged(int value);
    void phaseRotatorAutoChanged(bool enabled);
    void phaseRotatorAutoResetRequested();
    void phaseRotatorStatusChanged(const QString &status);
    void ctcssToneHzChanged(int hz);
    void rxEqChanged();
    void txEqChanged();
    void cfcChanged();
    void emnrPost2Changed();
    void fmdeveationchanged(double value);
    void amCarrierlevelchanged(double level);
    void audioCompressionchanged(int level);
    void micModeChanged(bool mode);
    void showRadioPopupChanged(bool value);
	void receiverDataReady();

    void CwHangTimeChanged(int CwHangTime);
    void CwSidetoneFreqChanged(int CwSidetoneFreq);
    void CwKeyReversedChanged(int CwKeyReversed);
    void CwKeyerModeChanged(int CwKeyerMode);
    void InternalCwChanged(int InternalCW);
    void CwKeyerSpeedChanged(int CwKeyerSpeed);
    void CwPttDelayChanged(int CwPttDelay);
    void CwSidetoneVolumeChanged(int CwSidetoneVolume);
    void CwKeyerWeightChanged(int CwKeyerWeight);
    void CwKeyerSpacingChanged(int CwKeyerSpacing);


public:

    QAudioDevice m_inputDevice;
    QAudioDevice m_outputDevice;

    void	debugSystemState();
	int 	loadSettings();
	int 	saveSettings();
	/** Prefer settings.json; migrate from the INI sibling when JSON is missing or unreadable. */
	int 	loadPersistentSettings();
    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &root);
    bool saveJson(const QString &filePath = QString()) const;
    bool loadJson(const QString &filePath = QString());
    QString defaultJsonConfigPath() const;
	/** Point settings storage at an absolute .ini path (primarily for tests). */
	void    reopenSettingsStorage(const QString &absoluteIniPath);
	QSDR::_ServerMode			getCurrentServerMode();
	QSDR::_HWInterfaceMode		getHWInterface();
	QSDR::_DataEngineState		getDataEngineState();

	PanGraphicsMode				getPanadapterMode(int rx);
	WaterfallColorMode			getWaterfallColorMode(int rx);
	PanAveragingMode            getPanAveragingMode(int rx);
	PanDetectorMode             getPanDetectorMode(int rx);

	QString	getServerModeString(QSDR::_ServerMode mode);
	QString	getHWInterfaceModeString(QSDR::_HWInterfaceMode mode);
	QString	getHDataEngineStateString(QSDR::_DataEngineState mode);
	QString	getErrorString(QSDR::_Error err);

	QString getValue1000(double value, int valuePrefix, QString unitBase);
	QString getValue1024(double value, int valuePrefix, QString unitBase);

	THPSDRDevices 	getHPSDRDevices();

	bool getSettingsLoaded();
	bool getMainPower();
	bool getDefaultSkin();

	int getMinimumWidgetWidth();
	int getMinimumGroupBoxWidth();
	int getMultiRxView();
	bool getPBOPresence();
	bool getFBOPresence();

	bool getConnected();
	bool getClientConnected();
	bool getTxAllowed();
    QString appStyleSheet;

    QString get_appStyleSheet(){
        return appStyleSheet;
    }
	QString getTitleStr();
	QString getVersionStr();
	QString getSettingsFilename();
	QString getCallsign();



	QString getServerAddr();
	QString getHPSDRDeviceLocalAddr();

	quint16 getServerPort();
	quint16 getListenPort();
	quint16 getAudioPort();
	quint16	getMetisPort();

	TNetworkDevicecard			getCurrentMetisCard()		{ QReadLocker locker(&m_dataRwLock); return m_currentHPSDRDevice; }
    TSDRDevice                  getLastConnectedDevice()    { QReadLocker locker(&m_dataRwLock); return m_lastConnectedDevice; }
	QList<TNetworkDevicecard>	getMetisCardsList()			{ QReadLocker locker(&m_dataRwLock); return m_metisCards; }
    TSoapyDevice                getCurrentSoapyDevice()     { return m_soapyConfig->currentDevice(); }
    QList<TSoapyDevice>         getSoapyDeviceList()        { return m_soapyConfig->deviceList(); }
    QString     getSoapyRxAntenna()     const { return m_soapyConfig->rxAntenna(); }
    QStringList getSoapyAntennaList()   const { return m_soapyConfig->antennaList(); }
    QString     getSoapyTxAntenna()     const { return m_soapyConfig->txAntenna(); }
    QStringList getSoapyTxAntennaList() const { return m_soapyConfig->txAntennaList(); }
    QString     getSoapyHardwareKey()   const { return m_soapyConfig->hardwareKey(); }
    int         getSoapyLnaGain()       const { return m_soapyConfig->lnaGain(); }
    int         getSoapyTiaGain()       const { return m_soapyConfig->tiaGain(); }
    int         getSoapyPgaGain()       const { return m_soapyConfig->pgaGain(); }
    int         getSoapyOverallGain()   const { return m_soapyConfig->overallGain(); }
    bool        getSoapyAutoCalibrate() const { return m_soapyConfig->autoCalibrate(); }
    bool        getSoapyIQBalance()     const { return m_soapyConfig->iqBalance(); }
	qint64						getMaxFrequency()			{ return m_maxFrequency.load(); }
	qint64						getMinFrequency()			{ return m_minFrequency.load(); }
    int         getSoapyOverallGainMin() const { return m_soapyConfig->overallGainMin(); }
    int         getSoapyOverallGainMax() const { return m_soapyConfig->overallGainMax(); }
	QList<TReceiver>			getReceiverDataList()		{ QReadLocker locker(&m_dataRwLock); return m_receiverDataList; }
	QList<THamBandFrequencies>	getBandFrequencyList()		{ QReadLocker locker(&m_dataRwLock); return m_bandList; }
	QList<THamBandText>			getHamBandTextList()		{ QReadLocker locker(&m_dataRwLock); return m_bandTextList; }
	QList<TDefaultFilter>		getDefaultFilterList()		{ QReadLocker locker(&m_dataRwLock); return m_defaultFilterList; }
	TDefaultFilterMode			getCurrentFilterMode()		{ return m_filterMode; }
	quint16						getAlexConfig()				{ return m_alexConfigObj->alexConfig(); }
	QList<int>					getAlexStates()				{ return m_alexConfigObj->alexStates(); }
	QList<long>					getHPFLoFrequencies()		{ return m_alexConfigObj->hpfLoFrequencies(); }
	QList<long>					getHPFHiFrequencies()		{ return m_alexConfigObj->hpfHiFrequencies(); }
	QList<long>					getLPFLoFrequencies()		{ return m_alexConfigObj->lpfLoFrequencies(); }
	QList<long>					getLPFHiFrequencies()		{ return m_alexConfigObj->lpfHiFrequencies(); }
	QList<int>					getRxJ6Pins()				{ QReadLocker locker(&m_dataRwLock); return m_rxJ6pinList; }
	QList<int>					getTxJ6Pins()				{ QReadLocker locker(&m_dataRwLock); return m_txJ6pinList; }
    int                         get_tx_drivelevel()         { return m_transmitConfig->driveLevel(); }
    bool                        get_repeaterMode()          {return m_repeaterMode; }
    bool                        getTxFullDuplex() const     { return m_transmitConfig->txFullDuplex(); }
    int							getFramesPerSecond(int rx);
	QString						getDSPModeString(int mode);
    DSPMode                     getDSPMode(int rx);
	int                         getFreeDVMode(int rx);
	QString                     getCodec2ModeString(int mode);
	QList<int>                  availableCodec2Modes();

	HamBand						getCurrentHamBand(int rx);
	QList<int>					getMercuryAttenuators(int rx);
	QSDR::_DSPCore				getReceiverDspCore(int rx) const;
	QList<DSPMode>				getDSPModeList(int rx) const;
	QList<qint64>				getLastCenterFrequencyList(int rx) const;
	QList<qint64>				getLastVfoFrequencyList(int rx) const;
	qreal						getdBmPanScaleMin(int rx, HamBand band) const;
	qreal						getdBmPanScaleMax(int rx, HamBand band) const;
	float						getFreqRulerPosition(int rx) const;
	qreal						getFilterLo(int rx) const;
	qreal						getFilterHi(int rx) const;
	TDefaultFilterMode			getDefaultFilterMode(int rx) const;
	qreal						getAGCAttackTime(int rx) const;
	qreal						getAGCDecayTime(int rx) const;
	qreal						getAGCHangTime(int rx) const;
	bool						getHangEnabled(int rx) const;
	bool						getAgcLines(int rx) const;
	int							getWaterfallOffsetLo(int rx) const;
	int							getWaterfallOffsetHi(int rx) const;
	//int getMercuryAttenuator();

	bool getPennyOCEnabled()		{ return m_pennyOCEnabled; }
	int	 getHpsdrNetworkDevices()	{ return m_hpsdrNetworkDevices; }
	int	 getNetworkInterfacesNo()	{ return m_NetworkInterfacesNo; }
	bool getMercuryPresence()		{ return m_devices.mercuryPresence; }
	int	 getMercuryVersion()		{ return m_devices.mercuryFWVersion; }
	bool getPenelopePresence()		{ return m_devices.penelopePresence; }
	int  getPenelopeVersion()		{ return m_devices.penelopeFWVersion; }
	bool getPennyLanePresence()		{ return m_devices.pennylanePresence; }
	int  getPennyLaneVersion()		{ return m_devices.pennylaneFWVersion; }
	bool getHermesPresence()		{ return m_devices.hermesPresence; }
	int  getHermesVersion()			{ return m_devices.hermesFWVersion; }
	int	 getHPSDRHardware()			{ return m_hpsdrHardware; }
	bool getAlexPresence()			{ return m_devices.alexPresence; }
	bool getExcaliburPresence()		{ return m_devices.excaliburPresence; }
	bool getMetisPresence()			{ return m_devices.metisPresence; }
	int  getMetisVersion()			{ return m_devices.metisFWVersion; }
	int  getSocketBufferSize()		{ return m_networkConfig->socketBufferSize(); }
	bool getManualSocketBufferSize() { return m_manualSocketBufferSize; }
	bool getFirmwareVersionCheck()	{ return m_checkFirmwareVersions; }

	// wideband data & options
	TWideband	getWidebandOptions()		{ return m_widebandOptions; }

	bool		getWidebandStatus()			{ return m_widebandOptions.wideBandDisplayStatus; }
	bool		getWidebandData()			{ return m_widebandOptions.wideBandData; }
	qreal		getWidebanddBmScaleMin()	{ return m_widebandOptions.dBmWBScaleMin; }
	qreal		getWidebanddBmScaleMax()	{ return m_widebandOptions.dBmWBScaleMax; }
	int			getWidebandBuffers()		{ return m_widebandOptions.numberOfBuffers; }



	bool getPanGridStatus(int rx);
	bool getPeakHoldStatus(int rx);
	bool getPanLockedStatus(int rx);
	bool getClickVFOStatus(int rx);
	bool getHairCrossStatus(int rx);

	int		getMercurySpeed()			{ return m_mercurySpeed; }
	int		getOutputSampleIncrement()	{ return m_outputSampleIncrement; }
	int		getNumberOfReceivers()		{ return m_mercuryReceivers; }
    int     getCurrentReceivers()		{ return m_mercuryReceivers; }
	int		getCurrentReceiver()		{ return m_currentReceiver; }
	bool	getFrequencyRx1onRx2()		{ return m_frequencyRx1onRx2; }
	int		getSampleRate()				{ return m_sampleRate; }

	//int getMercuryAttenuator()		{ return m_mercuryAttenuator; }
    int     getMercuryDither()			{ return m_mercuryDither; }
    int     getMercuryRandom()			{ return m_mercuryRandom; }
    int     get10MHzSource()			{ return m_hardwareConfig->source10Mhz(); }
    int     get122_8MHzSource()			{ return m_hardwareConfig->source122_88Mhz(); }
    int     getMicSource()				{ return m_transmitConfig->micSource(); }
    int     getRxClass()				{ return m_RxClass; }
    int     getRxTiming()				{ return m_RxTiming; }
    int     getMicInputDev()            { return m_transmitConfig->micInputDev(); }
    int     getDigitalAudioInputDev()   { return m_transmitConfig->digitalAudioInputDev(); }
	QString getMicInputSourceName()     { return m_transmitConfig->micInputSourceName(); }
	QString getDigitalInputSourceName() { return m_transmitConfig->digitalInputSourceName(); }
    int     getMicInputLevel()          { return static_cast<int>(m_transmitConfig->micGain()); }
    int     getDriveLevel()             { return m_transmitConfig->driveLevel(); }
    bool    getRepeaterMode()           { return m_repeaterMode; }
    double  getRepeaterOffset()         { return m_transmitConfig->repeaterOffset(); }
    double  getFMpreemphesis() const;
    int     getPhaseRotator() const;
    bool    getPhaseRotatorAuto() const;
    int     getCtcssToneHz() const;
    bool    getRxEqEnabled()            { return m_audioConfig->rxEqEnabled(); }
    QVector<int> getRxEqBands()         { return m_audioConfig->rxEqBands(); }
    int     getRxEqCurveDeg()           { return m_audioConfig->rxEqCurveDeg(); }
    bool    getTxEqEnabled() const;
    QVector<int> getTxEqBands() const;
    int     getTxEqCurveDeg() const;
    bool    getCfcEnabled() const;
    bool    getCfcPeqEnabled() const;
    double  getCfcPrecomp() const;
    double  getCfcPrePeq() const;
    int     getCfcCurveDeg() const;
    QVector<double> getCfcFreqs()       { return m_transmitConfig->cfcFreqs(); }
    QVector<double> getCfcLevels() const;
    QVector<double> getCfcPost() const;
    bool    getEmnrPost2Enabled()       { return m_audioConfig->emnrPost2Enabled(); }
    double  getEmnrPost2Factor()        { return m_audioConfig->emnrPost2Factor(); }
    double  getEmnrPost2Nlevel()        { return m_audioConfig->emnrPost2Nlevel(); }
    double  getEmnrPost2Taper()         { return m_audioConfig->emnrPost2Taper(); }
    double  getEmnrPost2Rate()          { return m_audioConfig->emnrPost2Rate(); }
    double  getFMDeveation() const;
    double  getAMCarrierLevel() const;
    double  getAudioCompression() const;

	qreal	getMainVolume(int rx);
	qreal	getMouseWheelFreqStep(int rx);// { return m_mouseWheelFreqStep; }
	ADCMode getADCMode(int rx);
	AGCMode getAGCMode(int rx);
	QString getADCModeString(int rx);
	QString getAGCModeString(int rx);
    qreal   getAGCGain(int rx);
    int     getAGCMaximumGain_dB(int rx);
	qreal	getAGCFixedGain_dB(int rx);
	int		getAGCHangThreshold(int rx);
	int		getAGCHangLeveldB(int rx);
    int     getAGCSlope(int rx);
    int 	getfftSize(int rx);
    int     getNrAGC(int rx);
    int     getNr2GainMethod(int rx);
    int     getNr2NpeMethod(int rx);
    bool    getNr2ae(int rx);
    bool    getSnb(int rx);
    bool    getAnf(int rx);
    bool    getCwDecode(int rx);
    int     getnbMode(int rx);
    int     getnrMode(int rx);





	
	int		getSpectrumSize()			{ return m_displayConfig->spectrumSize(); }
	
	qreal	getdBmDistScaleMin()		{ return m_displayConfig->dBmDistScaleMin(); }
	qreal	getdBmDistScaleMax()		{ return m_displayConfig->dBmDistScaleMax(); }

	int		getSMeterHoldTime()			{ return m_displayConfig->sMeterHoldTime(); }

	qreal	getFilterFrequencyLow()		{ return m_filterFrequencyLow; }
	qreal	getFilterFrequencyHigh()	{ return m_filterFrequencyHigh; }

	QList<QHostAddress>				m_ipAddressesList;
	QList<QNetworkInterface>		m_networkInterfaces;
	
	// audio
//	QAudio::Mode mode() const			{ return m_audioMode; }
    QAudio::State state() const			{ return m_audioState; }
	QAudioFormat getAudioFormat() const { return m_format; }

	// colors
	TPanadapterColors getPanadapterColors();

	bool getSpectrumAveraging(int rx);
	int getSpectrumAveragingCnt(int rx);
	int getFFTMultiplicator(int rx);//			{ return m_fft; }
    QStringList getFilterBtnText(int rx);

    QMutex debugMutex;

    void    getConfigPath();
    QString cfg_dir;

public slots:
	void	setMainPower(bool power);
	void	setDefaultSkin(bool value);
	void	setSettingsFilename(QString filename);

	void	setSystemMessage(const QString &msg, int time);
	void	setSettingsLoaded(bool loaded);
	void	setCPULoad(short load);
	void	setCallsign(const QString &callsign);

	void	setPBOPresence(bool value);
	void	setFBOPresence(bool value);

	void	setMainVolume(int rx, float volume);
	void	setMainVolumeMute(int rx, bool value);

	void	setSystemState(
				QSDR::_Error err, 
				QSDR::_HWInterfaceMode hwmode, 
				QSDR::_ServerMode mode, 
				QSDR::_DataEngineState state);

	void	setGraphicsState(
				int rx,
				PanGraphicsMode panMode,
				WaterfallColorMode waterfallColorMode);

	void setTxAllowed(bool value);

    RadioState setRadioState(RadioState mode);
	RadioState getRadioState() { return m_radioState;}

    void        setRigCtlServer(RigCtlServer *server) { m_rigCtlServer = server; }
    RigCtlServer *rigCtlServer() const { return m_rigCtlServer; }
    void        setTciServer(TciServer *server) { m_tciServer = server; }
    TciServer  *tciServer() const { return m_tciServer; }
    // TCI WebSocket server enable/disable (persisted). Emits
    // tciServerEnabledChanged so the server can be started/stopped at runtime.
    bool        getTciServerEnabled() const { return m_tciConfig->serverEnabled(); }
    void        setTciServerEnabled(bool enabled);
    float       getTciRxGain() const { return m_tciConfig->rxGain(); }
    void        setTciRxGain(float gain);
    float       getTciTxGain() const { return m_tciConfig->txGain(); }
    void        setTciTxGain(float gain);
    // Lock-free hint set by TciServer when any client is subscribed to the IQ
    // stream. The DSP thread reads it to skip building/emitting per-block IQ
    // when nobody is listening (avoids the interleave alloc + copy each block).
    void        setTciIqActive(bool active) { m_tciIqActive.store(active, std::memory_order_relaxed); }
    bool        tciIqActive() const { return m_tciIqActive.load(std::memory_order_relaxed); }
	void setMultiRxView(int view);
	void setFreeDVStatus(int rx, bool sync, float snr, quint64 rxFrames);
	void setSpectrumBuffer(int rx, const qVectorFloat& buffer);
	void addFreeDVTxFrames(int rx, quint64 txFrames);
	void setFreeDVMode(int rx, int mode);
	void setSampleSize(int rx, int size);
    void setRxList (QList<SliceProcessor*> list);
	void setMetisCardList(QList<TNetworkDevicecard> list);
	void searchHpsdrNetworkDevices();
#ifdef HAVE_SOAPYSDR
    void searchSoapyDevices();
#endif
    void searchDevices();
	void clearMetisCardList();
	void setHPSDRDeviceNumber(int value);
	void setCurrentHPSDRDevice(TNetworkDevicecard card);
    void setSoapyDeviceList(QList<TSoapyDevice> list);
    void setCurrentSoapyDevice(TSoapyDevice device);
    void setSoapyMessage(QString message);
    // SoapySDR radio parameter setters
    void setSoapyRxAntenna(const QString &antenna);
    void setSoapyAntennaList(const QStringList &list);  // runtime, from device
    void setSoapyTxAntenna(const QString &antenna);
    void setSoapyTxAntennaList(const QStringList &list);  // runtime, from device
    void setSoapyHardwareKey(const QString &key);       // runtime, from device
    void setSoapyLnaGain(int gain);
    void setSoapyTiaGain(int gain);
    void setSoapyPgaGain(int gain);
    void setSoapyOverallGain(int gain);
    void setSoapyOverallGainRange(int minGain, int maxGain);
    void setSoapyAutoCalibrate(bool enabled);
    void setSoapyIQBalance(bool enabled);
	void addNetworkIOComboBoxEntry(QString str);
	void clearNetworkIOComboBoxEntry();
	void addServerNetworkInterface(QString nicName, QString ipAddress);
	void addHPSDRDeviceNIC(QString nicName, QString ipAddress);
	void setNumberOfNetworkInterfaces(int value);
	void setServerNetworkInterface(int index);
	void setHPSDRDeviceNIC(int index);
	void setServerWidgetNIC(int index);
	void setHPSDRWidgetNIC(int index);
	void setServerAddr(QString addr);
	void setHPSDRDeviceLocalAddr(QString addr);
	void setServerPort(quint16 port);
	void setListenPort(quint16 port);
	void setAudioPort(quint16 port);
	void setMetisPort(quint16 port);
	void setClientConnected(bool value);
	void setClientNoConnected(int client);
	void setRxConnectedStatus(int rx, bool value);
	void setAudioRx(int rx);
	void setConnected(bool value);
	void setCheckFirmwareVersion(bool value);

	void setHPSDRDevices(THPSDRDevices devices);
	//void setHermesPresence(bool value);
	void setHermesVersion(int value);
	void setHPSDRHardware(int value);
	void setMercuryPresence(bool value);
	void setMercuryVersion(int value);
	void setPenelopePresence(bool value);
	void setPenelopeVersion(int value);
	void setPennyLanePresence(bool value);
	void setPennyLaneVersion(int value);
	void setAlexPresence(bool value);
	void setExcaliburPresence(bool value);
	void setMetisVersion(int value);

	//void setAlexConfiguration(const QList<TAlexConfiguration> &conf);
	void setAlexConfiguration(quint16 conf);
	void setAlexHPFLoFrequencies(int filter, long value);
	void setAlexHPFHiFrequencies(int filter, long value);
	void setAlexLPFLoFrequencies(int filter, long value);
	void setAlexLPFHiFrequencies(int filter, long value);
	void setAlexStates(const QList<int> &states);
	void setAlexState(int pos, int value);
	void setAlexState(int value);
	void setAlexToManual(bool value);
	int checkAlexState(int state);

	void setPennyOCEnabled(bool value);
	void setRxJ6Pin(HamBand band, int value);
	void setRxJ6Pins(const QList<int> &states);
	void setTxJ6Pin(HamBand band, int value);
	void setTxJ6Pins(const QList<int> &states);

	void setIQPort(int rx, int port);


	void setReceivers(int value);
	//void setReceiver(int value);
	void setCurrentReceiver(int value);
	void setSampleRate(int value);
	void setMercuryAttenuator(int value);
	void setDither(int value);
	void setRandom(int value);
	void set10MhzSource(int source);
	void set122_88MhzSource(int source);
    void setMicSource(int source);
    void setMicInputDev(int index);
	void setMicInputSourceName(const QString &name);
	void setDigitalAudioInputDev(int index);
	void setDigitalInputSourceName(const QString &name);
    void setMicInputLevel(int level);
    void setDriveLevel(int level);
	void setClass(int value);
	void setTiming(int value);
	void setCtrFrequency(int mode, int rx, qint64 frequency);
	void setCtrFrequency(int rx, qint64 frequency);
	qint64 getCtrFrequency(int rx);
	void setMaxFrequency(qint64 value);
	void setMinFrequency(qint64 value);
	void setVFOFrequency(int mode, int rx, qint64 frequency);
	void setVfoFrequency(int rx, qint64 frequency);
	/** Retune the dial, recentring the panadapter only when the target would fall
	 *  outside the displayed span (VFO A/B switches, memory recalls). */
	void setVfoFrequencyVisible(int rx, qint64 frequency);
	qint64 getVfoFrequency(int rx);
	void setNCOFrequency(bool value, int rx, qint64 frequency);
		
	void clientDisconnected(int client);
	void setFramesPerSecond(int rx, int value);
	void setMouseWheelFreqStep(int rx, qreal value);
	void setSocketBufferSize(int value);
	void setManualSocketBufferSize(bool value);
	
	void setReceiverDataReady();

	void setSpectrumSize(int value);
	void setdBmPanScaleMin(int rx, qreal value);
	void setdBmPanScaleMax(int rx, qreal value);
		
	void setdBmDistScaleMin(qreal value);
	void setdBmDistScaleMax(qreal value);
	
	void setHamBand(int rx, bool byButton, HamBand band);
	void setDSPMode(int rx, DSPMode mode);
	void setADCMode(int rx, ADCMode mode);
	void setAGCMode(int rx, AGCMode mode);
	void setAGCGain(int rx, int value);
	void setAGCMaximumGain_dB(int rx, qreal value);
	void setAGCFixedGain_dB(int rx, qreal value);
	void setAGCThreshold_dB(int rx, qreal value);

	void setAGCHangThresholdSlider(int rx, qreal value);
	void setAGCHangThreshold(int rx, int value);
	void setAGCHangLevel_dB(int rx, qreal value);
	void setAGCLineLevels(int rx, qreal thresh, qreal hang);
	void setAGCShowLines(int rx, bool value);
	void setAGCVariableGain_dB(int rx, qreal value);
	void setAGCAttackTime(int rx, qreal value);
	void setAGCDecayTime(int rx, qreal value);
	void setAGCHangTime(int rx, qreal value);
	void setRXFilter(int rx, qreal low, qreal high);

	void setfftSize(int rx, int size);
	void setfmsqLevel(int rx, int level);

	// wideband data & options
	void setWidebandBuffers(int value);
	void setWidebandOptions(TWideband options);
	void setWidebandStatus(bool value);
	void setWidebandData(bool value);
	void setWidebanddBmScaleMin(qreal value);
	void setWidebanddBmScaleMax(qreal value);
	//void setWidebandAveraging(bool value);
	//void setWidebandAveragingCnt(int value);
	void setWideBandRulerPosition(float pos);

	void setFreqRulerPosition(int rx, float pos);
	//void setRulerPosition(float pos);


	void setAudioFormat(const QAudioFormat &format);
	void setAudioPosition(qint64 position);
	void setAudioBuffer(qint64 position, qint64 length, const QByteArray &buffer);
	//void setAudioBuffer(const QByteArray &buffer);


	void moveDisplayWidget(int value);

	void setPanadapterColors(TPanadapterColors type);
	void setPanGrid(bool value, int rx);
	void setPeakHold(bool value, int rx);
	void setPanLocked(bool value, int rx);
	void setClickVFO(bool value, int rx);
	void setHairCross(bool value, int rx);
	
	void setSpectrumAveraging(int rx, bool value);
	void setSpectrumAveragingCnt(int rx, int value);
	
/* Waterfall */
	void setWaterfallTime(int rx, int value);
	void setWaterfallOffesetLo(int rx, int value);
	void setWaterfallOffesetHi(int rx, int value);
	void setPanAveragingMode(int rx,PanAveragingMode mode);
    void setPanDetectorMode(int rx,PanDetectorMode mode);
    /*Noiseblanker*/
    void setNoiseBlankerMode(int rx, int nb);
	void setNoiseFilterMode(int rx, int nr);
	void setNR2GainMethod(int rx, int value);
    void setNR2NpeMethod(int rx, int value);
    void setNRAgc(int rx, int value);
    void setNR2Ae(int rx, bool value);
    void setAnf(int rx, bool value);
    void setSnb(int rx, bool value);
    void setCwDecode(int rx, bool value);
    void setRepeaterMode(bool mode);
    void setTxFullDuplex(bool fullDuplex);
    void setRepeaterOffset(int offset);
    void setAudioCompression(int level);
    void setAMCarrierLevel(int level);
    void setFMPreEmphasize(int level);
    void setPhaseRotator(int level);
    void setPhaseRotatorAuto(bool enabled);
    void requestPhaseRotatorAutoReset();
    void setPhaseRotatorStatus(const QString &status);
    void setCtcssToneHz(int hz);
    void setRxEqEnabled(bool enabled);
    void setRxEqBands(const QVector<int> &bands);
    void setRxEqBand(int index, int gainDb);
    void setRxEqCurveDeg(int deg);
    void setTxEqEnabled(bool enabled);
    void setTxEqBands(const QVector<int> &bands);
    void setTxEqBand(int index, int gainDb);
    void setTxEqCurveDeg(int deg);
    void setCfcEnabled(bool enabled);
    void setCfcPeqEnabled(bool enabled);
    void setCfcPrecomp(double db);
    void setCfcPrePeq(double db);
    void setCfcCurveDeg(int deg);
    void setCfcLevel(int index, double db);
    void setCfcPostBand(int index, double db);
    void setEmnrPost2Enabled(bool enabled);
    void setEmnrPost2Factor(double pct);
    void setEmnrPost2Nlevel(double pct);
    void setEmnrPost2Taper(double pct);
    void setEmnrPost2Rate(double seconds);
    void setFmDeveation(int level);

    void setCwHangTime(int CwHangTime);
    void setCwSidetoneFreq(int CwSidetoneFreq);
    void setCwKeyReversed(int CwKeyReversed);
    void setCwKeyerMode(int CwKeyerMode);
    void setInternalCw(int InternalCW);
    void setCwKeyerSpeed(int CwKeyerSpeed);
    void setCwPttDelay(int CwPttDelay);
    void setCwSidetoneVolume(int CwSidetoneVolume);
    void setCwKeyerWeight(int CwKeyerWeight);
    void setCwKeyerSpacing(int CwKeyerSpacing);



    void setSMeterHoldTime(int value);

	void showNetworkIODialog();
	void showWarningDialog(const QString &msg);

	void showRadioPopupWidget();

	QList<qint64> getCtrFrequencies();
	QList<qint64> getVfoFrequencies();


public:
    bool isInternalCw() const;
    int getCwKeyerSpeed() const;
    int getCwKeyerMode() const;
    int isCwKeyReversed() const;
    int getCwSidetoneFreq() const;
    int getCwSidetoneVolume() const;
    int getCwPttDelay() const;
    int getCwHangTime() const;
    int getCwKeyerWeight() const;
    int getCwKeyerSpacing() const;



    bool is_transmitting(){
        if (m_radioState > 0) return true;
        else return false;
    }
    RadioModel* m_radioModel = nullptr;


private slots:

private:
    mutable QReadWriteLock      m_dataRwLock;

	std::atomic<QSDR::_Error>				m_systemError{QSDR::NoError};
	std::atomic<QSDR::_ServerMode>			m_serverMode{QSDR::SDRMode};
	std::atomic<QSDR::_HWInterfaceMode>		m_hwInterface{QSDR::NoInterfaceMode};
	std::atomic<QSDR::_DataEngineState>		m_dataEngineState{QSDR::DataEngineDown};

//	QAudio::Mode	m_audioMode;
    QAudio::State	m_audioState;
	QAudioFormat    m_format;

	THPSDRDevices				m_devices;
	TDefaultFilterMode			m_filterMode;
	TNetworkDevicecard			m_currentHPSDRDevice;
    TSDRDevice                  m_lastConnectedDevice;
	TTransmitter				m_transmitter;
	TWideband					m_widebandOptions;

	QList<TNetworkDevicecard>	m_metisCards;
	QList<TReceiver>			m_receiverDataList;
	QList<TReceiver>			pam_receiverDataList;
	QList<THamBandFrequencies>	m_bandList;
	QList<THamBandText>			m_bandTextList;
	QList<TDefaultFilter>		m_defaultFilterList;
	//QList<QCLDevice>			m_clDevices;
	QList<QString>				m_rxStringList;
	QList<int>					m_rxJ6pinList;
	QList<int>					m_txJ6pinList;

	QString			m_titleString;
	QString			m_versionString;
	QString			m_callsignString;
	QString			settingsFilename;

	QDateTime		startTime;
	QDateTime		now;

	QHostAddress	m_hostAddress;


	bool	setLoaded;

	std::atomic<bool>	m_mainPower{false};
    std::atomic<RadioState> m_radioState{RadioState::RX};
    RigCtlServer *m_rigCtlServer = nullptr;
    TciServer    *m_tciServer = nullptr;
    std::atomic<bool>          m_tciIqActive{false};
	std::atomic<bool>	m_defaultSkin{true};
	std::atomic<bool>	m_connected{false};
	std::atomic<bool>	m_clientConnected{false};
	std::atomic<bool>	m_pboFound{false};
	std::atomic<bool>	m_fboFound{false};
	std::atomic<bool>	m_manualSocketBufferSize{false};
	std::atomic<bool>	m_pennyOCEnabled{false};

	//bool	main_mute;
	std::atomic<bool>	m_checkFirmwareVersions{true};
	std::atomic<bool>	m_specAveraging{false};
	std::atomic<bool>	m_panGrid{true};
	std::atomic<bool>	m_peakHold{false};
	std::atomic<bool>	m_packetsToggle{true};

	std::atomic<bool>	m_frequencyRx1onRx2{false};
	std::atomic<bool>	m_radioPopupVisible{false};

	std::atomic<qint64>	m_maxFrequency{MAXFREQUENCY};
	std::atomic<qint64>	m_minFrequency{0};

	std::atomic<int>		m_hpsdrHardware{0};
	std::atomic<int>		m_hpsdrNetworkDevices{0};
	std::atomic<int>		m_NetworkInterfacesNo{0};
	std::atomic<int>		m_clientNoConnected{0};

	std::atomic<int>		m_mercuryReceivers{1};
    std::atomic<int>		m_currentReceiver{0};
	std::atomic<int>		m_sampleRate{48000};
	std::atomic<int>		m_mercurySpeed{0};

	std::atomic<int>		m_mercuryAttenuator{0};
	std::atomic<int>		m_mercuryDither{0};
	std::atomic<int>		m_mercuryRandom{0};

	std::atomic<int>		m_outputSampleIncrement{0};
	std::atomic<int>		m_RxClass{0};
	std::atomic<int>		m_RxTiming{0};

	std::atomic<int>		m_framesPerSecond{15};

	QList<bool>			m_freeDVSyncList;
	QList<float>		m_freeDVSnrList;
	QList<quint64>		m_freeDVRxFramesList;
	QList<quint64>		m_freeDVTxFramesList;

	//int		m_wbBuffers;
    std::atomic<bool>    m_repeaterMode{false};

	long freq1;
	
	int control_register;
	bool connect_at_startup;

	qreal	m_filterFrequencyLow;
	qreal	m_filterFrequencyHigh;


    bool    m_use_repeaterOffset;


	//int		m_fft;

	void	checkHPSDRDevices();
    qreal   getRxFilterBandwidth(int rx, int index);
};


//******************************************************
// Macros

/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** The following Macro "CHECKED_CONNECT" is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/
// Macro which connects a signal to a slot, and which causes application to
// abort if the connection fails.  This is intended to catch programming errors
// such as mis-typing a signal or slot name.  It is necessary to write our own
// macro to do this - the following idiom
//     Q_ASSERT(connect(source, signal, receiver, slot));
// will not work because Q_ASSERT compiles to a no-op in release builds.

#define CHECKED_CONNECT(source, signal, receiver, slot) \
    if(!connect(source, signal, receiver, slot)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);

#define CHECKED_CONNECT_OPT(source, signal, receiver, slot, opt) \
    if(!connect(source, signal, receiver, slot, opt)) \
        qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, __LINE__);



///******************************************************
// Debug output

class NullDebug {

public:
	template <typename T>
	NullDebug& operator << (const T) { return *this; }
};

inline NullDebug nullDebug() { return NullDebug(); }


#ifdef LOG_SETTINGS
#   define SETTINGS_DEBUG qDebug().nospace() << "Settings::\t"
#else
#   define SETTINGS_DEBUG nullDebug()
#endif

#endif  // CUSDR_SETTINGS_H
