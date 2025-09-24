#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QWidget>
#include <QAbstractSlider>
#include "Util/cusdr_buttons.h"
#include "basewidget.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"

typedef struct {
    QString txt;
    qreal filterWidth;

}filterStruct;

static filterStruct Narrow_FilterGroup[12]={

    {"1k",1150.0f},
    {"800",950.0f},
    {"750",900.0f},
    {"600",750.0f},
    {"500",650.0f},
    {"400",550.0f},
    {"250",400.0f},
    {"100",350.0f},
    {"50",200.0f},
    {"25",175.0f},
    {"Var1",2000.0f},
    {"Var2",2000.0f}
};

static filterStruct Mid_FilterGroup[12]={
    {"5k",4550.0f},
    {"4k4",4550.0f},
    {"3k8",3950.0f},
    {"3k3",3450.0f},
    {"2k9",3050.0f},
    {"2k7",2850.0f},
    {"2k4",2550.0f},
    {"2k1",2250.0f},
    {"1k8",1950.0f},
    {"1k",1150.0f},
    {"Var1",10000.0f},
    {"Var2",10000.0f}
};

static filterStruct Wide_FilterGroup[12]={
    {"16k",8000.0f},
    {"12k",6000.0f},
    {"10k",5000.0f},
    {"8k",4000.0f},
    {"6k6",3300.0f},
    {"5k2", 2600.0f},
    {"4k",2000.0f},
    {"3k3",3450.0f},
    {"3k1",3450.0f},
    {"2k9",3050.0f},
    {"Var1",20000.0f},
    {"Var2",20000.0f}
};


typedef enum _filterMode{
    M_LSB,
    M_USB,
    M_DSB
} filterMode;

typedef enum _filterGroup{
    NARROW_FILTER,
    MID_FILTER,
    WIDEBAND_FILTER
} filterGroup;


namespace Ui {
    class FilterWidget;
}

class FilterWidget : public baseWidget
{
Q_OBJECT

public:
    FilterWidget(QWidget *parent = 0);
    ~FilterWidget();
    filterMode setFilterMode(DSPMode mode);
    filterGroup setFilterGroup(DSPMode mode);
    void updateFilterWidget();

private:
    Ui::FilterWidget *ui;
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
    filterGroup m_FilterGroup;
    filterMode m_FilterMode;
    filterStruct * m_FilterData;
    qreal SetVarSlider(QAbstractSlider *slider);

    HamBand m_hamBand;


    void	setupConnections();

public slots:
    void btnCallback();
    void filterChanged(QObject *sender, int rx, qreal low, qreal high);
    void filterChangedByBtn();
    void dspModeChanged(QObject *sender,int rx, DSPMode mode);
    void slider_changed(int value);
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






};



#endif // FILTERWIDGET_H
