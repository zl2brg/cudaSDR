#include "Util/AudioDeviceService.h"
#include "tx_settings_dialog.h"
#include "ui_tx_settings_dialog.h"
#include "eq_curve_plot.h"
#include "QtWDSP/qtwdsp_dspEngine.h"
#include "AudioEngine/cusdr_audio_input.h"
#include "cusdr_settings.h"
#include "cusdr_hamDatabase.h"
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

    QPushButton *flatBtn = new QPushButton(QStringLiteral("Flat"), txEqGroup);
    flatBtn->setToolTip(QStringLiteral("Reset all equalizer bands to 0 dB"));
    flatBtn->setFixedHeight(22);
    connect(flatBtn, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < EqCurvePlot::kBandSliderCount; ++i) {
            emit txEqBandRequested(i, 0);
        }
    });

    txEqTop->addWidget(m_txEqEnable);
    txEqTop->addStretch();
    txEqTop->addWidget(flatBtn);
    txEqTop->addWidget(m_txEqCurveDeg);
    txEqLayout->addLayout(txEqTop);

    m_txEqPlot = new EqCurvePlot(txEqGroup);
    m_txEqPlot->setInteractive(true);
    m_txEqPlot->setFixedHeight(125);
    connect(m_txEqPlot, &EqCurvePlot::bandGainChanged, this, &tx_settings_dialog::txEqBandRequested);
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

    QHBoxLayout *compHeader = new QHBoxLayout();
    compHeader->addWidget(new QLabel(QStringLiteral("Compression Levels"), cfcGroup));
    compHeader->addStretch();
    QPushButton *compFlatBtn = new QPushButton(QStringLiteral("Flat"), cfcGroup);
    compFlatBtn->setToolTip(QStringLiteral("Reset all compression levels to 0 dB"));
    compFlatBtn->setFixedHeight(20);
    connect(compFlatBtn, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < EqCurvePlot::kCfcBandCount; ++i) {
            emit cfcLevelRequested(i, 0.0);
        }
    });
    compHeader->addWidget(compFlatBtn);
    cfcLayout->addLayout(compHeader);

    m_cfcCompPlot = new EqCurvePlot(cfcGroup);
    m_cfcCompPlot->setPlotMode(EqCurvePlot::PlotMode::Cfc);
    m_cfcCompPlot->setAccentColor(QColor(255, 160, 40));
    m_cfcCompPlot->setInteractive(true);
    m_cfcCompPlot->setFixedHeight(105);
    connect(m_cfcCompPlot, &EqCurvePlot::bandGainChanged, this, [this](int band, int value) {
        emit cfcLevelRequested(band, static_cast<double>(value));
    });
    cfcLayout->addWidget(m_cfcCompPlot);

    QHBoxLayout *peqHeader = new QHBoxLayout();
    peqHeader->addWidget(new QLabel(QStringLiteral("Post EQ Levels"), cfcGroup));
    peqHeader->addStretch();
    QPushButton *peqFlatBtn = new QPushButton(QStringLiteral("Flat"), cfcGroup);
    peqFlatBtn->setToolTip(QStringLiteral("Reset all post-EQ levels to 0 dB"));
    peqFlatBtn->setFixedHeight(20);
    connect(peqFlatBtn, &QPushButton::clicked, this, [this]() {
        for (int i = 0; i < EqCurvePlot::kCfcBandCount; ++i) {
            emit cfcPostRequested(i, 0.0);
        }
    });
    peqHeader->addWidget(peqFlatBtn);
    cfcLayout->addLayout(peqHeader);

    m_cfcPeqPlot = new EqCurvePlot(cfcGroup);
    m_cfcPeqPlot->setPlotMode(EqCurvePlot::PlotMode::Cfc);
    m_cfcPeqPlot->setAccentColor(QColor(50, 215, 160));
    m_cfcPeqPlot->setInteractive(true);
    m_cfcPeqPlot->setFixedHeight(105);
    connect(m_cfcPeqPlot, &EqCurvePlot::bandGainChanged, this, [this](int band, int value) {
        emit cfcPostRequested(band, static_cast<double>(value));
    });
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
    if (m_txEqPlot) {
        m_txEqPlot->setBandGains(bands);
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
    if (m_cfcCompPlot) {
        QVector<int> intLevels;
        intLevels.reserve(levels.size());
        for (double v : levels)
            intLevels.append(qRound(v));
        m_cfcCompPlot->setBandGains(intLevels);
    }
}

void tx_settings_dialog::setCfcPost(const QVector<double> &post)
{
    if (m_cfcPeqPlot) {
        QVector<int> intPost;
        intPost.reserve(post.size());
        for (double v : post)
            intPost.append(qRound(v));
        m_cfcPeqPlot->setBandGains(intPost);
    }
}

void tx_settings_dialog::refreshEqCurvePlots()
{
    QVector<double> X(AudioConfig::kEqDrawPoints, 0.0);
    QVector<double> Y(AudioConfig::kEqDrawPoints, 0.0);
    if (m_txEqPlot) {
        GetTXAEQDraw(TX_ID, X.data(), Y.data());
        const QVector<int> bands = m_txEqPlot->bandGains();
        const double preamp = bands.isEmpty() ? 0.0 : static_cast<double>(bands.at(0));
        m_txEqPlot->setBandEqCurve(X, Y, preamp);
        updateTxEqPassband();
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

void tx_settings_dialog::updateTxEqPassband()
{
    if (!m_txEqPlot)
        return;
    const DSPMode mode = Settings::instance()->getDSPMode(0);
    const auto tf = resolvedTxPassband(Settings::instance()->getDefaultFilterList(), mode, 0, 0);
    double lo = 0.0, hi = 3000.0;
    if (isLowerSidebandMode(mode)) {
        lo = std::abs(tf.filterHi);
        hi = std::abs(tf.filterLo);
    } else if (isUpperSidebandMode(mode)) {
        lo = std::max(0.0, tf.filterLo);
        hi = std::max(lo, tf.filterHi);
    } else {
        lo = 0.0;
        hi = std::max(100.0, std::max(std::abs(tf.filterLo), std::abs(tf.filterHi)));
    }
    m_txEqPlot->setAudioPassband(lo, hi);
}

void tx_settings_dialog::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateTxEqPassband();
    refreshEqCurvePlots();
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
