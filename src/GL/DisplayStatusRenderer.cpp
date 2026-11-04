/**
 * @file  DisplayStatusRenderer.cpp
 * @brief Renderer for upper status badges (sync, ADC, loss, volts, temp, HW info)
 *        and lower status indicators (attenuator, dither, random, rate, clocks) in OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#include "DisplayStatusRenderer.h"
#include "cusdr_oglDisplayPanel.h"
#include "cusdr_fonts.h"
#include "cusdr_oglText.h"
#include <cmath>

DisplayStatusRenderer::DisplayStatusRenderer(OGLDisplayPanel *panel)
    : m_panel(panel)
{
}

void DisplayStatusRenderer::paintUpperRegion()
{
    if (!m_panel)
        return;

    QString str;

    GLint x1 = m_panel->m_rxRect.left() + m_panel->m_blankWidth;
    GLint y1 = m_panel->m_rxRect.top();

    // sync status
    str = QString(m_panel->m_SYNCString);
    QRect rect = QRect(x1, y1, m_panel->m_syncWidth + 2 * m_panel->m_blankWidth, m_panel->m_blankHeight);

    switch (m_panel->m_syncStatus) {
        case 0:
            m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
            break;
        case 1:
            m_panel->drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
            break;
        case 2:
            m_panel->drawPanelRect(rect, QColor(242, 56, 109), -2.0f);
            break;
    }
    m_panel->qglColor(Qt::black);
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y1, m_panel->m_SYNCString);

    // ADC status
    str = QString(m_panel->m_ADCString);
    x1 += m_panel->m_syncWidth + 2 * m_panel->m_blankWidth + 2;
    rect = QRect(x1, y1, m_panel->m_adcWidth + 2 * m_panel->m_blankWidth, m_panel->m_blankHeight);

    switch (m_panel->m_adcStatus) {
        case 0:
            m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
            break;
        case 1:
            m_panel->drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
            break;
        case 2:
            m_panel->drawPanelRect(rect, QColor(242, 56, 109), -2.0f);
            break;
    }
    m_panel->qglColor(Qt::black);
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y1, m_panel->m_ADCString);

    // Packet loss status
    str = QString(m_panel->m_PacketLossString);
    x1 += m_panel->m_adcWidth + 2 * m_panel->m_blankWidth + 2;
    rect = QRect(x1, y1, m_panel->m_packetLossWidth + 2 * m_panel->m_blankWidth, m_panel->m_blankHeight);

    switch (m_panel->m_packetLossStatus) {
        case 0:
            m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
            break;
        case 1:
            m_panel->drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
            break;
        case 2:
            m_panel->drawPanelRect(rect, QColor(242, 56, 109), -2.0f);
            break;
    }
    m_panel->qglColor(Qt::black);
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y1, m_panel->m_PacketLossString);

    // Metis status
    str = m_panel->m_metisString;
    x1 += m_panel->m_packetLossWidth + 2 * m_panel->m_blankWidth + 2;

    // Supply Voltage
    if (m_panel->m_supplyVolts > 0.1) {
        QString voltStr = QString("%1V").arg(m_panel->m_supplyVolts, 0, 'f', 1);
        int voltWidth = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(voltStr);
        rect = QRect(x1, y1, voltWidth + 2 * m_panel->m_blankWidth, m_panel->m_blankHeight);
        m_panel->drawPanelRect(rect, QColor(100, 120, 140), -2.0f); // Blue-grey
        m_panel->qglColor(QColor(206, 236, 248));
        m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y1, voltStr);
        x1 += voltWidth + 5 * m_panel->m_blankWidth;
    }

    // Temperature
    if (m_panel->m_temperature > 0.1) {
        QString tempStr = QString("%1°C").arg(m_panel->m_temperature, 0, 'f', 1);
        int tempWidth = m_panel->m_oglTextSmall->fontMetrics().horizontalAdvance(tempStr);
        rect = QRect(x1, y1, tempWidth + 2 * m_panel->m_blankWidth, m_panel->m_blankHeight);
        m_panel->drawPanelRect(rect, QColor(80, 80, 80), -2.0f); // Deep grey
        m_panel->qglColor(QColor(206, 236, 248));
        m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y1, tempStr);
        x1 += tempWidth + 5 * m_panel->m_blankWidth;
    }

    if (m_panel->m_hwInterface == QSDR::Metis && m_panel->m_dataEngineState == QSDR::DataEngineUp)
        rect = QRect(x1, y1, m_panel->m_metisStringWidth + m_panel->m_versionStringWidth, m_panel->m_blankHeight);
    else
        rect = QRect(x1, y1, m_panel->m_metisStringWidth, m_panel->m_blankHeight);

    if (m_panel->m_hwInterface == QSDR::Metis) {
        m_panel->drawPanelRect(rect, m_panel->m_textBackgroundColor, -2.0f);
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            str.append(m_panel->m_metisVersion);
            m_panel->qglColor(QColor(206, 236, 248));
        } else {
            m_panel->qglColor(QColor(0, 0, 0));
        }
    } else {
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1, y1, 1.0f, str);

    // Mercury status
    str = m_panel->m_mercuryString;

    if (m_panel->m_hwInterface == QSDR::Metis && m_panel->m_dataEngineState == QSDR::DataEngineUp) {
        x1 += m_panel->m_metisStringWidth + m_panel->m_versionStringWidth + m_panel->m_blankWidth;
        rect = QRect(x1, y1, m_panel->m_mercuryStringWidth + m_panel->m_versionStringWidth, m_panel->m_blankHeight);
    } else {
        x1 += m_panel->m_metisStringWidth + m_panel->m_blankWidth;
        rect = QRect(x1, y1, m_panel->m_mercuryStringWidth, m_panel->m_blankHeight);
    }

    if (m_panel->set->getMercuryPresence() && m_panel->m_hwInterface == QSDR::Metis) {
        m_panel->drawPanelRect(rect, m_panel->m_textBackgroundColor, -2.0f);
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            str.append(m_panel->m_mercuryVersion);
            m_panel->qglColor(QColor(206, 236, 248));
        } else {
            m_panel->qglColor(QColor(0, 0, 0));
        }
    } else {
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1, y1, 1.0f, str);

    // Penelope status
    str = m_panel->m_penelopeString;

    if (m_panel->m_hwInterface == QSDR::Metis && m_panel->m_dataEngineState == QSDR::DataEngineUp)
        x1 += m_panel->m_mercuryStringWidth + m_panel->m_versionStringWidth + m_panel->m_blankWidth;
    else
        x1 += m_panel->m_mercuryStringWidth + m_panel->m_blankWidth;

    if (m_panel->set->getPenelopePresence() && m_panel->m_hwInterface == QSDR::Metis) {
        str = m_panel->m_penelopeString;

        if (m_panel->m_dataEngineState == QSDR::DataEngineUp)
            rect = QRect(x1, y1, m_panel->m_penelopeStringWidth + m_panel->m_versionStringWidth, m_panel->m_blankHeight);
        else
            rect = QRect(x1, y1, m_panel->m_penelopeStringWidth, m_panel->m_blankHeight);

        m_panel->drawPanelRect(rect, m_panel->m_textBackgroundColor, -2.0f);

        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            str.append(m_panel->m_penelopeVersion);
            m_panel->qglColor(QColor(206, 236, 248));
        } else {
            m_panel->qglColor(QColor(0, 0, 0));
        }
    } else if (m_panel->set->getPennyLanePresence() && m_panel->m_hwInterface == QSDR::Metis) {
        str = m_panel->m_pennylaneString;

        if (m_panel->m_dataEngineState == QSDR::DataEngineUp)
            rect = QRect(x1, y1, m_panel->m_pennylaneStringWidth + m_panel->m_versionStringWidth, m_panel->m_blankHeight);
        else
            rect = QRect(x1, y1, m_panel->m_pennylaneStringWidth, m_panel->m_blankHeight);

        m_panel->drawPanelRect(rect, m_panel->m_textBackgroundColor, -2.0f);

        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            str.append(m_panel->m_pennylaneVersion);
            m_panel->qglColor(QColor(206, 236, 248));
        } else {
            m_panel->qglColor(QColor(0, 0, 0));
        }
    } else {
        rect = QRect(x1, y1, m_panel->m_penelopeStringWidth, m_panel->m_blankHeight);
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }

    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1, y1, 1.0f, str);

    // Hermes status
    str = m_panel->m_hermesString;

    if (m_panel->set->getPennyLanePresence())
        x1 += m_panel->m_pennylaneStringWidth;
    else
        x1 += m_panel->m_penelopeStringWidth;

    x1 += m_panel->m_blankWidth;

    if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
        if (m_panel->m_hwInterface == QSDR::Metis) {
            if (m_panel->set->getPenelopePresence() || m_panel->set->getPennyLanePresence())
                x1 += m_panel->m_versionStringWidth;
            rect = QRect(x1, y1, m_panel->m_hermesStringWidth, m_panel->m_blankHeight);
        } else {
            rect = QRect(x1, y1, m_panel->m_hermesStringWidth + m_panel->m_versionStringWidth, m_panel->m_blankHeight);
        }
    } else {
        rect = QRect(x1, y1, m_panel->m_hermesStringWidth, m_panel->m_blankHeight);
    }

    if (m_panel->set->getHPSDRHardware() == 1) {
        m_panel->drawPanelRect(rect, m_panel->m_textBackgroundColor, -2.0f);
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            str.append(m_panel->m_hermesVersion);
            m_panel->qglColor(QColor(206, 236, 248));
        } else {
            m_panel->qglColor(QColor(0, 0, 0));
        }
    } else {
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1, y1, 1.0f, str);

    // Excalibur status
    str = m_panel->m_excaliburString;

    if (m_panel->m_dataEngineState == QSDR::DataEngineUp && m_panel->m_hwInterface == QSDR::Hermes)
        x1 += m_panel->m_hermesStringWidth + m_panel->m_versionStringWidth + m_panel->m_blankWidth;
    else
        x1 += m_panel->m_hermesStringWidth + m_panel->m_blankWidth;

    rect = QRect(x1, y1, m_panel->m_excaliburStringWidth, m_panel->m_blankHeight);

    if (m_panel->set->getExcaliburPresence() && m_panel->m_hwInterface == QSDR::Metis) {
        m_panel->drawPanelRect(rect, m_panel->m_textBackgroundColor, -2.0f);
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            str.append(m_panel->m_excaliburVersion);
            m_panel->qglColor(QColor(206, 236, 248));
        } else {
            m_panel->qglColor(QColor(0, 0, 0));
        }
    } else {
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1, y1, 1.0f, str);

    // Alex status
    str = m_panel->m_alexString;
    x1 += m_panel->m_excaliburStringWidth + m_panel->m_blankWidth;
    rect = QRect(x1, y1, m_panel->m_alexStringWidth + m_panel->m_blankWidth, m_panel->m_blankHeight);

    if (m_panel->set->getAlexPresence()) {
        m_panel->drawPanelRect(rect, m_panel->m_textBackgroundColor, -2.0f);
        if (m_panel->m_dataEngineState == QSDR::DataEngineUp) {
            str.append(m_panel->m_alexVersion);
            m_panel->qglColor(QColor(206, 236, 248));
        } else {
            m_panel->qglColor(QColor(0, 0, 0));
        }
    } else {
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1, y1, 1.0f, str);

    // RigCtl status
    x1 += m_panel->m_alexStringWidth + m_panel->m_blankWidth;
    rect = QRect(x1, y1, m_panel->m_rigCtlStringWidth + 2 * m_panel->m_blankWidth, m_panel->m_blankHeight);
    if (m_panel->m_rigCtlConnected) {
        m_panel->drawPanelRect(rect, QColor(56, 242, 115), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    } else {
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y1, m_panel->m_rigCtlString);

    // TCI status: green = connected, yellow = TX waiting, orange = TX silent,
    // cyan = TX audio arriving from the client (debug: WSJT-X vs cudaSDR).
    x1 += m_panel->m_rigCtlStringWidth + 2 * m_panel->m_blankWidth + 2;
    rect = QRect(x1, y1, m_panel->m_tciStringWidth + 2 * m_panel->m_blankWidth, m_panel->m_blankHeight);
    if (m_panel->m_tciConnected) {
        QColor tciColor(56, 242, 115);
        if (m_panel->m_tciTxAudioDebug == 1)
            tciColor = QColor(242, 220, 56);
        else if (m_panel->m_tciTxAudioDebug == 2)
            tciColor = QColor(242, 140, 56);
        else if (m_panel->m_tciTxAudioDebug == 3)
            tciColor = QColor(56, 200, 242);
        m_panel->drawPanelRect(rect, tciColor, -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    } else {
        m_panel->drawPanelRect(rect, QColor(68, 68, 68), -2.0f);
        m_panel->qglColor(QColor(0, 0, 0));
    }
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y1, m_panel->m_tciString);
}

void DisplayStatusRenderer::paintLowerRegion()
{
    if (!m_panel)
        return;

    QString str;

    GLint x1 = m_panel->m_rxRect.left() + m_panel->m_blankWidth;
    GLint y2 = m_panel->m_rxRect.height() - m_panel->m_lowerRectY;

    // Attenuator
    m_panel->qglColor(QColor(106, 136, 148));
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, m_panel->m_AttnString);

    x1 += m_panel->m_AttnWidth + 2 * m_panel->m_blankWidth;
    if (m_panel->m_mercuryAttenuator == 0) {
        str = QStringLiteral("0 dB");
    } else if (m_panel->m_mercuryAttenuator == 1 || m_panel->m_mercuryAttenuator == 10 || m_panel->m_mercuryAttenuator == -10) {
        str = QStringLiteral("-10 dB");
    } else if (m_panel->m_mercuryAttenuator == 2 || m_panel->m_mercuryAttenuator == 20 || m_panel->m_mercuryAttenuator == -20) {
        str = QStringLiteral("-20 dB");
    } else if (m_panel->m_mercuryAttenuator == 3 || m_panel->m_mercuryAttenuator == 30 || m_panel->m_mercuryAttenuator == -30) {
        str = QStringLiteral("-30 dB");
    } else {
        str = QStringLiteral("%1 dB").arg(m_panel->m_mercuryAttenuator > 0 ? -m_panel->m_mercuryAttenuator : m_panel->m_mercuryAttenuator);
    }

    int attnValueWidth = m_panel->m_oglTextSmall->fontMetrics().tightBoundingRect(str).width();
    m_panel->qglColor(m_panel->m_activeTextColor);
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1, y2, str);

    // Dither status
    x1 += attnValueWidth + 5 * m_panel->m_blankWidth;

    if (m_panel->m_dither == 1)
        m_panel->qglColor(m_panel->m_activeTextColor);
    else
        m_panel->qglColor(QColor(68, 68, 68));

    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, m_panel->m_ditherString);

    // Random status
    x1 += m_panel->m_ditherWidth + 5 * m_panel->m_blankWidth;

    if (m_panel->m_random == 1)
        m_panel->qglColor(m_panel->m_activeTextColor);
    else
        m_panel->qglColor(QColor(68, 68, 68));

    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, m_panel->m_randomString);

    // Sample rate status
    x1 += m_panel->m_randomWidth + 10 * m_panel->m_blankWidth;
    str = "%1";

    m_panel->qglColor(QColor(166, 196, 208));
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, str.arg(m_panel->m_sample_rate, 3, 10, QLatin1Char(' ')));

    int samplerateWidth = m_panel->m_oglTextSmall->fontMetrics().tightBoundingRect(str.arg(m_panel->m_sample_rate, 3, 10, QLatin1Char(' '))).width();
    x1 += samplerateWidth + 4 * m_panel->m_blankWidth;

    str = "kHz";
    int samplerateUnitWidth = m_panel->m_oglTextSmall->fontMetrics().tightBoundingRect(str).width();
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, str);

    // server modus status
    x1 += samplerateUnitWidth + 10 * m_panel->m_blankWidth;
    switch (m_panel->m_serverMode) {
        case QSDR::NoServerMode:
            str = "No Server mode";
            break;
        case QSDR::SDRMode:
            str = "SDR Mode";
            break;
    }
    int serverModeStringWidth = m_panel->m_oglTextSmall->fontMetrics().tightBoundingRect(str).width();

    m_panel->qglColor(QColor(166, 196, 208));
    m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, str);

    if (m_panel->m_hwInterface == QSDR::Metis) {
        x1 += serverModeStringWidth + 15 * m_panel->m_blankWidth;

        // 10 MHz source status
        m_panel->qglColor(QColor(106, 136, 148));
        m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, m_panel->m_10MHzString);

        x1 += m_panel->m_10MHzWidth + 4 * m_panel->m_blankWidth;
        m_panel->qglColor(QColor(166, 196, 208));
        int src10MHStringWidth = m_panel->m_oglTextSmall->fontMetrics().tightBoundingRect(m_panel->m_src10mhz).width();
        m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, m_panel->m_src10mhz);

        // 122.88 MHz source status
        x1 += src10MHStringWidth + 10 * m_panel->m_blankWidth;
        m_panel->qglColor(QColor(106, 136, 148));
        m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, m_panel->m_12288MHzString);

        x1 += m_panel->m_12288MHzWidth + 4 * m_panel->m_blankWidth;
        m_panel->qglColor(QColor(166, 196, 208));
        m_panel->renderPanelText(m_panel->m_oglTextSmallItalic, x1 + m_panel->m_blankWidth, y2, m_panel->m_src122_88mhz);
    }
}
