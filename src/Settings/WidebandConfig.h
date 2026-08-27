#ifndef WIDEBANDCONFIG_H
#define WIDEBANDCONFIG_H

#include <QObject>
#include <QJsonObject>
#include "SettingsTypes.h"

class QSettings;

/**
 * Persistence DTO for the wideband panadapter. Runtime state stays on
 * Settings::m_widebandOptions; JSON load/save copies through this object.
 */
class WidebandConfig : public QObject {
    Q_OBJECT

public:
    explicit WidebandConfig(QObject *parent = nullptr);

    bool dataEnabled() const { return m_dataEnabled; }
    void setDataEnabled(bool enabled);

    bool displayEnabled() const { return m_displayEnabled; }
    void setDisplayEnabled(bool enabled);

    bool averaging() const { return m_averaging; }
    void setAveraging(bool enabled);

    int averagingCnt() const { return m_averagingCnt; }
    void setAveragingCnt(int count);

    qreal dBmScaleMin() const { return m_dBmScaleMin; }
    void setdBmScaleMin(qreal value);

    qreal dBmScaleMax() const { return m_dBmScaleMax; }
    void setdBmScaleMax(qreal value);

    PanGraphicsMode panMode() const { return m_panMode; }
    void setPanMode(PanGraphicsMode mode);

    void load(const QJsonObject &json);
    void save(QJsonObject &json) const;

    void loadIni(QSettings *settings);
    void saveIni(QSettings *settings) const;

signals:
    void dataEnabledChanged(bool enabled);
    void displayEnabledChanged(bool enabled);
    void averagingChanged(bool enabled);
    void averagingCntChanged(int count);
    void dBmScaleMinChanged(qreal value);
    void dBmScaleMaxChanged(qreal value);
    void panModeChanged(PanGraphicsMode mode);

private:
    bool m_dataEnabled = true;
    bool m_displayEnabled = false;
    bool m_averaging = true;
    int m_averagingCnt = 5;
    qreal m_dBmScaleMin = -140.0;
    qreal m_dBmScaleMax = -10.0;
    PanGraphicsMode m_panMode = Line;
};

#endif // WIDEBANDCONFIG_H
