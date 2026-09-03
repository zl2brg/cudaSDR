#include "Util/AudioDeviceService.h"
#include "tx_settings_dialog.h"
#include "ui_tx_settings_dialog.h"
#include "eq_curve_plot.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "AudioEngine/cusdr_audio_input.h"
#include "cusdr_settings.h"
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

    // Phase-rotator auto-cal + TX EQ (added in code so older .ui stays compatible).
    m_phaseRotatorAuto = new QCheckBox(QStringLiteral("Phrot Auto-Cal"), this);
    m_phaseRotatorReset = new QPushButton(QStringLiteral("Reset Auto"), this);
    m_phaseRotatorStatus = new QLabel(this);
    m_phaseRotatorStatus->setWordWrap(true);
    m_phaseRotatorStatus->setStyleSheet(QStringLiteral("color: gray;"));
    QHBoxLayout *phrotRow = new QHBoxLayout();
    phrotRow->addWidget(m_phaseRotatorAuto);
    phrotRow->addWidget(m_phaseRotatorReset);
    ui->verticalLayout_4->addLayout(phrotRow);
    ui->verticalLayout_4->addWidget(m_phaseRotatorStatus);
    connect(m_phaseRotatorAuto, &QCheckBox::toggled, this, &tx_settings_dialog::phaseRotatorAutoRequested);
    connect(m_phaseRotatorReset, &QPushButton::clicked, this, &tx_settings_dialog::phaseRotatorAutoResetRequested);

    QGroupBox *txEqGroup = new QGroupBox(QStringLiteral("TX Equalizer"), this);
    QVBoxLayout *txEqLayout = new QVBoxLayout(txEqGroup);
    txEqLayout->setContentsMargins(6, 4, 6, 6);
    QHBoxLayout *txEqTop = new QHBoxLayout();
    m_txEqEnable = new QCheckBox(QStringLiteral("Enable"), txEqGroup);
    m_txEqCurveDeg = new QSpinBox(txEqGroup);
    m_txEqCurveDeg->setRange(0, 3);
    m_txEqCurveDeg->setPrefix(QStringLiteral("NURBS "));
    m_txEqCurveDeg->setToolTip(QStringLiteral("0 = classic linear; 1–3 = NURBS degree"));
    txEqTop->addWidget(m_txEqEnable);
    txEqTop->addStretch();
    txEqTop->addWidget(m_txEqCurveDeg);
    txEqLayout->addLayout(txEqTop);
    QHBoxLayout *eqRow = new QHBoxLayout();
    eqRow->setSpacing(2);
    for (int i = 0; i < EqCurvePlot::kBandSliderCount; ++i) {
        QVBoxLayout *col = new QVBoxLayout();
        const QString labelText = (i == 0)
            ? QStringLiteral("Pre")
            : QString::fromLatin1(EqCurvePlot::bandLabel(i - 1));
        QLabel *lab = new QLabel(labelText, txEqGroup);
        lab->setAlignment(Qt::AlignHCenter);
        if (i == 0)
            lab->setMinimumWidth(32);
        QSlider *slider = new QSlider(Qt::Vertical, txEqGroup);
        slider->setRange(-12, 12);
        slider->setValue(0);
        slider->setFixedHeight(72);
        slider->setToolTip(i == 0
            ? QStringLiteral("Preamp gain (dB)")
            : QStringLiteral("%1 Hz gain (dB)").arg(labelText));
        connect(slider, &QSlider::valueChanged, this, [this, i](int value) {
            emit txEqBandRequested(i, value);
        });
        m_txEqSliders.append(slider);
        col->addWidget(lab);
        col->addWidget(slider, 0, Qt::AlignHCenter);
        eqRow->addLayout(col);
    }
    txEqLayout->addLayout(eqRow);
    m_txEqPlot = new EqCurvePlot(txEqGroup);
    txEqLayout->addWidget(m_txEqPlot);
    ui->verticalLayoutScroll->insertWidget(1, txEqGroup);
    connect(m_txEqEnable, &QCheckBox::toggled, this, &tx_settings_dialog::txEqEnabledRequested);
    connect(m_txEqCurveDeg, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &tx_settings_dialog::txEqCurveDegRequested);

    // Continuous Frequency Compressor (CFC) + post-EQ
    QGroupBox *cfcGroup = new QGroupBox(QStringLiteral("TX CFC"), this);
    QVBoxLayout *cfcLayout = new QVBoxLayout(cfcGroup);
    QHBoxLayout *cfcTop = new QHBoxLayout();
    m_cfcEnable = new QCheckBox(QStringLiteral("CFC"), cfcGroup);
    m_cfcPeqEnable = new QCheckBox(QStringLiteral("Post EQ"), cfcGroup);
    m_cfcCurveDeg = new QSpinBox(cfcGroup);
    m_cfcCurveDeg->setRange(0, 3);
    m_cfcCurveDeg->setPrefix(QStringLiteral("NURBS "));
    cfcTop->addWidget(m_cfcEnable);
    cfcTop->addWidget(m_cfcPeqEnable);
    cfcTop->addStretch();
    cfcTop->addWidget(m_cfcCurveDeg);
    cfcLayout->addLayout(cfcTop);

    QHBoxLayout *cfcPre = new QHBoxLayout();
    m_cfcPrecomp = new QDoubleSpinBox(cfcGroup);
    m_cfcPrecomp->setRange(-20.0, 20.0);
    m_cfcPrecomp->setDecimals(1);
    m_cfcPrecomp->setSuffix(QStringLiteral(" dB"));
    m_cfcPrecomp->setPrefix(QStringLiteral("Pre "));
    m_cfcPrePeq = new QDoubleSpinBox(cfcGroup);
    m_cfcPrePeq->setRange(-20.0, 20.0);
    m_cfcPrePeq->setDecimals(1);
    m_cfcPrePeq->setSuffix(QStringLiteral(" dB"));
    m_cfcPrePeq->setPrefix(QStringLiteral("Peq "));
    cfcPre->addWidget(m_cfcPrecomp);
    cfcPre->addWidget(m_cfcPrePeq);
    cfcLayout->addLayout(cfcPre);

    static const char *const kCfcLabels[] = {
        "50", "150", "300", "500", "750", "1k2", "1k7", "2k3", "2k8", "3k1"
    };
    QHBoxLayout *cfcLvlRow = new QHBoxLayout();
    cfcLvlRow->setSpacing(2);
    for (int i = 0; i < 10; ++i) {
        QVBoxLayout *col = new QVBoxLayout();
        QLabel *lab = new QLabel(QString::fromLatin1(kCfcLabels[i]), cfcGroup);
        lab->setAlignment(Qt::AlignHCenter);
        QSlider *slider = new QSlider(Qt::Vertical, cfcGroup);
        slider->setRange(-16, 16);
        slider->setValue(0);
        slider->setFixedHeight(64);
        slider->setToolTip(QStringLiteral("CFC level %1 Hz (dB)").arg(QString::fromLatin1(kCfcLabels[i])));
        connect(slider, &QSlider::valueChanged, this, [this, i](int value) {
            emit cfcLevelRequested(i, static_cast<double>(value));
        });
        m_cfcLevelSliders.append(slider);
        col->addWidget(lab);
        col->addWidget(slider);
        cfcLvlRow->addLayout(col);
    }
    cfcLayout->addWidget(new QLabel(QStringLiteral("Comp levels"), cfcGroup));
    cfcLayout->addLayout(cfcLvlRow);

    QHBoxLayout *cfcPostRow = new QHBoxLayout();
    cfcPostRow->setSpacing(2);
    for (int i = 0; i < 10; ++i) {
        QVBoxLayout *col = new QVBoxLayout();
        QSlider *slider = new QSlider(Qt::Vertical, cfcGroup);
        slider->setRange(-16, 16);
        slider->setValue(0);
        slider->setFixedHeight(48);
        slider->setToolTip(QStringLiteral("Post EQ %1 Hz (dB)").arg(QString::fromLatin1(kCfcLabels[i])));
        connect(slider, &QSlider::valueChanged, this, [this, i](int value) {
            emit cfcPostRequested(i, static_cast<double>(value));
        });
        m_cfcPostSliders.append(slider);
        col->addWidget(slider);
        cfcPostRow->addLayout(col);
    }
    cfcLayout->addWidget(new QLabel(QStringLiteral("Post EQ levels"), cfcGroup));
    cfcLayout->addLayout(cfcPostRow);

    m_cfcCompPlot = new EqCurvePlot(cfcGroup);
    m_cfcPeqPlot = new EqCurvePlot(cfcGroup);
    cfcLayout->addWidget(new QLabel(QStringLiteral("Comp curve"), cfcGroup));
    cfcLayout->addWidget(m_cfcCompPlot);
    cfcLayout->addWidget(new QLabel(QStringLiteral("Post EQ curve"), cfcGroup));
    cfcLayout->addWidget(m_cfcPeqPlot);
    ui->verticalLayoutScroll->insertWidget(2, cfcGroup);

    connect(m_cfcEnable, &QCheckBox::toggled, this, &tx_settings_dialog::cfcEnabledRequested);
    connect(m_cfcPeqEnable, &QCheckBox::toggled, this, &tx_settings_dialog::cfcPeqEnabledRequested);
    connect(m_cfcPrecomp, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &tx_settings_dialog::cfcPrecompRequested);
    connect(m_cfcPrePeq, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &tx_settings_dialog::cfcPrePeqRequested);
    connect(m_cfcCurveDeg, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &tx_settings_dialog::cfcCurveDegRequested);

    connect(ui->ctcss_tone, QOverload<int>::of(&QSpinBox::valueChanged), this, &tx_settings_dialog::ctcssToneHzRequested);
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

    connect(ui->tx_use_rx_filter, &QCheckBox::toggled, this, [this](bool checked) {
        ui->tx_filter_low->setEnabled(!checked);
        ui->tx_filter_high->setEnabled(!checked);
        emit txUseRxFilterRequested(checked);
    });
    connect(ui->tx_filter_low, &QSpinBox::valueChanged, this, &tx_settings_dialog::txFilterLowRequested);
    connect(ui->tx_filter_high, &QSpinBox::valueChanged, this, &tx_settings_dialog::txFilterHighRequested);
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
    if (m_phaseRotatorAuto)
        m_phaseRotatorAuto->setEnabled(enabled);
    if (m_phaseRotatorReset)
        m_phaseRotatorReset->setEnabled(enabled);
}

void tx_settings_dialog::setPhaseRotatorAuto(bool enabled)
{
    if (!m_phaseRotatorAuto)
        return;
    const QSignalBlocker blocker(m_phaseRotatorAuto);
    m_phaseRotatorAuto->setChecked(enabled);
}

void tx_settings_dialog::setPhaseRotatorStatus(const QString &status)
{
    if (m_phaseRotatorStatus)
        m_phaseRotatorStatus->setText(status);
}

void tx_settings_dialog::setTxEqEnabled(bool enabled)
{
    if (!m_txEqEnable)
        return;
    const QSignalBlocker blocker(m_txEqEnable);
    m_txEqEnable->setChecked(enabled);
}

void tx_settings_dialog::setTxEqBands(const QVector<int> &bands)
{
    for (int i = 0; i < m_txEqSliders.size(); ++i) {
        const QSignalBlocker blocker(m_txEqSliders.at(i));
        m_txEqSliders.at(i)->setValue(i < bands.size() ? bands.at(i) : 0);
    }
}

void tx_settings_dialog::setTxEqCurveDeg(int deg)
{
    if (!m_txEqCurveDeg)
        return;
    const QSignalBlocker blocker(m_txEqCurveDeg);
    m_txEqCurveDeg->setValue(deg);
}

void tx_settings_dialog::setCfcEnabled(bool enabled)
{
    if (!m_cfcEnable)
        return;
    const QSignalBlocker blocker(m_cfcEnable);
    m_cfcEnable->setChecked(enabled);
}

void tx_settings_dialog::setCfcPeqEnabled(bool enabled)
{
    if (!m_cfcPeqEnable)
        return;
    const QSignalBlocker blocker(m_cfcPeqEnable);
    m_cfcPeqEnable->setChecked(enabled);
}

void tx_settings_dialog::setCfcPrecomp(double db)
{
    if (!m_cfcPrecomp)
        return;
    const QSignalBlocker blocker(m_cfcPrecomp);
    m_cfcPrecomp->setValue(db);
}

void tx_settings_dialog::setCfcPrePeq(double db)
{
    if (!m_cfcPrePeq)
        return;
    const QSignalBlocker blocker(m_cfcPrePeq);
    m_cfcPrePeq->setValue(db);
}

void tx_settings_dialog::setCfcCurveDeg(int deg)
{
    if (!m_cfcCurveDeg)
        return;
    const QSignalBlocker blocker(m_cfcCurveDeg);
    m_cfcCurveDeg->setValue(deg);
}

void tx_settings_dialog::setCfcLevels(const QVector<double> &levels)
{
    for (int i = 0; i < m_cfcLevelSliders.size(); ++i) {
        const QSignalBlocker blocker(m_cfcLevelSliders.at(i));
        m_cfcLevelSliders.at(i)->setValue(i < levels.size() ? qRound(levels.at(i)) : 0);
    }
}

void tx_settings_dialog::setCfcPost(const QVector<double> &post)
{
    for (int i = 0; i < m_cfcPostSliders.size(); ++i) {
        const QSignalBlocker blocker(m_cfcPostSliders.at(i));
        m_cfcPostSliders.at(i)->setValue(i < post.size() ? qRound(post.at(i)) : 0);
    }
}

void tx_settings_dialog::refreshEqCurvePlots()
{
    QVector<double> X(AudioConfig::kEqDrawPoints, 0.0);
    QVector<double> Y(AudioConfig::kEqDrawPoints, 0.0);
    if (m_txEqPlot) {
        GetTXAEQDraw(TX_ID, X.data(), Y.data());
        const double preamp = m_txEqSliders.isEmpty() ? 0.0 : static_cast<double>(m_txEqSliders.at(0)->value());
        m_txEqPlot->setBandEqCurve(X, Y, preamp);
    }
    if (m_cfcCompPlot) {
        GetTXACFCOMPCompDraw(TX_ID, X.data(), Y.data());
        m_cfcCompPlot->setCurve(X, Y);
    }
    if (m_cfcPeqPlot) {
        GetTXACFCOMPPeqDraw(TX_ID, X.data(), Y.data());
        m_cfcPeqPlot->setCurve(X, Y);
    }
}

void tx_settings_dialog::setCtcssToneHz(int hz)
{
    const QSignalBlocker blocker(ui->ctcss_tone);
    ui->ctcss_tone->setValue(hz);
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

void tx_settings_dialog::setTxFilterLow(int val)
{
    const QSignalBlocker blocker(ui->tx_filter_low);
    ui->tx_filter_low->setValue(val);
}

void tx_settings_dialog::setTxFilterHigh(int val)
{
    const QSignalBlocker blocker(ui->tx_filter_high);
    ui->tx_filter_high->setValue(val);
}

void tx_settings_dialog::setTxUseRxFilter(bool enabled)
{
    const QSignalBlocker blocker(ui->tx_use_rx_filter);
    ui->tx_use_rx_filter->setChecked(enabled);
    ui->tx_filter_low->setEnabled(!enabled);
    ui->tx_filter_high->setEnabled(!enabled);
}
