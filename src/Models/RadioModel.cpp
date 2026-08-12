#include "RadioModel.h"
#include "RadioTelemetry.h"
#include "BandPlanManager.h"

#include <QDebug>

RadioModel::RadioModel(QObject *parent)
    : QObject(parent)
    , m_telemetry(new RadioTelemetry(this, this))
    , m_bandPlan(new BandPlanManager(this))
{
    // Allocation bars: SDR-Band-Plans International (CC0).
    // Spot markers: KiwiSDR default DX labels (LGPL/GPL project data) plus
    // Region 3 digimode dial freqs (fills FT8/FT4 gaps in the Kiwi set).
    if (!m_bandPlan->loadFromResource(QStringLiteral(":/bandplans/international.xml")))
        qWarning() << "RadioModel: failed to load international band plan resource";
    if (!m_bandPlan->loadKiwiDxFromResource(QStringLiteral(":/bandplans/kiwi-dx.json")))
        qWarning() << "RadioModel: failed to load Kiwi DX label database";
    {
        BandPlanManager digi;
        if (digi.loadSpotsFromResource(QStringLiteral(":/bandplans/digimode-spots.json")))
            m_bandPlan->mergeSpots(digi.spots());
    }
}

RadioModel::~RadioModel()
{
    qDeleteAll(m_slices);
}

void RadioModel::setConnected(bool connected) {
    if (m_connected == connected) return;
    m_connected = connected;
    emit connectedChanged(m_connected);
}

void RadioModel::setSampleRate(int rate) {
    if (m_sampleRate == rate) return;
    m_sampleRate = rate;
    emit sampleRateChanged(m_sampleRate);
}

void RadioModel::setActiveReceivers(int count) {
    if (count < 1)
        count = 1;
    if (m_activeReceivers == count) return;
    m_activeReceivers = count;
    emit activeReceiversChanged(m_activeReceivers);
}

void RadioModel::setHardwareType(const QString &type) {
    if (m_hardwareType == type) return;
    m_hardwareType = type;
    emit hardwareTypeChanged(m_hardwareType);
}

void RadioModel::setPanadapterColors(const TPanadapterColors &colors) {
    m_colors = colors;
    emit colorsChanged();
}

void RadioModel::addSlice(SliceModel *slice) {
    if (!slice || m_slices.contains(slice)) return;
    m_slices.append(slice);
}

void RadioModel::removeSlice(SliceModel *slice) {
    if (m_slices.removeOne(slice)) {
        if (m_txSliceIndex >= 0 && m_txSliceIndex >= m_slices.size()) {
            m_txSliceIndex = -1;
            emit txSliceIndexChanged(m_txSliceIndex);
        }
        slice->deleteLater();
    }
}

void RadioModel::setTxSliceIndex(int index) {
    if (index < -1)
        index = -1;
    if (index >= m_slices.size())
        index = -1;
    if (m_txSliceIndex == index)
        return;
    m_txSliceIndex = index;
    emit txSliceIndexChanged(m_txSliceIndex);
}

qint64 RadioModel::effectiveTxFrequency() const {
    if (m_txSliceIndex >= 0 && m_txSliceIndex < m_slices.size() && m_slices.at(m_txSliceIndex))
        return m_slices.at(m_txSliceIndex)->frequency();
    if (!m_slices.isEmpty() && m_slices.at(0))
        return m_slices.at(0)->frequency();
    return static_cast<qint64>(m_tx.txFrequency);
}
