/*
*   Copyright 2018 Simon Eatough, Zl2BRG
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

#ifndef CUDASDR_CUSDR_NOISEWIDGIT_H
#define CUDASDR_CUSDR_NOISEWIDGIT_H


#include <QWidget>
#include <QPainter>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>

#include "Util/cusdr_buttons.h"
#include "cusdr_settings.h"
#include "cusdr_fonts.h"

/*
class NoiseFilterWidget : public QWidget {
Q_OBJECT

public:
    NoiseFilterWidget(QWidget *parent = 0);public slots:
    QSize	sizeHint() const;
    QSize	minimumSizeHint() const;

private:
    Settings	*set;
    QSDR::_ServerMode			m_serverMode;
    QSDR::_HWInterfaceMode		m_hwInterface;
    QSDR::_DataEngineState		m_dataEngineState;

    QList<TReceiver>	m_rxDataList;

    CFonts		*fonts;
    TFonts		m_fonts;

    ~NoiseFilterWidget();






    QComboBox   *nbModeCombo;
    QComboBox   *nbFilterModeCombo;
    QComboBox   *nr2GainMethodCombo;

    QSlider		*m_hangThresholdSlider;

    QCheckBox   *snbCheckBox;
    QCheckBox   *anfCheckBox;
    QRadioButton *nrPreAGC;
    QRadioButton *nrPostAGC;
    QRadioButton   *nrOSMS;
    QRadioButton   *nrMMSE;
    QCheckBox   *nr2AE;


    QLabel	   *m_nbModelabel;
    QLabel     *m_nbFilterlabel;
    QLabel     *nrGainMethodlabel;
    QLabel     *nrprepostlabel;
    QLabel     *nr2NPElabel;



    int		m_minimumWidgetWidth;
    int		m_minimumGroupBoxWidth;
    int		m_btnSpacing;
    int		m_fontHeight;
    int		m_maxFontWidth;
    int		m_currentReceiver;




    bool	m_mouseOver;
    QGroupBox *noiseBlankerModeGroupBox;
    QGroupBox *noiseFilterModeGroupBox;
    QGroupBox *dspCombo;


    void	setupConnections();
    void    createNoiseBlankerGroup();
    void    createNoiseFilterGroup();
    void    createDspGroup();




private slots:
    void	systemStateChanged(
            QObject *sender,
            QSDR::_Error err,
            QSDR::_HWInterfaceMode hwmode,
            QSDR::_ServerMode mode,
            QSDR::_DataEngineState state);

    void	setCurrentReceiver(QObject *sender, int rx);
};

 */

#endif //CUDASDR_CUSDR_NOISEWIDGIT_H
