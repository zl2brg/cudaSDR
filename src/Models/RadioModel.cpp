#include "RadioModel.h"
#include "RadioTelemetry.h"

RadioModel::RadioModel(QObject *parent)
    : QObject(parent)
    , m_telemetry(new RadioTelemetry(this, this))
{
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
        slice->deleteLater();
    }
}
