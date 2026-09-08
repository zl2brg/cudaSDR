/**
 * @file  DisplayFreqRenderer.h
 * @brief Renderer for VFO frequency rows, digits, and RX region in OGLDisplayPanel.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-08
 */

#ifndef DISPLAY_FREQ_RENDERER_H
#define DISPLAY_FREQ_RENDERER_H

#include <QtGlobal>
#include <QString>
#include <QColor>

class OGLDisplayPanel;
class OGLText;

class DisplayFreqRenderer {
public:
    explicit DisplayFreqRenderer(OGLDisplayPanel *panel);
    ~DisplayFreqRenderer() = default;

    void paintRxRegion();
    void paintVfoFrequencyRow(int which, bool active, int yBaseline, int originX,
                              const QString &f1str, const QString &f2str,
                              const QColor &fontcolor);
    void renderFreqText(OGLText *text, int &x1, int y1, const QColor &fontcolor,
                        const QString &freqstr, int digit, int digit_pos, int fixed_width = 0);
    QString freqMhzDisplayString(qint64 frequencyHz) const;
    void splitFreqDisplay(qint64 frequencyHz, QString *f1str, QString *f2str) const;

private:
    OGLDisplayPanel *m_panel;
};

#endif // DISPLAY_FREQ_RENDERER_H
