#ifndef RADIOCTRL_H
#define RADIOCTRL_H

#include <QWidget>
#include <QAbstractSlider>
#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"
#include "basewidget.h"

#include "UI/band_widget.h"


namespace Ui {
    class RadioCtrl;
}

class RadioCtrl : public baseWidget
{
Q_OBJECT

public:
    RadioCtrl(QWidget *parent = 0,int rx = 0);
    ~RadioCtrl();
    void updateFilterWidget();

private:
    Ui::RadioCtrl *ui;
    Settings	*set;
    QSDR::_ServerMode			m_serverMode;
    QSDR::_HWInterfaceMode		m_hwInterface;
    QSDR::_DataEngineState		m_dataEngineState;

     QList<TReceiver>	m_rxDataList;
    void        getSettings();
    CFonts		*fonts;
    TFonts		m_fonts;
    QList<long>			m_lastCtrFrequencyList;
    QList<long>			m_lastVfoFrequencyList;
    void setFilterbyMode(QString str, DSPMode mode);
    long	m_ctrFrequency;
    long	m_vfoFrequency;



    qreal SetVarSlider(QAbstractSlider *slider);
    void setFilterWidget();
    void setModeWidget();
    void setBandWidget();

    int		m_minimumWidgetWidth;
    int		m_minimumGroupBoxWidth;
    int		m_btnSpacing;
    int		m_fontHeight;
    int		m_maxFontWidth;
    int		m_currentReceiver;
    bool	m_mouseOver;

    void	setupConnections();

public slots:
    void modeChange();
    void ModebtnCallback();
    void BandbtnCallback();
    void FilterbtnCallback();
    void vfoFrequencyChanged(QObject* sender, int mode, int rx, long frequency);
    void ctrFrequencyChanged(QObject* sender, int mode, int rx, long frequency);
    void filterChanged(QObject *sender, int rx, qreal low, qreal high);
    void filterChangedByBtn();
    void dspModeChanged(QObject *sender,int rx, DSPMode mode);
    void slider_changed(int value);
    void bandChanged(QObject *sender, int rx, bool byButton, HamBand band);

private slots:
    void	systemStateChanged(
            QObject *sender,
            QSDR::_Error err,
            QSDR::_HWInterfaceMode hwmode,
            QSDR::_ServerMode mode,
            QSDR::_DataEngineState state);
/*
    void	setCurrentReceiver(QObject *sender, int rx);
    void    nfModeChanged(int value);
    void    nbModeChanged(int value);
    void    nr2GainChanged(int value);
    void    agcProcChanged(int value);
    void    snbChanged(bool value);
    void    anfChanged(bool value);
    void    nr2aeChanged(bool value);
    void    omsChanged(bool value);
    void    mmseChanged(bool value);
    void    preAgcChanged(bool value);
    void    postAgcChanged(bool nalue);
    void    npeModeChanged(int value);
*/





public slots:
    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;



};



#endif // RADIOCTRL_H
