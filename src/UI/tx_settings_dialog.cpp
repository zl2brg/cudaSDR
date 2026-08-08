#include "Util/AudioDeviceService.h"
#include "tx_settings_dialog.h"
#include "ui_tx_settings_dialog.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "AudioEngine/cusdr_audio_input.h"
#include <QSignalBlocker>
#include <QDebug>

namespace {
int findDeviceComboIndex(const QList<QAudioDevice> &devices, const QString &name, int offset)
{
    for (int i = 0; i < devices.size(); ++i) {
        if (devices.at(i).description() == name)
            return i + offset;
    }
    return -1;
}
}

tx_settings_dialog::tx_settings_dialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tx_settings_dialog),
    m_codec2ModeCombo(nullptr),
    m_currentReceiver(0)
{
    setContentsMargins(4, 0, 4, 0);
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->amCarrierLevel->setSliderPosition(0.5);
    ui->audioCompression->setSliderPosition(0);
    ui->fm_deviation->setValue(5);

    // Digital voice controls are inserted dynamically so older .ui files stay compatible.
    QGroupBox *digitalVoiceGroup = new QGroupBox("Digital Voice", this);
    QVBoxLayout *digitalVoiceLayout = new QVBoxLayout(digitalVoiceGroup);
    QLabel *codec2Label = new QLabel("FreeDV mode", digitalVoiceGroup);
    m_codec2ModeCombo = new QComboBox(this);
    m_codec2ModeCombo->setObjectName("codec2ModeCombo");

    digitalVoiceLayout->addWidget(codec2Label);
    digitalVoiceLayout->addWidget(m_codec2ModeCombo);
    ui->verticalLayoutScroll->insertWidget(2, digitalVoiceGroup);

    ui->groupBox->setContentsMargins(2,2,2,2);
    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);

    // Setup internal and external connections
    connect(AudioDeviceService::instance(), &AudioDeviceService::audioInputsChanged,
            this, &tx_settings_dialog::triggerRefreshDevices);

    connect(ui->audiodevlist, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        emit micInputDevChanged(index);
        if (index == 0)
            emit micInputSourceNameChanged("hpsdr-local");
        else
            emit micInputSourceNameChanged(ui->audiodevlist->itemText(index));
    });

    connect(ui->digitalAudioDevList, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        emit digitalAudioInputDevChanged(index);
        if (index == 0)
            emit digitalInputSourceNameChanged("none");
        else
            emit digitalInputSourceNameChanged(ui->digitalAudioDevList->itemText(index));
    });

    connect(m_codec2ModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            int mode = m_codec2ModeCombo->itemData(index).toInt();
            emit freeDVModeRequested(m_currentReceiver, mode);
        }
    });

    connect(ui->audioCompression, &QSlider::valueChanged, this, &tx_settings_dialog::audioCompressionRequested);
    connect(ui->amCarrierLevel, &QSlider::valueChanged, this, &tx_settings_dialog::amCarrierLevelRequested);
    connect(ui->fm_deviation, &QSpinBox::valueChanged, this, [this](int val) {
        emit fmDeviationRequested(val * 1000);
    });
    connect(ui->fmPre, &QCheckBox::toggled, this, &tx_settings_dialog::fmPreEmphasisRequested);
    connect(ui->phaseRotator, &QCheckBox::toggled, this, &tx_settings_dialog::phaseRotatorRequested);
    connect(ui->KeyerMode, &QComboBox::currentIndexChanged, this, &tx_settings_dialog::cwKeyerModeRequested);
    connect(ui->internal_keyer, &QCheckBox::stateChanged, this, [this](int state) {
        emit internalCwRequested(state == Qt::Checked);
    });
    connect(ui->keyer_reverse, &QCheckBox::stateChanged, this, [this](int state) {
        emit cwKeyReversedRequested(state == Qt::Checked);
    });
    connect(ui->keyer_spacing, &QCheckBox::stateChanged, this, [this](int state) {
        emit cwKeyerSpacingRequested(state == Qt::Checked);
    });
    connect(ui->keyer_speed, &QSpinBox::valueChanged, this, &tx_settings_dialog::cwKeyerSpeedRequested);
    connect(ui->ptt_delay, &QSpinBox::valueChanged, this, &tx_settings_dialog::cwPttDelayRequested);
    connect(ui->sidetone_freq, &QSpinBox::valueChanged, this, &tx_settings_dialog::cwSidetoneFreqRequested);
    connect(ui->sidetone_volume, &QSpinBox::valueChanged, this, &tx_settings_dialog::cwSidetoneVolumeRequested);
    connect(ui->cw_hangtime, &QSpinBox::valueChanged, this, &tx_settings_dialog::cwHangTimeRequested);
    connect(ui->weight, &QSlider::valueChanged, this, &tx_settings_dialog::cwKeyerWeightRequested);
}

tx_settings_dialog::~tx_settings_dialog()
{
    delete ui;
    disconnect(0, 0, 0);
}

void tx_settings_dialog::setAvailableCodec2Modes(const QList<int>& modes)
{
    const QSignalBlocker blocker(m_codec2ModeCombo);
    m_codec2ModeCombo->clear();
    for (int mode : modes) {
        if (m_codec2ModeStringResolver) {
            m_codec2ModeCombo->addItem(m_codec2ModeStringResolver(mode), mode);
        } else {
            m_codec2ModeCombo->addItem(QString::number(mode), mode);
        }
    }
}

void tx_settings_dialog::setCodec2ModeStringResolver(std::function<QString(int)> resolver)
{
    m_codec2ModeStringResolver = resolver;
}

void tx_settings_dialog::setAmCarrierLevel(double level)
{
    const QSignalBlocker blocker(ui->amCarrierLevel);
    ui->amCarrierLevel->setValue(static_cast<int>(level));
}

void tx_settings_dialog::setAudioCompression(double compression)
{
    const QSignalBlocker blocker(ui->audioCompression);
    ui->audioCompression->setValue(static_cast<int>(compression));
}

void tx_settings_dialog::setFmDeviation(int dev)
{
    const QSignalBlocker blocker(ui->fm_deviation);
    ui->fm_deviation->setValue(dev / 1000);
}

void tx_settings_dialog::setFmPreEmphasis(bool enabled)
{
    const QSignalBlocker blocker(ui->fmPre);
    ui->fmPre->setChecked(enabled);
}

void tx_settings_dialog::setPhaseRotator(bool enabled)
{
    const QSignalBlocker blocker(ui->phaseRotator);
    ui->phaseRotator->setChecked(enabled);
}

void tx_settings_dialog::setCwSidetoneFreq(int freq)
{
    const QSignalBlocker blocker(ui->sidetone_freq);
    ui->sidetone_freq->setValue(freq);
}

void tx_settings_dialog::setCwSidetoneVolume(int vol)
{
    const QSignalBlocker blocker(ui->sidetone_volume);
    ui->sidetone_volume->setValue(vol);
}

void tx_settings_dialog::setCwHangTime(int time)
{
    const QSignalBlocker blocker(ui->cw_hangtime);
    ui->cw_hangtime->setValue(time);
}

void tx_settings_dialog::setCwKeyerMode(int mode)
{
    const QSignalBlocker blocker(ui->KeyerMode);
    ui->KeyerMode->setCurrentIndex(mode);
}

void tx_settings_dialog::setInternalCw(bool val)
{
    const QSignalBlocker blocker(ui->internal_keyer);
    ui->internal_keyer->setChecked(val);
}

void tx_settings_dialog::setCwKeyReversed(bool val)
{
    const QSignalBlocker blocker(ui->keyer_reverse);
    ui->keyer_reverse->setChecked(val);
}

void tx_settings_dialog::setCwKeyerSpacing(bool val)
{
    const QSignalBlocker blocker(ui->keyer_spacing);
    ui->keyer_spacing->setChecked(val);
}

void tx_settings_dialog::setCwKeyerSpeed(int speed)
{
    const QSignalBlocker blocker(ui->keyer_speed);
    ui->keyer_speed->setValue(speed);
}

void tx_settings_dialog::setCwPttDelay(int delay)
{
    const QSignalBlocker blocker(ui->ptt_delay);
    ui->ptt_delay->setValue(delay);
}

void tx_settings_dialog::setCwKeyerWeight(int weight)
{
    const QSignalBlocker blocker(ui->weight);
    ui->weight->setValue(weight);
}

void tx_settings_dialog::setCurrentReceiver(int rx)
{
    m_currentReceiver = rx;
}

void tx_settings_dialog::setFreeDVMode(int rx, int mode)
{
    if (rx != m_currentReceiver) return;
    const QSignalBlocker blocker(m_codec2ModeCombo);
    const int idx = m_codec2ModeCombo->findData(mode);
    if (idx >= 0) {
        m_codec2ModeCombo->setCurrentIndex(idx);
    } else {
        m_codec2ModeCombo->setCurrentIndex(0);
    }
}

void tx_settings_dialog::triggerRefreshDevices()
{
    emit audioDevicesRefreshRequested();
}

void tx_settings_dialog::refreshAudioDevices(const QString& savedMicName, const QString& savedDigitalName)
{
    const QSignalBlocker micBlocker(ui->audiodevlist);
    const QSignalBlocker digBlocker(ui->digitalAudioDevList);

    const QString currentMic = ui->audiodevlist->currentText();
    const QString currentDig = ui->digitalAudioDevList->currentText();

    // Populate Mic List
    ui->audiodevlist->clear();
    ui->audiodevlist->addItem("HPSDR Mic Input");
    
    const QList<QAudioDevice> micInputs = TransmitAudioInput::availableAudioInputDevices();
    for (const QAudioDevice &deviceInfo : micInputs) {
        ui->audiodevlist->addItem(deviceInfo.description());
    }

    // Restore mic selection
    int micIndex = -1;
    if (!currentMic.isEmpty()) {
        micIndex = ui->audiodevlist->findText(currentMic);
    }
    
    if (micIndex < 0) {
        if (savedMicName == "hpsdr-local") {
            micIndex = 0;
        } else {
            micIndex = findDeviceComboIndex(micInputs, savedMicName, 1);
            if (micIndex < 0) {
                const QString defaultName = QMediaDevices::defaultAudioInput().description();
                micIndex = findDeviceComboIndex(micInputs, defaultName, 1);
            }
            if (micIndex < 0)
                micIndex = 0;
        }
    }
    ui->audiodevlist->setCurrentIndex(micIndex);

    // Populate digital audio input device list
    ui->digitalAudioDevList->clear();
    ui->digitalAudioDevList->addItem("None");
    for (const QAudioDevice &deviceInfo : micInputs) {
        ui->digitalAudioDevList->addItem(deviceInfo.description());
    }

    // Restore digital selection
    int digitalIndex = -1;
    if (!currentDig.isEmpty()) {
        digitalIndex = ui->digitalAudioDevList->findText(currentDig);
    }

    if (digitalIndex < 0) {
        if (savedDigitalName == "none") {
            digitalIndex = 0;
        } else {
            digitalIndex = findDeviceComboIndex(micInputs, savedDigitalName, 1);
            if (digitalIndex < 0) {
                const QString defaultName = QMediaDevices::defaultAudioInput().description();
                digitalIndex = findDeviceComboIndex(micInputs, defaultName, 1);
            }
            if (digitalIndex < 0)
                digitalIndex = 0;
        }
    }
    ui->digitalAudioDevList->setCurrentIndex(digitalIndex);
}
