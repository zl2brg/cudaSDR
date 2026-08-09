#include "eq_curve_plot.h"

#include <QPainter>
#include <algorithm>
#include <cmath>

EqCurvePlot::EqCurvePlot(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(56);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void EqCurvePlot::setCurve(const QVector<double> &xHz, const QVector<double> &yDb)
{
    m_x = xHz;
    m_y = yDb;
    update();
}

void EqCurvePlot::clearCurve()
{
    m_x.clear();
    m_y.clear();
    update();
}

void EqCurvePlot::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(30, 30, 30));
    p.setPen(QColor(70, 70, 70));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    if (m_x.size() < 2 || m_x.size() != m_y.size())
        return;

    double xmin = m_x.first(), xmax = m_x.first();
    double ymin = m_y.first(), ymax = m_y.first();
    for (int i = 0; i < m_x.size(); ++i) {
        xmin = std::min(xmin, m_x.at(i));
        xmax = std::max(xmax, m_x.at(i));
        ymin = std::min(ymin, m_y.at(i));
        ymax = std::max(ymax, m_y.at(i));
    }
    if (!(xmax > xmin))
        return;
    if (ymax <= ymin) {
        ymin -= 1.0;
        ymax += 1.0;
    }
    // Pad vertically so a flat curve is visible.
    const double pad = std::max(1.0, (ymax - ymin) * 0.1);
    ymin -= pad;
    ymax += pad;

    const int w = width() - 4;
    const int h = height() - 4;
    if (w < 2 || h < 2)
        return;

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(QColor(80, 180, 255), 1.5));
    QPointF prev;
    bool havePrev = false;
    for (int i = 0; i < m_x.size(); ++i) {
        const double xn = (m_x.at(i) - xmin) / (xmax - xmin);
        const double yn = (m_y.at(i) - ymin) / (ymax - ymin);
        const QPointF pt(2.0 + xn * w, 2.0 + (1.0 - yn) * h);
        if (havePrev)
            p.drawLine(prev, pt);
        prev = pt;
        havePrev = true;
    }

    // Zero-dB reference if in range.
    if (ymin < 0.0 && ymax > 0.0) {
        const double y0 = 2.0 + (1.0 - (0.0 - ymin) / (ymax - ymin)) * h;
        p.setPen(QPen(QColor(120, 120, 120), 1, Qt::DotLine));
        p.drawLine(QPointF(2, y0), QPointF(2 + w, y0));
    }
}
