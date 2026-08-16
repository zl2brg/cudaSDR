#ifndef RADIOMODEL_H
#define RADIOMODEL_H

#include <QObject>
#include <QList>
#include <QColor>
#include "SliceModel.h"
#include "Settings/DisplayConfig.h"
#include "cusdr_settings.h"

class RadioTelemetry;
class BandPlanManager;
class DxClusterClient;

class RadioModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)
    Q_PROPERTY(int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
    Q_PROPERTY(int activeReceivers READ activeReceivers WRITE setActiveReceivers NOTIFY activeReceiversChanged)
    Q_PROPERTY(QString hardwareType READ hardwareType WRITE setHardwareType NOTIFY hardwareTypeChanged)
    Q_PROPERTY(TPanadapterColors colors READ panadapterColors WRITE setPanadapterColors NOTIFY colorsChanged)

public:
    explicit RadioModel(QObject *parent = nullptr);
    ~RadioModel();

    bool connected() const { return m_connected; }
    void setConnected(bool connected);

    int sampleRate() const { return m_sampleRate; }
    void setSampleRate(int rate);

    /** Hardware DDC count in use (not slices().size(), which is a preallocated pool). */
    int activeReceivers() const { return m_activeReceivers; }
    void setActiveReceivers(int count);

    QString hardwareType() const { return m_hardwareType; }
    void setHardwareType(const QString &type);

    TPanadapterColors panadapterColors() const { return m_colors; }
    void setPanadapterColors(const TPanadapterColors &colors);

    /** Live Protocol 1/2 TX C&C snapshot (Alex, drive, mox/ptt, atten, …). */
    TCCParameterTx& txParams() { return m_tx; }
    const TCCParameterTx& txParams() const { return m_tx; }

    QList<SliceModel*> slices() const { return m_slices; }
    void addSlice(SliceModel *slice);
    void removeSlice(SliceModel *slice);

    /**
     * Slice used for TX dial / Alex LPF when split (TCI VFO-B) is active.
     * -1 means TX follows the primary RX slice (index 0 / current receiver).
     */
    int txSliceIndex() const { return m_txSliceIndex; }
    void setTxSliceIndex(int index);

    /** Frequency the protocols should encode for TX / Alex. */
    qint64 effectiveTxFrequency() const;

    RadioTelemetry* telemetry() const { return m_telemetry; }
    BandPlanManager* bandPlan() const { return m_bandPlan; }
    DxClusterClient* dxClusterClient() const { return m_dxClusterClient; }

signals:
    void connectedChanged(bool connected);
    void sampleRateChanged(int rate);
    void activeReceiversChanged(int count);
    void hardwareTypeChanged(const QString &type);
    void colorsChanged();
    void txSliceIndexChanged(int index);

private:
    bool m_connected = false;
    int m_sampleRate = 48000;
    int m_activeReceivers = 1;
    int m_txSliceIndex = -1;
    QString m_hardwareType = "Unknown";
    TPanadapterColors m_colors;
    TCCParameterTx m_tx{};
    QList<SliceModel*> m_slices;
    RadioTelemetry* m_telemetry = nullptr;
    BandPlanManager* m_bandPlan = nullptr;
    DxClusterClient* m_dxClusterClient = nullptr;
    QTimer* m_spotPruneTimer = nullptr;
};

#endif // RADIOMODEL_H
