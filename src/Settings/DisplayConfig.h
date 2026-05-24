#ifndef DISPLAYCONFIG_H
#define DISPLAYCONFIG_H

#include <QObject>
#include <QColor>
#include <QJsonObject>
#include <QJsonArray>

typedef struct t_panadapterColors {
    QColor panBackgroundColor;
    QColor waterfallColor;
    QColor panLineColor;
    QColor panLineFilledColor;
    QColor panSolidTopColor;
    QColor panSolidBottomColor;
    QColor wideBandLineColor;
    QColor wideBandFilledColor;
    QColor wideBandSolidTopColor;
    QColor wideBandSolidBottomColor;
    QColor distanceLineColor;
    QColor distanceLineFilledColor;
    QColor panCenterLineColor;
    QColor gridLineColor;
    QColor panFilterColor;
} TPanadapterColors;

class DisplayConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(int spectrumSize READ spectrumSize WRITE setSpectrumSize NOTIFY spectrumSizeChanged)
    Q_PROPERTY(qreal dBmDistScaleMin READ dBmDistScaleMin WRITE setdBmDistScaleMin NOTIFY dBmDistScaleMinChanged)
    Q_PROPERTY(qreal dBmDistScaleMax READ dBmDistScaleMax WRITE setdBmDistScaleMax NOTIFY dBmDistScaleMaxChanged)
    Q_PROPERTY(int sMeterHoldTime READ sMeterHoldTime WRITE setSMeterHoldTime NOTIFY sMeterHoldTimeChanged)

public:
    explicit DisplayConfig(QObject *parent = nullptr);

    int spectrumSize() const { return m_spectrumSize; }
    void setSpectrumSize(int size);

    qreal dBmDistScaleMin() const { return m_dBmDistScaleMin; }
    void setdBmDistScaleMin(qreal val);

    qreal dBmDistScaleMax() const { return m_dBmDistScaleMax; }
    void setdBmDistScaleMax(qreal val);

    int sMeterHoldTime() const { return m_sMeterHoldTime; }
    void setSMeterHoldTime(int time);

    TPanadapterColors panadapterColors() const { return m_colors; }
    void setPanadapterColors(const TPanadapterColors &colors);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    // Helper to convert QColor to JSON string
    static QString colorToString(const QColor &color);
    static QColor stringToColor(const QString &str, const QColor &def = Qt::black);

signals:
    void spectrumSizeChanged(int size);
    void dBmDistScaleMinChanged(qreal val);
    void dBmDistScaleMaxChanged(qreal val);
    void sMeterHoldTimeChanged(int time);
    void panadapterColorsChanged();

private:
    int m_spectrumSize;
    qreal m_dBmDistScaleMin;
    qreal m_dBmDistScaleMax;
    int m_sMeterHoldTime;
    TPanadapterColors m_colors;
};

#endif // DISPLAYCONFIG_H
