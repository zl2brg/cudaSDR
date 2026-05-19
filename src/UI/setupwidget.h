#ifndef SETUPWIGDET_H
#define SETUPWIGDET_H

#include <QWidget>
#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"


namespace Ui {
    class SetupWidget;
}

class SetupWidget : public QDialog
{
Q_OBJECT

public:
    SetupWidget(QDialog *parent = 0);
    ~SetupWidget();

private:
    Ui::SetupWidget *ui;
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
            QSDR::_Error err,
            QSDR::_HWInterfaceMode hwmode,
            QSDR::_ServerMode mode,
            QSDR::_DataEngineState state);

public slots:
    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;

};

#endif // SETUPWIGDET_H

