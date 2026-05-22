#ifndef NOISEFILTERWIDGET_H
#define NOISEFILTERWIDGET_H

#include <QWidget>
#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"


namespace Ui {
class NoiseFilterWidget;
}

class SliceModel;
class NoiseFilterWidget : public QWidget
{
    Q_OBJECT

public:
    NoiseFilterWidget(SliceModel *model, QWidget *parent = nullptr);
    ~NoiseFilterWidget();

private:
    SliceModel*      m_sliceModel;
    Ui::NoiseFilterWidget *ui;
    Settings *set;
    QSDR::_ServerMode			m_serverMode;
    QSDR::_HWInterfaceMode		m_hwInterface;
    QSDR::_DataEngineState		m_dataEngineState;

    QList<TReceiver>	m_rxDataList;
    void        getSettings();
    CFonts		*fonts;
    TFonts		m_fonts;

    int     m_rx;
    int		m_minimumGroupBoxWidth;
    int		m_btnSpacing;
    int		m_fontHeight;
    int		m_maxFontWidth;
    int		m_currentReceiver;
    bool	m_mouseOver;

    void	setupConnections();


private slots:
    void	systemStateChanged(
            QSDR::_Error err,
            QSDR::_HWInterfaceMode hwmode,
            QSDR::_ServerMode mode,
            QSDR::_DataEngineState state);

    void	setCurrentReceiver(int rx);
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
    void    postAgcChanged(bool value);
    void    npeModeChanged(int value);


};

#endif // NOISEFILTERWIDGET_H
