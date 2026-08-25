#include "eq_curve_plot.h"

#include <QPainter>
#include <algorithm>
#include <cmath>

namespace {

static const double kBandHz[EqCurvePlot::kBandCount] = {
    32.0, 63.0, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0, 16000.0
};

static const char *const kBandLabels[EqCurvePlot::kBandCount] = {
    "32", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
};

} // namespace

const char *EqCurvePlot::bandLabel(int index)
{
    if (index < 0 || index >= kBandCount)
        return "";
    return kBandLabels[index];
}

double EqCurvePlot::bandFrequencyHz(int index)
{
    if (index < 0 || index >= kBandCount)
        return 0.0;
    return kBandHz[index];
}

double EqCurvePlot::bandHzToPosition(double hz)
{
    if (hz <= kBandHz[0])
        return 0.0;
    if (hz >= kBandHz[kBandCount - 1])
        return static_cast<double>(kBandCount - 1);
    for (int i = 0; i < kBandCount - 1; ++i) {
        if (hz <= kBandHz[i + 1]) {
            const double logHz = std::log(hz);
            const double t = (logHz - std::log(kBandHz[i])) / (std::log(kBandHz[i + 1]) - std::log(kBandHz[i]));
            return static_cast<double>(i) + t;
        }
    }
    return static_cast<double>(kBandCount - 1);
}

double EqCurvePlot::bandPositionToX(double bandPos, int plotWidth)
{
    // Match slider column centers: Pre occupies column 0, bands 1..10 use columns 1..10.
    return (bandPos + 1.5) / static_cast<double>(kBandSliderCount) * static_cast<double>(plotWidth);
}

EqCurvePlot::EqCurvePlot(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(80);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void EqCurvePlot::setCurve(const QVector<double> &x, const QVector<double> &yDb)
{
    m_style = Style::Generic;
    m_x = x;
    m_y = yDb;
    m_preampDb = 0.0;
    update();
}

void EqCurvePlot::setBandEqCurve(const QVector<double> &xNorm, const QVector<double> &yDb, double preampDb,
                                 double audioSampleRate)
{
    m_style = Style::BandEq;
    m_x = xNorm;
    m_y = yDb;
    m_preampDb = preampDb;
    m_audioSampleRate = audioSampleRate > 0.0 ? audioSampleRate : kEqAudioSampleRate;
    update();
}

void EqCurvePlot::clearCurve()
{
    m_x.clear();
    m_y.clear();
    m_preampDb = 0.0;
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

    const int labelH = (m_style == Style::BandEq) ? 14 : 0;
    const int top = 2;
    const int bottom = height() - 2 - labelH;
    const int w = width() - 4;
    const int h = bottom - top;
    if (w < 2 || h < 2)
        return;

    double ymin = m_y.first() + m_preampDb;
    double ymax = ymin;
    for (int i = 0; i < m_y.size(); ++i) {
        const double yDb = m_y.at(i) + m_preampDb;
        ymin = std::min(ymin, yDb);
        ymax = std::max(ymax, yDb);
    }
    if (ymax <= ymin) {
        ymin -= 1.0;
        ymax += 1.0;
    }
    const double pad = std::max(1.0, (ymax - ymin) * 0.1);
    ymin -= pad;
    ymax += pad;

    auto yToPixel = [&](double yDb) {
        const double yn = (yDb - ymin) / (ymax - ymin);
        return top + (1.0 - yn) * h;
    };

    // Zero-dB reference if in range.
    if (ymin < 0.0 && ymax > 0.0) {
        const double y0 = yToPixel(0.0);
        p.setPen(QPen(QColor(120, 120, 120), 1, Qt::DotLine));
        p.drawLine(QPointF(2, y0), QPointF(2 + w, y0));
    }

    if (m_style == Style::BandEq) {
        // Vertical guides at band centers (skip Pre column).
        p.setPen(QPen(QColor(50, 50, 50), 1));
        for (int i = 0; i < kBandCount; ++i) {
            const double x = 2.0 + bandPositionToX(static_cast<double>(i), w);
            p.drawLine(QPointF(x, top), QPointF(x, bottom));
        }

        // Pre column marker (boundary between preamp and first band).
        const double preBoundaryX = 2.0 + bandPositionToX(-0.5, w);
        p.setPen(QPen(QColor(45, 45, 45), 1, Qt::DotLine));
        p.drawLine(QPointF(preBoundaryX, top), QPointF(preBoundaryX, bottom));
    }

    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(QColor(80, 180, 255), 1.5));
    QPointF prev;
    bool havePrev = false;
    for (int i = 0; i < m_x.size(); ++i) {
        double xPixel = 0.0;
        if (m_style == Style::BandEq) {
            const double hz = m_x.at(i) * m_audioSampleRate * 0.5;
            const double bandPos = bandHzToPosition(hz);
            xPixel = 2.0 + bandPositionToX(bandPos, w);
        } else {
            double xmin = m_x.first(), xmax = m_x.first();
            for (int j = 0; j < m_x.size(); ++j) {
                xmin = std::min(xmin, m_x.at(j));
                xmax = std::max(xmax, m_x.at(j));
            }
            if (!(xmax > xmin))
                return;
            const double xn = (m_x.at(i) - xmin) / (xmax - xmin);
            xPixel = 2.0 + xn * w;
        }

        const QPointF pt(xPixel, yToPixel(m_y.at(i) + m_preampDb));
        if (havePrev)
            p.drawLine(prev, pt);
        prev = pt;
        havePrev = true;
    }

    if (m_style == Style::BandEq) {
        p.setPen(QColor(150, 150, 150));
        p.setFont(QFont(QStringLiteral("Sans Serif"), 7));
        for (int i = 0; i < kBandCount; ++i) {
            const double x = 2.0 + bandPositionToX(static_cast<double>(i), w);
            const QRect textRect(static_cast<int>(x - 16), bottom + 1, 32, labelH);
            p.drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, QString::fromLatin1(kBandLabels[i]));
        }
        const QRect preRect(0, bottom + 1,
                          static_cast<int>(2.0 + bandPositionToX(-0.5, w)), labelH);
        p.drawText(preRect, Qt::AlignHCenter | Qt::AlignTop, QStringLiteral("Pre"));
    }
}
