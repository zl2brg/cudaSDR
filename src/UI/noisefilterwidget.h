#ifndef NOISEFILTERWIDGET_H
#define NOISEFILTERWIDGET_H

#include <QWidget>
#include "cusdr_settings.h"
#include "Util/cusdr_buttons.h"
#include "cusdr_fonts.h"

namespace Ui {
class NoiseFilterWidget;
}

class NoiseFilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoiseFilterWidget(QWidget *parent = nullptr);
    ~NoiseFilterWidget();

    // MVC View interface setters
    void setNrMode(int value);
    void setNbMode(int value);
    void setNr2GainMethod(int value);
    void setNr2NpeMethod(int value);
    void setNrAgc(int value);
    void setNr2Ae(bool value);
    void setSnb(bool value);
    void setAnf(bool value);
    void setReceiver(int rx) { m_rx = rx; }
    int getReceiver() const { return m_rx; }

signals:
    // MVC View interface signals
    void nrModeRequested(int value);
    void nbModeRequested(int value);
    void nr2GainMethodRequested(int value);
    void nr2NpeMethodRequested(int value);
    void nrAgcRequested(int value);
    void nr2AeRequested(bool value);
    void snbRequested(bool value);
    void anfRequested(bool value);

public slots:
    void	systemStateChanged(
            QSDR::_Error err,
            QSDR::_HWInterfaceMode hwmode,
            QSDR::_ServerMode mode,
            QSDR::_DataEngineState state);

private slots:
    void    nfModeChanged(int value);
    void    nbModeChanged(int value);
    void    nr2GainChanged(int value);
    void    snbChanged(bool value);
    void    anfChanged(bool value);
    void    nr2aeChanged(bool value);
    void    omsChanged(bool value);
    void    mmseChanged(bool value);
    void    preAgcChanged(bool value);
    void    postAgcChanged(bool value);

private:
    Ui::NoiseFilterWidget *ui;
    QSDR::_ServerMode			m_serverMode;
    QSDR::_HWInterfaceMode		m_hwInterface;
    QSDR::_DataEngineState		m_dataEngineState;

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
};

#endif // NOISEFILTERWIDGET_H
