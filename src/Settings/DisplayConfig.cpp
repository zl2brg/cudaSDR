#include "DisplayConfig.h"
#include <QSettings>

DisplayConfig::DisplayConfig(QObject *parent)
    : QObject(parent)
    , m_spectrumSize(4096)
    , m_dBmDistScaleMin(-20)
    , m_dBmDistScaleMax(100)
    , m_sMeterHoldTime(2000)
    , m_showPanadapterSMeter(true)
    , m_showPanadapterFreq(true)
{
    // Default colors
    m_colors.panBackgroundColor = QColor(102, 69, 8);
    m_colors.waterfallColor = QColor(246, 146, 6);
    m_colors.panLineColor = QColor(246, 164, 76);
    m_colors.panLineFilledColor = QColor(246, 159, 7);
    m_colors.panSolidTopColor = QColor(230, 246, 204);
    m_colors.panSolidBottomColor = QColor(102, 96, 8);
    m_colors.wideBandLineColor = QColor(73, 111, 7);
    m_colors.wideBandFilledColor = QColor(137, 172, 62);
    m_colors.wideBandSolidTopColor = QColor(236, 38, 16);
    m_colors.wideBandSolidBottomColor = QColor(232, 134, 29);
    m_colors.distanceLineColor = QColor(246, 7, 19);
    m_colors.distanceLineFilledColor = QColor(232, 29, 86);
    m_colors.panCenterLineColor = QColor(80, 180, 240, 180);
    m_colors.gridLineColor = QColor(7, 96, 96);
    m_colors.panFilterColor = QColor(150, 150, 150, 100);
}

void DisplayConfig::setSpectrumSize(int size) {
    if (m_spectrumSize != size) {
        m_spectrumSize = size;
        emit spectrumSizeChanged(m_spectrumSize);
    }
}

void DisplayConfig::setdBmDistScaleMin(qreal val) {
    if (m_dBmDistScaleMin != val) {
        m_dBmDistScaleMin = val;
        emit dBmDistScaleMinChanged(m_dBmDistScaleMin);
    }
}

void DisplayConfig::setdBmDistScaleMax(qreal val) {
    if (m_dBmDistScaleMax != val) {
        m_dBmDistScaleMax = val;
        emit dBmDistScaleMaxChanged(m_dBmDistScaleMax);
    }
}

void DisplayConfig::setSMeterHoldTime(int time) {
    if (m_sMeterHoldTime != time) {
        m_sMeterHoldTime = time;
        emit sMeterHoldTimeChanged(m_sMeterHoldTime);
    }
}

void DisplayConfig::setShowPanadapterSMeter(bool show) {
    if (m_showPanadapterSMeter != show) {
        m_showPanadapterSMeter = show;
        emit showPanadapterSMeterChanged(m_showPanadapterSMeter);
    }
}

void DisplayConfig::setShowPanadapterFreq(bool show) {
    if (m_showPanadapterFreq != show) {
        m_showPanadapterFreq = show;
        emit showPanadapterFreqChanged(m_showPanadapterFreq);
    }
}

void DisplayConfig::setPanadapterColors(const TPanadapterColors &colors) {
    // Basic assignment for now. Could do detailed check if needed.
    m_colors = colors;
    emit panadapterColorsChanged();
}

QString DisplayConfig::colorToString(const QColor &color) {
    return color.name(QColor::HexArgb);
}

QColor DisplayConfig::stringToColor(const QString &str, const QColor &def) {
    QColor c(str);
    return c.isValid() ? c : def;
}

void DisplayConfig::load(const QJsonObject &json) {
    if (json.contains("spectrumSize")) setSpectrumSize(json["spectrumSize"].toInt());
    if (json.contains("dBmDistScaleMin")) setdBmDistScaleMin(json["dBmDistScaleMin"].toDouble());
    if (json.contains("dBmDistScaleMax")) setdBmDistScaleMax(json["dBmDistScaleMax"].toDouble());
    if (json.contains("sMeterHoldTime")) setSMeterHoldTime(json["sMeterHoldTime"].toInt());
    if (json.contains("showPanadapterSMeter")) setShowPanadapterSMeter(json["showPanadapterSMeter"].toBool());
    if (json.contains("showPanadapterFreq")) setShowPanadapterFreq(json["showPanadapterFreq"].toBool());

    if (json.contains("colors")) {
        QJsonObject colors = json["colors"].toObject();
        TPanadapterColors next = m_colors;
        if (colors.contains("panBackground")) next.panBackgroundColor = stringToColor(colors["panBackground"].toString(), next.panBackgroundColor);
        if (colors.contains("waterfall")) next.waterfallColor = stringToColor(colors["waterfall"].toString(), next.waterfallColor);
        if (colors.contains("panLine")) next.panLineColor = stringToColor(colors["panLine"].toString(), next.panLineColor);
        if (colors.contains("panLineFilled")) next.panLineFilledColor = stringToColor(colors["panLineFilled"].toString(), next.panLineFilledColor);
        if (colors.contains("panSolidTop")) next.panSolidTopColor = stringToColor(colors["panSolidTop"].toString(), next.panSolidTopColor);
        if (colors.contains("panSolidBottom")) next.panSolidBottomColor = stringToColor(colors["panSolidBottom"].toString(), next.panSolidBottomColor);
        if (colors.contains("wideBandLine")) next.wideBandLineColor = stringToColor(colors["wideBandLine"].toString(), next.wideBandLineColor);
        if (colors.contains("wideBandFilled")) next.wideBandFilledColor = stringToColor(colors["wideBandFilled"].toString(), next.wideBandFilledColor);
        if (colors.contains("wideBandSolidTop")) next.wideBandSolidTopColor = stringToColor(colors["wideBandSolidTop"].toString(), next.wideBandSolidTopColor);
        if (colors.contains("wideBandSolidBottom")) next.wideBandSolidBottomColor = stringToColor(colors["wideBandSolidBottom"].toString(), next.wideBandSolidBottomColor);
        if (colors.contains("distanceLine")) next.distanceLineColor = stringToColor(colors["distanceLine"].toString(), next.distanceLineColor);
        if (colors.contains("distanceLineFilled")) next.distanceLineFilledColor = stringToColor(colors["distanceLineFilled"].toString(), next.distanceLineFilledColor);
        if (colors.contains("panCenterLine")) next.panCenterLineColor = stringToColor(colors["panCenterLine"].toString(), next.panCenterLineColor);
        if (colors.contains("gridLine")) next.gridLineColor = stringToColor(colors["gridLine"].toString(), next.gridLineColor);
        if (colors.contains("panFilter")) next.panFilterColor = stringToColor(colors["panFilter"].toString(), next.panFilterColor);
        setPanadapterColors(next);
    }
}

void DisplayConfig::save(QJsonObject &json) const {
    json["spectrumSize"] = m_spectrumSize;
    json["dBmDistScaleMin"] = m_dBmDistScaleMin;
    json["dBmDistScaleMax"] = m_dBmDistScaleMax;
    json["sMeterHoldTime"] = m_sMeterHoldTime;
    json["showPanadapterSMeter"] = m_showPanadapterSMeter;
    json["showPanadapterFreq"] = m_showPanadapterFreq;

    QJsonObject colors;
    colors["panBackground"] = colorToString(m_colors.panBackgroundColor);
    colors["waterfall"] = colorToString(m_colors.waterfallColor);
    colors["panLine"] = colorToString(m_colors.panLineColor);
    colors["panLineFilled"] = colorToString(m_colors.panLineFilledColor);
    colors["panSolidTop"] = colorToString(m_colors.panSolidTopColor);
    colors["panSolidBottom"] = colorToString(m_colors.panSolidBottomColor);
    colors["wideBandLine"] = colorToString(m_colors.wideBandLineColor);
    colors["wideBandFilled"] = colorToString(m_colors.wideBandFilledColor);
    colors["wideBandSolidTop"] = colorToString(m_colors.wideBandSolidTopColor);
    colors["wideBandSolidBottom"] = colorToString(m_colors.wideBandSolidBottomColor);
    colors["distanceLine"] = colorToString(m_colors.distanceLineColor);
    colors["distanceLineFilled"] = colorToString(m_colors.distanceLineFilledColor);
    colors["panCenterLine"] = colorToString(m_colors.panCenterLineColor);
    colors["gridLine"] = colorToString(m_colors.gridLineColor);
    colors["panFilter"] = colorToString(m_colors.panFilterColor);
    json["colors"] = colors;
}

void DisplayConfig::loadIni(QSettings *settings) {
    int value;

    value = settings->value("graphics/dBmDistScaleMin", -20).toInt();
    if ((value < -200) || (value > 0)) value = -20;
    setdBmDistScaleMin(static_cast<qreal>(value));

    value = settings->value("graphics/dBmDistScaleMax", 100).toInt();
    if ((value < -100) || (value > 200)) value = 100;
    setdBmDistScaleMax(static_cast<qreal>(value));

    value = settings->value("graphics/sMeterHoldTime", 2000).toInt();
    if ((value < 0) || (value > 10000)) value = 2000;
    setSMeterHoldTime(value);

    setShowPanadapterSMeter(settings->value("graphics/showPanadapterSMeter", true).toBool());
    setShowPanadapterFreq(settings->value("graphics/showPanadapterFreq", true).toBool());

    // Color loading
    TPanadapterColors colors = m_colors;
    QColor color;

    color = settings->value("colors/panBackground", QColor(102, 69, 8)).value<QColor>();
    if (color.isValid()) colors.panBackgroundColor = color;

    color = settings->value("colors/waterfall", QColor(246, 146, 6)).value<QColor>();
    if (color.isValid()) colors.waterfallColor = color;

    color = settings->value("colors/panLine", QColor(246, 164, 76)).value<QColor>();
    if (color.isValid()) colors.panLineColor = color;

    color = settings->value("colors/panLineFilled", QColor(246, 159, 7)).value<QColor>();
    if (color.isValid()) colors.panLineFilledColor = color;

    color = settings->value("colors/panSolidTop", QColor(230, 246, 204)).value<QColor>();
    if (color.isValid()) colors.panSolidTopColor = color;

    color = settings->value("colors/panSolidBottom", QColor(102, 96, 8)).value<QColor>();
    if (color.isValid()) colors.panSolidBottomColor = color;

    color = settings->value("colors/panWideBandLine", QColor(73, 111, 7)).value<QColor>();
    if (color.isValid()) colors.wideBandLineColor = color;

    color = settings->value("colors/panWideBandFilled", QColor(137, 172, 62)).value<QColor>();
    if (color.isValid()) colors.wideBandFilledColor = color;

    color = settings->value("colors/panWideBandSolidTop", QColor(236, 38, 16)).value<QColor>();
    if (color.isValid()) colors.wideBandSolidTopColor = color;

    color = settings->value("colors/panWideBandSolidBottom", QColor(232, 134, 29)).value<QColor>();
    if (color.isValid()) colors.wideBandSolidBottomColor = color;

    color = settings->value("colors/distanceLine", QColor(246, 7, 19)).value<QColor>();
    if (color.isValid()) colors.distanceLineColor = color;

    color = settings->value("colors/distanceLineFilled", QColor(232, 29, 86)).value<QColor>();
    if (color.isValid()) colors.distanceLineFilledColor = color;

    color = settings->value("colors/panCenterLine", QColor(80, 180, 240, 180)).value<QColor>();
    if (color.isValid()) colors.panCenterLineColor = color;

    color = settings->value("colors/gridLine", QColor(7, 96, 96)).value<QColor>();
    if (color.isValid()) colors.gridLineColor = color;

    color = settings->value("colors/panFilter", QColor(150, 150, 150, 100)).value<QColor>();
    if (color.isValid()) colors.panFilterColor = color;

    setPanadapterColors(colors);
}

void DisplayConfig::saveIni(QSettings *settings) const {
    settings->setValue("graphics/dBmDistScaleMin",  m_dBmDistScaleMin);
    settings->setValue("graphics/dBmDistScaleMax",  m_dBmDistScaleMax);
    settings->setValue("graphics/sMeterHoldTime",   m_sMeterHoldTime);
    settings->setValue("graphics/showPanadapterSMeter", m_showPanadapterSMeter);
    settings->setValue("graphics/showPanadapterFreq", m_showPanadapterFreq);

    settings->setValue("colors/panBackground", m_colors.panBackgroundColor);
    settings->setValue("colors/waterfall", m_colors.waterfallColor);
    settings->setValue("colors/panLine", m_colors.panLineColor);
    settings->setValue("colors/panLineFilled", m_colors.panLineFilledColor);
    settings->setValue("colors/panSolidTop", m_colors.panSolidTopColor);
    settings->setValue("colors/panSolidBottom", m_colors.panSolidBottomColor);
    settings->setValue("colors/panWideBandLine", m_colors.wideBandLineColor);
    settings->setValue("colors/panWideBandFilled", m_colors.wideBandFilledColor);
    settings->setValue("colors/panWideBandSolidTop", m_colors.wideBandSolidTopColor);
    settings->setValue("colors/panWideBandSolidBottom", m_colors.wideBandSolidBottomColor);
    settings->setValue("colors/distanceLine", m_colors.distanceLineColor);
    settings->setValue("colors/distanceLineFilled", m_colors.distanceLineFilledColor);
    settings->setValue("colors/panCenterLine", m_colors.panCenterLineColor);
    settings->setValue("colors/gridLine", m_colors.gridLineColor);
    settings->setValue("colors/panFilter", m_colors.panFilterColor);
}
