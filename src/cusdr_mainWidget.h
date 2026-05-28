#ifndef CUSDR_MAIN_H
#define CUSDR_MAIN_H

#include <QProcess>
#include <QActionGroup>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QSlider>
#include <QTableWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QVariant>
#include <QTimer>
#include <QComboBox>
#include <QPainter>

#include "cusdr_settings.h"
#include "cusdr_fonts.h"
#include "Util/cusdr_buttons.h"
#include "DataEngine/cusdr_dataEngine.h"
#include "UI/cusdr_setupwidget.h"
#include "cusdr_displayTabWidget.h"
#include "cusdr_serverWidget.h"
#include "GL/cusdr_oglWidebandPanel.h"
#include "GL/cusdr_oglReceiverPanel.h"
#include "GL/cusdr_oglDisplayPanel.h"
#include "GL/cusdr_ogl3DPanel.h"
#include "setupwidget.h"
#include "tx_settings_dialog.h"

#ifdef LOG_MAIN
#define MAIN_DEBUG qDebug().nospace() << "MainWindow::\t"
#else
#define MAIN_DEBUG nullDebug()
#endif

class WarningDialog;
class MainWindowUI;

class RadioModel;
class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    RadioModel* radioModel() const { return m_radioModel; }
    
    explicit MainWindow(RadioModel *model, QWidget *parent = nullptr);
    ~MainWindow() override;

	void	setup();

public slots:
	void	update();
	void	masterSwitchChanged(bool power);
    void systemStateChanged(
            QSDR::_Error err,
            QSDR::_HWInterfaceMode hwmode,
            QSDR::_ServerMode mode,
            QSDR::_DataEngineState state);
	void	startButtonClickedEvent();
	void	widgetBtnClickedEvent();
	void	wideBandBtnClickedEvent();
	void    radioStateChange(RadioState state);
	void	alexBtnClickedEvent();
	void	muteBtnClickedEvent();
	void    moxBtnClickedEvent();
	void    tunBtnClickedEvent();
	
	void	showWidgetEvent();
	void	closeWidgetEvent();

	void	suspendSignal();
	void	showWarningDialog(const QString &msg);

    void ctrlDisplayBtnClickedEvent();
	void closeMainWindow();
	void maximizeMainWindow();
	void setMainWindowGeometry();
	void updateTitle();
	void updateStatusBar(short load);
	void setFullScreen();
	void getRegion();
    void cusdr_setup();

	void setServerMode(QSDR::_ServerMode mode);
	void setTxAllowed(bool value);
	void setCurrentReceiver(int rx);
	void setNumberOfReceivers(int value);
	void setSDRMode(bool);

	void getNetworkInterfaces();
	void setMainVolume(int value);
    void setMicLevel(int value);
    void setDriveLevel(int value);

    void setADCMode(int rx, ADCMode mode);
	void setAGCMode(int rx, AGCMode mode, bool hang);
	void setAGCGain(int value);
	void setAGCGain(int rx, qreal value);
	void getLastFrequency();
	void addReceiver();
	void setAttenuator();
	void showNetworkIODialog();
	void showRadioPopup(bool value);
	void mercuryAttenuatorChanged(HamBand band, int value);
    void handleDeviceListChanged(const QList<TNetworkDevicecard> &list);
#ifdef HAVE_SOAPYSDR
    void handleSoapyDeviceListChanged(const QList<TSoapyDevice> &list);
#endif
    void clearDiscoveredDevices();
    void processDiscoveryResults();
	void alexPresenceChanged(bool value);
	void alexConfigurationChanged(quint16 value);
	void alexStateChanged(HamBand band, const QList<int> &states);
	void widebandVisibilityChanged(bool value);
	void showStatusBarMessage(const QString &msg, int timeout);
	void clearStatusBarMessage();
	void showAboutDialog();

private:
	void	setSystemState(
				QSDR::_Error err,
				QSDR::_HWInterfaceMode hwmode,
				QSDR::_ServerMode mode,
				QSDR::_DataEngineState state);

#if defined(Q_OS_WIN32)
#endif
	void	setupConnections();
	void	setupLayout();
	
	void	createReceiverPanels(int rx);
	void	updateFromSettings();
	void	setAttenuatorButton();
    void    setupActions();
    void    checkStartButtonState();


private:
    MainWindowUI*               ui;
	Settings*					set;
    QDialog                     *setupWidget;

	QSDR::_Error				m_error;
	QSDR::_ServerMode			m_serverMode;
	QSDR::_HWInterfaceMode		m_hwInterface;
	QSDR::_DataEngineState		m_dataEngineState;

	QMutex						m_mutex;

	QDir 						m_currentDir;
	
	QMainWindow*				centralwidget;

	QList<QGLReceiverPanel* >	rxWidgetList;
	QVector<float>				rxVolumeList;

	QDockWidget*				widebandDock;
	QDockWidget*				m_3DPanDock = nullptr;
	QDockWidget*				rx1Dock;
    QDockWidget*				rxDock;
	QList<QDockWidget* >		dockWidgetList;
	QList<QDockWidget* >		rxDockWidgetList;
    QMenuBar*                   menuBar;
    QMenu *                     File;
	QMenu *                     Help;
    QAction                     *test;
	
	QList<QHostAddress>			m_ipList;
	QList<QNetworkInterface>	m_niList;

	CFonts*						fonts;
	TFonts						m_fonts;

	QSize			m_oldSize;
	QPoint			m_oldPosition;

	QPixmap			m_originalPixmap;
	QPixmap			m_widgetMask;

    QTimer*			m_resizeTimer;

	QPoint			m_dragPosition;
	QPoint			m_pos;
    QRect			m_rect;
	QString			m_message;
	
	QGridLayout*	m_contentLayout;

	QLabel*			m_statusBarMessage;

	QString			m_windowsSettingsFilename;
	QString			m_cpuLoadString;
	QString			m_dateTimeString;
	QString			m_statusBarMessageString;

	QWidget*		m_buttonWidget;
	QWidget*		m_secondButtonWidget;
	ADCMode			m_adcMode;
	AGCMode			m_agcMode;

	DataEngine*			m_dataEngine;
	RadioPopupWidget*	m_radioPopupWidget;
	ServerWidget*		m_serverWidget;
    QTabWidget*  m_hpsdrTabWidget;
	DisplayTabWidget*	m_displayTabWidget = NULL;
	QGLWidebandPanel*	m_wbDisplay;
	WarningDialog*		m_warningDialog;
    tx_settings_dialog* m_audioInput;
	HamBand				m_currentHamBand;

	quint16				m_alexConfig;
	QList<int>			m_alexStates;
	QList<int>			m_mercuryAttn;
	
	bool		m_resizeFrame;
	bool		m_mousePressed;
	bool		m_quitHighBotton;
	bool		m_fullScreen;
	bool		m_mover;
	bool		m_msgBrowserVisible;

	double		m_alpha;

	int			m_deltaX_max;
    int			m_deltaY_max;
	int			m_resizePosition;
    RadioModel*                             m_radioModel;
    QList<QVariant>                         m_discoveredDevices;
    QTimer                                  m_discoveryTimer;
    bool                                    m_isStartupDiscovery;
	int			m_numberOfReceivers;

	int			m_alexAttnState;
	int			m_mercuryAttnState;

	int			m_oldSampleRate;

protected:
	void resizeEvent(QResizeEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
        void setAGCSliderValue(int value);
};

//***************************************************************************
// NetworkIODialog class

//***************************************************************************
// WarningDialog class

class WarningDialog : public QDialog {

    Q_OBJECT

public:
    
    WarningDialog(QWidget *parent = nullptr);
    ~WarningDialog();

public slots:
        void setWarningMessage(const QString &msg);

protected:
        void paintEvent(QPaintEvent *event) override;

private:
        Settings*               set;

        QFont                   m_titleFont;
        QLabel*                 m_warningLabel;
        AeroButton*             okBtn;
        QString                 m_message;

        int             m_btnWidth;
        int             m_btnHeight;
        int             m_msgFontWidth;
        int             m_msgFontHeight;
    
private slots:
        void    okBtnClicked();
};

#endif // CUSDR_MAIN_H
