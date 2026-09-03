#include "eq_curve_plot.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

namespace {

static const double kBandHz[EqCurvePlot::kBandCount] = {
    32.0, 63.0, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0, 16000.0
};

static const char *const kBandLabels[EqCurvePlot::kBandCount] = {
    "32", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
};

static const double kCfcHz[EqCurvePlot::kCfcBandCount] = {
    50.0, 150.0, 300.0, 500.0, 750.0, 1250.0, 1750.0, 2300.0, 2800.0, 3100.0
};

static const char *const kCfcLabels[EqCurvePlot::kCfcBandCount] = {
    "50", "150", "300", "500", "750", "1k2", "1k7", "2k3", "2k8", "3k1"
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

const char *EqCurvePlot::cfcBandLabel(int index)
{
    if (index < 0 || index >= kCfcBandCount)
        return "";
    return kCfcLabels[index];
}

double EqCurvePlot::cfcBandFrequencyHz(int index)
{
    if (index < 0 || index >= kCfcBandCount)
        return 0.0;
    return kCfcHz[index];
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

int EqCurvePlot::totalBandCount() const
{
    return (m_plotMode == PlotMode::Cfc) ? kCfcBandCount : kBandSliderCount;
}

double EqCurvePlot::minGainDb() const
{
    return (m_plotMode == PlotMode::Cfc) ? -16.0 : -12.0;
}

double EqCurvePlot::maxGainDb() const
{
    return (m_plotMode == PlotMode::Cfc) ? 16.0 : 12.0;
}

EqCurvePlot::EqCurvePlot(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(100);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_bandGains.fill(0, kBandSliderCount);
}

void EqCurvePlot::setPlotMode(PlotMode mode)
{
    m_plotMode = mode;
    const int count = totalBandCount();
    if (m_bandGains.size() < count)
        m_bandGains.resize(count);
    update();
}

void EqCurvePlot::setAccentColor(const QColor &color)
{
    m_accentColor = color;
    update();
}

void EqCurvePlot::setInteractive(bool interactive)
{
    m_interactive = interactive;
    if (interactive && m_plotMode == PlotMode::Generic)
        m_plotMode = PlotMode::BandEq;
    setMouseTracking(interactive);
    update();
}

void EqCurvePlot::setBandGains(const QVector<int> &gains)
{
    m_bandGains = gains;
    const int count = totalBandCount();
    while (m_bandGains.size() < count)
        m_bandGains.append(0);
    if (m_interactive && m_plotMode == PlotMode::Generic)
        m_plotMode = PlotMode::BandEq;
    update();
}

void EqCurvePlot::setAudioPassband(double lowHz, double highHz, bool visible)
{
    m_hasPassband = visible;
    m_passbandLowHz = std::min(lowHz, highHz);
    m_passbandHighHz = std::max(lowHz, highHz);
    update();
}

void EqCurvePlot::setCurve(const QVector<double> &x, const QVector<double> &yDb)
{
    m_x = x;
    m_y = yDb;
    m_preampDb = 0.0;
    update();
}

void EqCurvePlot::setBandEqCurve(const QVector<double> &xNorm, const QVector<double> &yDb, double preampDb,
                                 double audioSampleRate)
{
    m_plotMode = PlotMode::BandEq;
    m_x = xNorm;
    m_y = yDb;
    m_preampDb = preampDb;
    if (!m_bandGains.isEmpty())
        m_bandGains[0] = static_cast<int>(std::round(preampDb));
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

int EqCurvePlot::hitTestBand(const QPointF &pos, double *distOut) const
{
    if (!m_interactive || m_plotMode == PlotMode::Generic)
        return -1;

    const int count = totalBandCount();
    const int labelH = 16;
    const int left = 4;
    const int top = 4;
    const int bottom = height() - 4 - labelH;
    const int w = width() - 8;
    const int h = bottom - top;
    if (w < 2 || h < 2)
        return -1;

    double ymin = minGainDb();
    double ymax = maxGainDb();
    for (int g : m_bandGains) {
        ymin = std::min(ymin, static_cast<double>(g));
        ymax = std::max(ymax, static_cast<double>(g));
    }
    for (double v : m_y) {
        ymin = std::min(ymin, v + m_preampDb);
        ymax = std::max(ymax, v + m_preampDb);
    }
    ymin -= 2.0;
    ymax += 2.0;

    int bestIndex = -1;
    double bestDistSq = 1e9;

    for (int i = 0; i < count; ++i) {
        const double x = left + (i + 0.5) / static_cast<double>(count) * static_cast<double>(w);
        const double gainDb = (i < m_bandGains.size()) ? static_cast<double>(m_bandGains.at(i)) : 0.0;
        const double yn = (gainDb - ymin) / (ymax - ymin);
        const double y = top + (1.0 - yn) * h;

        const double dx = pos.x() - x;
        const double dy = pos.y() - y;
        const double distSq = dx * dx + dy * dy;

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestIndex = i;
        }
    }

    if (bestDistSq <= 16.0 * 16.0) {
        if (distOut) *distOut = std::sqrt(bestDistSq);
        return bestIndex;
    }

    // Secondary column hit-test (within +/- 14px horizontally, within graph vertically)
    for (int i = 0; i < count; ++i) {
        const double x = left + (i + 0.5) / static_cast<double>(count) * static_cast<double>(w);
        if (std::abs(pos.x() - x) <= 14.0 && pos.y() >= top - 6 && pos.y() <= bottom + 6) {
            if (distOut) *distOut = std::abs(pos.x() - x);
            return i;
        }
    }

    return -1;
}

double EqCurvePlot::pixelToYDb(double yPixel, double ymin, double ymax, int top, int h) const
{
    const double yn = 1.0 - (yPixel - top) / static_cast<double>(h);
    const double gain = ymin + yn * (ymax - ymin);
    return qBound(minGainDb(), std::round(gain), maxGainDb());
}

void EqCurvePlot::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(22, 22, 25));
    p.setPen(QColor(55, 55, 62));
    p.drawRect(rect().adjusted(0, 0, -1, -1));

    const bool hasLabels = (m_plotMode != PlotMode::Generic);
    const int labelH = hasLabels ? 16 : 0;
    const int left = 4;
    const int top = 4;
    const int bottom = height() - 4 - labelH;
    const int w = width() - 8;
    const int h = bottom - top;
    if (w < 2 || h < 2)
        return;

    const int count = totalBandCount();
    double ymin = minGainDb();
    double ymax = maxGainDb();
    for (int g : m_bandGains) {
        ymin = std::min(ymin, static_cast<double>(g));
        ymax = std::max(ymax, static_cast<double>(g));
    }
    for (double v : m_y) {
        ymin = std::min(ymin, v + m_preampDb);
        ymax = std::max(ymax, v + m_preampDb);
    }
    ymin -= 2.0;
    ymax += 2.0;

    auto yToPixel = [&](double yDb) {
        const double yn = (yDb - ymin) / (ymax - ymin);
        return top + (1.0 - yn) * h;
    };

    const double y0 = yToPixel(0.0);

    // Preamp column separator and subtle tint (BandEq mode only)
    const double preSepX = left + 1.0 / static_cast<double>(kBandSliderCount) * static_cast<double>(w);
    if (m_plotMode == PlotMode::BandEq) {
        p.fillRect(QRectF(left, top, preSepX - left, h), QColor(255, 175, 40, 8));
        p.setPen(QPen(QColor(60, 60, 68), 1, Qt::DotLine));
        p.drawLine(QPointF(preSepX, top), QPointF(preSepX, bottom));
    }

    // Audio passband highlight (BandEq mode only)
    if (m_hasPassband && m_plotMode == PlotMode::BandEq && m_passbandHighHz > m_passbandLowHz) {
        const double xLo = left + bandPositionToX(bandHzToPosition(m_passbandLowHz), w);
        const double xHi = left + bandPositionToX(bandHzToPosition(m_passbandHighHz), w);
        const double clampedXLo = std::max(preSepX, xLo);
        const double clampedXHi = std::min(left + static_cast<double>(w), xHi);
        if (clampedXHi > clampedXLo) {
            QLinearGradient pbGrad(clampedXLo, top, clampedXLo, bottom);
            pbGrad.setColorAt(0.0, QColor(0, 160, 240, 32));
            pbGrad.setColorAt(1.0, QColor(0, 120, 200, 12));
            p.fillRect(QRectF(clampedXLo, top, clampedXHi - clampedXLo, h), pbGrad);

            p.setPen(QPen(QColor(0, 190, 255, 120), 1, Qt::DashLine));
            p.drawLine(QPointF(clampedXLo, top), QPointF(clampedXLo, bottom));
            p.drawLine(QPointF(clampedXHi, top), QPointF(clampedXHi, bottom));

            p.setFont(QFont(QStringLiteral("Sans Serif"), 7));
            p.setPen(QColor(0, 210, 255, 180));
            const QString pbLabel = QStringLiteral("%1–%2 Hz").arg(static_cast<int>(m_passbandLowHz)).arg(static_cast<int>(m_passbandHighHz));
            p.drawText(QRectF(clampedXLo, top + 1, clampedXHi - clampedXLo, 12), Qt::AlignCenter, pbLabel);
        }
    }

    // Horizontal grid lines (0 dB dashed, plus/minus step lines)
    p.setPen(QPen(QColor(80, 80, 88), 1, Qt::DashLine));
    p.drawLine(QPointF(left, y0), QPointF(left + w, y0));

    p.setFont(QFont(QStringLiteral("Sans Serif"), 6));
    p.setPen(QColor(90, 90, 95));
    p.drawText(QRectF(left + w - 24, y0 - 5, 22, 10), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("0"));

    if (m_plotMode == PlotMode::Cfc) {
        p.setPen(QPen(QColor(42, 42, 46), 1, Qt::DotLine));
        p.drawLine(QPointF(left, yToPixel(16.0)), QPointF(left + w, yToPixel(16.0)));
        p.drawLine(QPointF(left, yToPixel(8.0)), QPointF(left + w, yToPixel(8.0)));
        p.drawLine(QPointF(left, yToPixel(-8.0)), QPointF(left + w, yToPixel(-8.0)));
        p.drawLine(QPointF(left, yToPixel(-16.0)), QPointF(left + w, yToPixel(-16.0)));

        p.setPen(QColor(90, 90, 95));
        p.drawText(QRectF(left + w - 24, yToPixel(16.0) - 5, 22, 10), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("+16"));
        p.drawText(QRectF(left + w - 24, yToPixel(-16.0) - 5, 22, 10), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("-16"));
    } else {
        p.setPen(QPen(QColor(42, 42, 46), 1, Qt::DotLine));
        p.drawLine(QPointF(left, yToPixel(12.0)), QPointF(left + w, yToPixel(12.0)));
        p.drawLine(QPointF(left, yToPixel(6.0)), QPointF(left + w, yToPixel(6.0)));
        p.drawLine(QPointF(left, yToPixel(-6.0)), QPointF(left + w, yToPixel(-6.0)));
        p.drawLine(QPointF(left, yToPixel(-12.0)), QPointF(left + w, yToPixel(-12.0)));

        p.setPen(QColor(90, 90, 95));
        p.drawText(QRectF(left + w - 24, yToPixel(12.0) - 5, 22, 10), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("+12"));
        p.drawText(QRectF(left + w - 24, yToPixel(-12.0) - 5, 22, 10), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("-12"));
    }

    // Vertical guides at band centers
    if (m_plotMode != PlotMode::Generic) {
        p.setPen(QPen(QColor(42, 42, 46), 1));
        const int startIdx = (m_plotMode == PlotMode::BandEq) ? 1 : 0;
        for (int i = startIdx; i < count; ++i) {
            const double x = left + (i + 0.5) / static_cast<double>(count) * static_cast<double>(w);
            p.drawLine(QPointF(x, top), QPointF(x, bottom));
        }
    }

    p.setRenderHint(QPainter::Antialiasing, true);

    // Response Curve & Area Fill
    if (m_plotMode == PlotMode::BandEq || m_plotMode == PlotMode::Cfc) {
        // Build smooth Catmull-Rom cubic spline directly passing through all drag points
        QVector<QPointF> pts;
        pts.reserve(count + 2);

        // Left edge anchor
        const double firstDb = (m_bandGains.size() > 0) ? static_cast<double>(m_bandGains.at(0)) : 0.0;
        pts.append(QPointF(left, yToPixel(firstDb)));

        for (int i = 0; i < count; ++i) {
            const double x = left + (i + 0.5) / static_cast<double>(count) * static_cast<double>(w);
            const double valDb = (i < m_bandGains.size()) ? static_cast<double>(m_bandGains.at(i)) : 0.0;
            pts.append(QPointF(x, yToPixel(valDb)));
        }

        // Right edge anchor
        const double lastDb = (m_bandGains.size() >= count) ? static_cast<double>(m_bandGains.at(count - 1)) : 0.0;
        pts.append(QPointF(left + w, yToPixel(lastDb)));

        QPainterPath splinePath;
        splinePath.moveTo(pts[0]);

        const int numSegments = pts.size() - 1;
        for (int i = 0; i < numSegments; ++i) {
            const QPointF p0 = (i == 0) ? pts[0] : pts[i - 1];
            const QPointF p1 = pts[i];
            const QPointF p2 = pts[i + 1];
            const QPointF p3 = (i + 2 < pts.size()) ? pts[i + 2] : pts[i + 1];

            const QPointF t1 = (p2 - p0) * 0.5;
            const QPointF t2 = (p3 - p1) * 0.5;

            const QPointF c1 = p1 + t1 / 3.0;
            const QPointF c2 = p2 - t2 / 3.0;

            splinePath.cubicTo(c1, c2, p2);
        }

        // Area fill under the curve to the 0 dB baseline
        QPainterPath areaPath = splinePath;
        areaPath.lineTo(left + w, y0);
        areaPath.lineTo(left, y0);
        areaPath.closeSubpath();

        QColor fillTop = m_accentColor; fillTop.setAlpha(45);
        QColor fillMid = m_accentColor; fillMid.setAlpha(20);
        QColor fillBot = m_accentColor; fillBot.setAlpha(5);

        QLinearGradient areaGrad(0, top, 0, bottom);
        areaGrad.setColorAt(0.0, fillTop);
        areaGrad.setColorAt(0.5, fillMid);
        areaGrad.setColorAt(1.0, fillBot);
        p.fillPath(areaPath, areaGrad);

        // Smooth curve line passing through every drag point
        p.setPen(QPen(m_accentColor, 2.2));
        p.drawPath(splinePath);
    } else if (m_x.size() >= 2 && m_x.size() == m_y.size()) {
        double xmin = m_x.first(), xmax = m_x.first();
        for (int j = 0; j < m_x.size(); ++j) {
            xmin = std::min(xmin, m_x.at(j));
            xmax = std::max(xmax, m_x.at(j));
        }
        if (xmax > xmin) {
            QPolygonF poly;
            for (int i = 0; i < m_x.size(); ++i) {
                const double xn = (m_x.at(i) - xmin) / (xmax - xmin);
                const double xPixel = left + xn * w;
                poly.append(QPointF(xPixel, yToPixel(m_y.at(i) + m_preampDb)));
            }
            if (poly.size() >= 2) {
                QPainterPath areaPath;
                areaPath.moveTo(poly.first().x(), y0);
                for (const QPointF &pt : poly)
                    areaPath.lineTo(pt);
                areaPath.lineTo(poly.last().x(), y0);
                areaPath.closeSubpath();

                QColor fillTop = m_accentColor; fillTop.setAlpha(45);
                QColor fillMid = m_accentColor; fillMid.setAlpha(20);
                QColor fillBot = m_accentColor; fillBot.setAlpha(5);

                QLinearGradient areaGrad(0, top, 0, bottom);
                areaGrad.setColorAt(0.0, fillTop);
                areaGrad.setColorAt(0.5, fillMid);
                areaGrad.setColorAt(1.0, fillBot);
                p.fillPath(areaPath, areaGrad);

                p.setPen(QPen(m_accentColor, 2.0));
                p.drawPolyline(poly);
            }
        }
    }

    // Draggable Control Nodes
    if (m_interactive && m_plotMode != PlotMode::Generic) {
        for (int i = 0; i < count; ++i) {
            const double x = left + (i + 0.5) / static_cast<double>(count) * static_cast<double>(w);
            const double valDb = (i < m_bandGains.size()) ? static_cast<double>(m_bandGains.at(i)) : 0.0;
            const double y = yToPixel(valDb);

            const bool isPre = (m_plotMode == PlotMode::BandEq && i == 0);
            const bool isHovered = (i == m_hoveredBand || i == m_draggedBand);
            const QColor baseColor = isPre ? QColor(255, 175, 40) : m_accentColor;
            QColor haloColor = baseColor;
            haloColor.setAlpha(75);

            if (isHovered) {
                p.setBrush(haloColor);
                p.setPen(Qt::NoPen);
                p.drawEllipse(QPointF(x, y), 9.0, 9.0);

                p.setBrush(baseColor);
                p.setPen(QPen(Qt::white, 2.0));
                p.drawEllipse(QPointF(x, y), 5.5, 5.5);

                p.setBrush(Qt::white);
                p.setPen(Qt::NoPen);
                p.drawEllipse(QPointF(x, y), 2.5, 2.5);
            } else {
                p.setBrush(baseColor);
                p.setPen(QPen(QColor(230, 235, 245), 1.5));
                p.drawEllipse(QPointF(x, y), 4.5, 4.5);

                p.setBrush(QColor(24, 24, 28));
                p.setPen(Qt::NoPen);
                p.drawEllipse(QPointF(x, y), 2.0, 2.0);
            }
        }

        // Floating Tooltip Badge over active node
        const int activeIdx = (m_draggedBand >= 0) ? m_draggedBand : m_hoveredBand;
        if (activeIdx >= 0 && activeIdx < count) {
            const double x = left + (activeIdx + 0.5) / static_cast<double>(count) * static_cast<double>(w);
            const int valDb = (activeIdx < m_bandGains.size()) ? m_bandGains.at(activeIdx) : 0;
            const double y = yToPixel(valDb);

            QString badgeText;
            if (m_plotMode == PlotMode::BandEq) {
                badgeText = (activeIdx == 0)
                    ? QStringLiteral("Pre: %1%2 dB").arg(valDb > 0 ? "+" : "").arg(valDb)
                    : QStringLiteral("%1 Hz: %2%3 dB").arg(bandLabel(activeIdx - 1)).arg(valDb > 0 ? "+" : "").arg(valDb);
            } else {
                badgeText = QStringLiteral("%1 Hz: %2%3 dB").arg(cfcBandLabel(activeIdx)).arg(valDb > 0 ? "+" : "").arg(valDb);
            }

            p.setFont(QFont(QStringLiteral("Sans Serif"), 8, QFont::Bold));
            const QFontMetrics fm = p.fontMetrics();
            const int bw = fm.horizontalAdvance(badgeText) + 12;
            const int bh = fm.height() + 4;

            double pillY = y - bh - 8;
            if (pillY < top + 2)
                pillY = y + 10;
            double pillX = qBound(static_cast<double>(left + 2), x - bw / 2.0, static_cast<double>(left + w - bw - 2));

            const QRectF pillRect(pillX, pillY, bw, bh);
            p.setBrush(QColor(18, 20, 24, 235));
            p.setPen(QPen(m_accentColor, 1.2));
            p.drawRoundedRect(pillRect, 4, 4);

            p.setPen(Qt::white);
            p.drawText(pillRect, Qt::AlignCenter, badgeText);
        }
    }

    // Bottom frequency labels
    if (m_plotMode != PlotMode::Generic) {
        p.setFont(QFont(QStringLiteral("Sans Serif"), 7));
        for (int i = 0; i < count; ++i) {
            const double x = left + (i + 0.5) / static_cast<double>(count) * static_cast<double>(w);
            const QRect textRect(static_cast<int>(x - 16), bottom + 1, 32, labelH);

            const bool isActive = (i == m_hoveredBand || i == m_draggedBand);
            if (isActive) {
                p.setPen((m_plotMode == PlotMode::BandEq && i == 0) ? QColor(255, 185, 60) : m_accentColor);
                p.setFont(QFont(QStringLiteral("Sans Serif"), 7, QFont::Bold));
            } else {
                p.setPen(QColor(140, 140, 145));
                p.setFont(QFont(QStringLiteral("Sans Serif"), 7));
            }

            QString label;
            if (m_plotMode == PlotMode::BandEq) {
                label = (i == 0) ? QStringLiteral("Pre") : QString::fromLatin1(bandLabel(i - 1));
            } else {
                label = QString::fromLatin1(cfcBandLabel(i));
            }
            p.drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, label);
        }
    }
}

void EqCurvePlot::mousePressEvent(QMouseEvent *event)
{
    if (!m_interactive || m_plotMode == PlotMode::Generic) {
        QWidget::mousePressEvent(event);
        return;
    }
    const int band = hitTestBand(event->position());
    if (band >= 0 && band < m_bandGains.size()) {
        if (event->button() == Qt::RightButton) {
            m_bandGains[band] = 0;
            emit bandGainChanged(band, 0);
            update();
            event->accept();
            return;
        } else if (event->button() == Qt::LeftButton) {
            m_draggedBand = band;
            m_isDragging = true;
            m_hoveredBand = band;

            const int labelH = 16;
            const int top = 4;
            const int bottom = height() - 4 - labelH;
            const int h = bottom - top;
            double ymin = minGainDb(), ymax = maxGainDb();
            for (int g : m_bandGains) {
                ymin = std::min(ymin, static_cast<double>(g));
                ymax = std::max(ymax, static_cast<double>(g));
            }
            for (double v : m_y) {
                ymin = std::min(ymin, v + m_preampDb);
                ymax = std::max(ymax, v + m_preampDb);
            }
            ymin -= 2.0;
            ymax += 2.0;

            const int newGain = static_cast<int>(pixelToYDb(event->position().y(), ymin, ymax, top, h));
            if (newGain != m_bandGains[band]) {
                m_bandGains[band] = newGain;
                emit bandGainChanged(band, newGain);
            }
            update();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void EqCurvePlot::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_interactive || m_plotMode == PlotMode::Generic) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    if (m_isDragging && m_draggedBand >= 0 && m_draggedBand < m_bandGains.size()) {
        const int labelH = 16;
        const int top = 4;
        const int bottom = height() - 4 - labelH;
        const int h = bottom - top;
        double ymin = minGainDb(), ymax = maxGainDb();
        for (int g : m_bandGains) {
            ymin = std::min(ymin, static_cast<double>(g));
            ymax = std::max(ymax, static_cast<double>(g));
        }
        for (double v : m_y) {
            ymin = std::min(ymin, v + m_preampDb);
            ymax = std::max(ymax, v + m_preampDb);
        }
        ymin -= 2.0;
        ymax += 2.0;

        const int newGain = static_cast<int>(pixelToYDb(event->position().y(), ymin, ymax, top, h));
        if (newGain != m_bandGains[m_draggedBand]) {
            m_bandGains[m_draggedBand] = newGain;
            emit bandGainChanged(m_draggedBand, newGain);
        }
        update();
        event->accept();
        return;
    }

    const int hovered = hitTestBand(event->position());
    if (hovered != m_hoveredBand) {
        m_hoveredBand = hovered;
        setCursor(m_hoveredBand >= 0 ? Qt::SizeVerCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void EqCurvePlot::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isDragging) {
        m_isDragging = false;
        m_draggedBand = -1;
        update();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void EqCurvePlot::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_interactive || m_plotMode == PlotMode::Generic) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    const int band = hitTestBand(event->position());
    if (band >= 0 && band < m_bandGains.size()) {
        m_bandGains[band] = 0;
        emit bandGainChanged(band, 0);
        update();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void EqCurvePlot::wheelEvent(QWheelEvent *event)
{
    if (!m_interactive || m_plotMode == PlotMode::Generic) {
        QWidget::wheelEvent(event);
        return;
    }
    const int band = (m_hoveredBand >= 0) ? m_hoveredBand : hitTestBand(event->position());
    if (band >= 0 && band < m_bandGains.size()) {
        const int delta = (event->angleDelta().y() > 0) ? 1 : -1;
        const int newGain = qBound(static_cast<int>(minGainDb()), m_bandGains[band] + delta, static_cast<int>(maxGainDb()));
        if (newGain != m_bandGains[band]) {
            m_bandGains[band] = newGain;
            emit bandGainChanged(band, newGain);
            update();
        }
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

void EqCurvePlot::leaveEvent(QEvent *event)
{
    if (!m_isDragging && m_hoveredBand != -1) {
        m_hoveredBand = -1;
        setCursor(Qt::ArrowCursor);
        update();
    }
    QWidget::leaveEvent(event);
}
