#ifndef MODEWIGDET_H
#define MODEWIGDET_H

#include <QWidget>
#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"


namespace Ui {
    class ModeWidget;
}

class ModeWidget : public QDialog
{
Q_OBJECT

public:
    ModeWidget(QDialog *parent = 0);
    ~ModeWidget();

private:
    Ui::ModeWidget *ui;
    Settings	*set;
    QSDR::_ServerMode			m_serverMode;
    QSDR::_HWInterfaceMode		m_hwInterface;
    QSDR::_DataEngineState		m_dataEngineState;

    QList<TReceiver>	m_rxDataList;
    void        getSettings();
    CFonts		*fonts;
    TFonts		m_fonts;

    int     m_rx;
    int		m_minimumWidgetWidth;
    int		m_minimumGroupBoxWidth;
    int		m_btnSpacing;
    int		m_fontHeight;
    int		m_maxFontWidth;
    int		m_currentReceiver;
    bool	m_mouseOver;

    void	setupConnections();


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



#endif // MODEWIGDET_H
