#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QObject>
#include <QDialog>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"


#define	btn_height		30
#define	btn_width		90
#define	btn_widths		42
#define	btn_width2		52
#define	btn_width3		60

class baseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit  baseWidget(QWidget *parent = nullptr,int rx = 0);
    void setupModeBtn(AeroButton *btn);
    void setupFilterBtn(AeroButton *btn);
    void setupBandBtn(AeroButton *btn);
    ~baseWidget();
    QList<TReceiver>	m_receiverDataList;
    QList<DSPMode>		m_dspModeList;
    QList<AeroButton *>	m_mode_btnList;
    QList<AeroButton *>	m_filter_btnList;
    QList<AeroButton *>	m_band_btnList;


    int		m_currentReceiver;
    int		m_receiver = 0;
    int		m_currentRx;
    DSPMode m_dspMode;
    qreal	m_filterLo;
    qreal	m_filterHi;
    HamBand m_hamBand;
    QList<long>			m_lastCtrFrequencyList;
    QList<long>			m_lastVfoFrequencyList;
    Settings	*set;
private:


    int		m_minimumWidgetWidth;
    int		m_minimumGroupBoxWidth;
    int		m_btnSpacing;
    int		m_fontHeight;
    int		m_maxFontWidth;
    bool    m_mouseOver;

signals:

public slots:

    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;
    virtual void  ModebtnCallback() = 0;
    virtual void  BandbtnCallback() = 0;
    virtual void  FilterbtnCallback() = 0;


};

#endif // BASEWIDGET_H
