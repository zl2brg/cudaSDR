#ifndef _CUSDR_COLOR_OPTIONS_WIDGET_H
#define _CUSDR_COLOR_OPTIONS_WIDGET_H

#include <QPen>
#include <QDebug>
#include <QBoxLayout>
#include <QGroupBox>

#include "Util/cusdr_buttons.h"
#include "Util/cusdr_colorTriangle.h"
#include "cusdr_settings.h"

class ColorOptionsWidget : public QWidget {
	Q_OBJECT

public:
	ColorOptionsWidget(QWidget *parent = 0);
	~ColorOptionsWidget();

	// MVC View Interface Setters
	void	setPanadapterColors(const TPanadapterColors& colors);

signals:
	// MVC View Interface Signals
	void	panadapterColorsRequested(const TPanadapterColors& colors);

public slots:
	QSize	sizeHint() const;
	QSize	minimumSizeHint() const;

private:
	QtColorTriangle				*m_colorTriangle;
	
    QString						m_menu_style;

	QColor			m_currentColor;
	QColor			m_newColor;

	QGroupBox		*m_colorChooserWidget;

	AeroButton		*m_resetBtn;
	AeroButton		*m_okBtn;
	AeroButton		*m_setPanBackground;
	AeroButton		*m_setPanCenterLine;
	AeroButton		*m_setPanLine;
	AeroButton		*m_setPanLineFilling;
	AeroButton		*m_setPanSolidTop;
	AeroButton		*m_setPanSolidBottom;
	AeroButton		*m_setWaterfall;
	AeroButton		*m_setWideBandLine;
	AeroButton		*m_setWideBandFilling;
	AeroButton		*m_setWideBandSolidTop;
	AeroButton		*m_setWideBandSolidBottom;
	AeroButton		*m_setDistanceLine;
	AeroButton		*m_setDistanceLineFilling;
	AeroButton		*m_setGridLine;
	AeroButton		*m_setPanFilter;
    QHBoxLayout     *hbox,*hbox1;
    QBoxLayout      *mainLayout;
    QVBoxLayout     *vbox;
    QGridLayout     *grid1;

	QList<AeroButton *>		m_changeColorBtnList;

	TPanadapterColors		m_panadapterColors;
	TPanadapterColors		m_oldPanadapterColors;
	
	int		m_fontHeight;
	int		m_maxFontWidth;

	int		m_minimumWidgetWidth;
	int		m_minimumGroupBoxWidth;
	int		m_btnSpacing;

	int		m_currentReceiver;
	int		m_btnChooserHit;
	int		m_sampleRate;

	void	createColorChooserWidget();

private slots:
	// Internal UI slots
	void	colorChooserChanged();
	void	resetColors();
	void	acceptColors();
	void	triangleColorChanged(QColor color);
};

#endif // _CUSDR_COLOR_OPTIONS_WIDGET_H
