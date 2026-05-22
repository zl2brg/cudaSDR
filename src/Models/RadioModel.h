#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include <QObject>
#include <QList>
#include <QColor>
#include "SliceModel.h"
#include "Settings/DisplayConfig.h"

class RadioTelemetry;

class RadioModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(QString hardwareType READ hardwareType WRITE setHardwareType NOTIFY hardwareTypeChanged)
    Q_PROPERTY(TPanadapterColors colors READ panadapterColors WRITE setPanadapterColors NOTIFY colorsChanged)

public:
    explicit RadioModel(QObject *parent = nullptr);
    ~RadioModel();

    bool connected() const { return m_connected; }
    void setConnected(bool connected);

    int sampleRate() const { return m_sampleRate; }
    void setSampleRate(int rate);

    QString hardwareType() const { return m_hardwareType; }
    void setHardwareType(const QString &type);

    TPanadapterColors panadapterColors() const { return m_colors; }
    void setPanadapterColors(const TPanadapterColors &colors);

    QList<SliceModel*> slices() const { return m_slices; }
    void addSlice(SliceModel *slice);
    void removeSlice(SliceModel *slice);

    RadioTelemetry* telemetry() const { return m_telemetry; }

signals:
    void connectedChanged(bool connected);
    void sampleRateChanged(int rate);
    void hardwareTypeChanged(const QString &type);
    void colorsChanged();

private:
    bool m_connected = false;
    int m_sampleRate = 48000;
    QString m_hardwareType = "Unknown";
    TPanadapterColors m_colors;
    QList<SliceModel*> m_slices;
    RadioTelemetry* m_telemetry = nullptr;
};

#endif // RADIOMODEL_H
