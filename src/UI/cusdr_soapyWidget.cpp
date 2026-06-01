#include "cusdr_soapyWidget.h"

#ifdef HAVE_SOAPYSDR

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpacerItem>

SoapyWidget::SoapyWidget(QWidget *parent)
    : QWidget(parent)
    , set(Settings::instance())
{
    buildUi();
    populateFromSettings();

    connect(set, &Settings::masterSwitchChanged,
            this, [](bool){ Settings::instance()->saveSettings(); });

    connect(set, &Settings::soapyAntennaListChanged,
            this, &SoapyWidget::onSoapyAntennaListChanged);
    connect(set, &Settings::soapyHardwareKeyChanged,
            this, &SoapyWidget::onSoapyHardwareKeyChanged);
    connect(set, &Settings::sampleRateChanged,
            this, &SoapyWidget::onSampleRateChanged);

    connect(m_antennaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SoapyWidget::onAntennaComboChanged);
    connect(m_dspRateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SoapyWidget::onDspRateChanged);
    connect(m_autoGainCheck, &QCheckBox::toggled,
            this, &SoapyWidget::onAutoCalToggled);
    connect(m_lnaSlider,   &QSlider::valueChanged,
            this, &SoapyWidget::onLnaSliderChanged);
    connect(m_lnaSpinBox,  QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SoapyWidget::onLnaSpinBoxChanged);
    connect(m_tiaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SoapyWidget::onTiaComboChanged);
    connect(m_pgaSlider,   &QSlider::valueChanged,
            this, &SoapyWidget::onPgaSliderChanged);
    connect(m_pgaSpinBox,  QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SoapyWidget::onPgaSpinBoxChanged);
    connect(m_overallGainSlider, &QSlider::valueChanged,
            this, &SoapyWidget::onOverallGainSliderChanged);
    connect(m_overallGainSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &SoapyWidget::onOverallGainSpinBoxChanged);
    connect(m_fullDuplexCheck, &QCheckBox::toggled,
            this, &SoapyWidget::onFullDuplexToggled);
    connect(m_iqBalanceCheck, &QCheckBox::toggled,
            this, &SoapyWidget::onIQBalanceToggled);
}

void SoapyWidget::buildUi()
{
    // --- Antenna group ---
    m_antennaGroup = new QGroupBox(tr("Antenna"), this);
    m_antennaCombo = new QComboBox(this);
    QHBoxLayout *antennaLayout = new QHBoxLayout(m_antennaGroup);
    antennaLayout->addWidget(new QLabel(tr("RX Antenna:")));
    antennaLayout->addWidget(m_antennaCombo, 1);

    // --- LimeSDR element gain group ---
    m_limeGainGroup = new QGroupBox(tr("Gain \xe2\x80\x94 LimeSDR (LNA / TIA / PGA)"), this);
    m_autoGainCheck = new QCheckBox(tr("Auto calibration"), this);
    m_lnaSlider  = new QSlider(Qt::Horizontal, this);
    m_lnaSlider->setRange(0, 30);
    m_lnaSpinBox = new QSpinBox(this);
    m_lnaSpinBox->setRange(0, 30);
    m_lnaSpinBox->setFixedWidth(45);
    m_tiaCombo = new QComboBox(this);
    m_tiaCombo->addItems({"0", "9", "12"});
    m_pgaSlider  = new QSlider(Qt::Horizontal, this);
    m_pgaSlider->setRange(0, 19);
    m_pgaSpinBox = new QSpinBox(this);
    m_pgaSpinBox->setRange(0, 19);
    m_pgaSpinBox->setFixedWidth(45);

    QFormLayout *limeLayout = new QFormLayout(m_limeGainGroup);
    limeLayout->addRow(m_autoGainCheck);
    auto addSliderRow = [&](QFormLayout *form, const QString &label,
                            QSlider *slider, QSpinBox *spin) {
        QWidget *row = new QWidget;
        QHBoxLayout *hl = new QHBoxLayout(row);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->addWidget(slider, 1);
        hl->addWidget(spin);
        form->addRow(label, row);
    };
    addSliderRow(limeLayout, tr("LNA (0\xe2\x80\x9330 dB):"), m_lnaSlider, m_lnaSpinBox);
    limeLayout->addRow(tr("TIA (0/9/12 dB):"), m_tiaCombo);
    addSliderRow(limeLayout, tr("PGA (0\xe2\x80\x9319 dB):"), m_pgaSlider, m_pgaSpinBox);

    // --- Overall gain group (for non-LimeSDR devices) ---
    m_overallGainGroup = new QGroupBox(tr("Gain \xe2\x80\x94 Overall"), this);
    m_overallGainSlider  = new QSlider(Qt::Horizontal, this);
    m_overallGainSlider->setRange(0, 70);
    m_overallGainSpinBox = new QSpinBox(this);
    m_overallGainSpinBox->setRange(0, 70);
    QHBoxLayout *overallLayout = new QHBoxLayout(m_overallGainGroup);
    overallLayout->addWidget(new QLabel(tr("Gain (0\xe2\x80\x9370 dB):")));
    overallLayout->addWidget(m_overallGainSlider, 1);
    overallLayout->addWidget(m_overallGainSpinBox);

    // --- DSP Rate group ---
    m_dspRateGroup = new QGroupBox(tr("DSP Sample Rate (Internal IQ)"), this);
    m_dspRateCombo = new QComboBox(this);
    m_dspRateCombo->addItems({"48 kHz", "96 kHz", "192 kHz", "384 kHz", "768 kHz", "1.536 MHz"});
    m_hwRateLabel = new QLabel(this);
    m_decimLabel = new QLabel(this);
    m_hwRateLabel->setStyleSheet("color: #888;");
    m_decimLabel->setStyleSheet("color: #888;");

    QFormLayout *rateLayout = new QFormLayout(m_dspRateGroup);
    rateLayout->addRow(tr("DSP Rate:"), m_dspRateCombo);
    rateLayout->addRow(tr("Hardware Rate:"), m_hwRateLabel);
    rateLayout->addRow(tr("Bridge:"), m_decimLabel);

    // --- TX Mode / correction group ---
    m_txModeGroup     = new QGroupBox(tr("TX Mode && Correction"), this);
    m_fullDuplexCheck = new QCheckBox(tr("Full duplex (simultaneous RX+TX)"), this);
    m_iqBalanceCheck  = new QCheckBox(tr("IQ balance correction"), this);
    QVBoxLayout *txModeLayout = new QVBoxLayout(m_txModeGroup);
    txModeLayout->addWidget(m_fullDuplexCheck);
    txModeLayout->addWidget(m_iqBalanceCheck);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_antennaGroup);
    mainLayout->addWidget(m_dspRateGroup);
    mainLayout->addWidget(m_limeGainGroup);
    mainLayout->addWidget(m_overallGainGroup);
    mainLayout->addWidget(m_txModeGroup);
    mainLayout->addStretch();
    setLayout(mainLayout);
}

void SoapyWidget::populateFromSettings()
{
    m_lnaSlider->blockSignals(true);
    m_lnaSpinBox->blockSignals(true);
    m_tiaCombo->blockSignals(true);
    m_pgaSlider->blockSignals(true);
    m_pgaSpinBox->blockSignals(true);
    m_overallGainSlider->blockSignals(true);
    m_overallGainSpinBox->blockSignals(true);
    m_autoGainCheck->blockSignals(true);
    m_dspRateCombo->blockSignals(true);

    m_lnaSlider->setValue(set->getSoapyLnaGain());
    m_lnaSpinBox->setValue(set->getSoapyLnaGain());
    int tia = set->getSoapyTiaGain();
    m_tiaCombo->setCurrentIndex(tia >= 12 ? 2 : tia >= 9 ? 1 : 0);
    m_pgaSlider->setValue(set->getSoapyPgaGain());
    m_pgaSpinBox->setValue(set->getSoapyPgaGain());
    m_overallGainSlider->setValue(set->getSoapyOverallGain());
    m_overallGainSpinBox->setValue(set->getSoapyOverallGain());
    m_autoGainCheck->setChecked(set->getSoapyAutoCalibrate());
    m_fullDuplexCheck->setChecked(set->getTxFullDuplex());
    m_iqBalanceCheck->setChecked(set->getSoapyIQBalance());

    onSampleRateChanged(set->getSampleRate());

    m_lnaSlider->blockSignals(false);
    m_lnaSpinBox->blockSignals(false);
    m_tiaCombo->blockSignals(false);
    m_pgaSlider->blockSignals(false);
    m_pgaSpinBox->blockSignals(false);
    m_overallGainSlider->blockSignals(false);
    m_overallGainSpinBox->blockSignals(false);
    m_autoGainCheck->blockSignals(false);
    m_dspRateCombo->blockSignals(false);

    QStringList antennas = set->getSoapyAntennaList();
    if (!antennas.isEmpty())
        onSoapyAntennaListChanged(antennas);

    updateGainGroupVisibility();
}

void SoapyWidget::onSampleRateChanged(int rate)
{
    m_dspRateCombo->blockSignals(true);
    switch (rate) {
        case 48000:   m_dspRateCombo->setCurrentIndex(0); break;
        case 96000:   m_dspRateCombo->setCurrentIndex(1); break;
        case 192000:  m_dspRateCombo->setCurrentIndex(2); break;
        case 384000:  m_dspRateCombo->setCurrentIndex(3); break;
        case 768000:  m_dspRateCombo->setCurrentIndex(4); break;
        case 1536000: m_dspRateCombo->setCurrentIndex(5); break;
    }
    m_dspRateCombo->blockSignals(false);

    // Update hardware labels if data source is active
    int hwRate = set->getSoapyRfSampleRate();
    if (hwRate > 0) {
        m_hwRateLabel->setText(QString("%1 MSPS").arg(hwRate / 1e6, 0, 'f', 3));
        double ratio = (double)hwRate / (double)rate;
        if (std::abs(ratio - 1.0) > 0.001) {
            m_decimLabel->setText(QString("Liquid Fractional (%1:1)").arg(ratio, 0, 'f', 2));
        } else {
            m_decimLabel->setText(tr("Native (1:1 pass-through)"));
        }
    } else {
        m_hwRateLabel->setText(tr("Disconnected"));
        m_decimLabel->setText(tr("n/a"));
    }
}

void SoapyWidget::onDspRateChanged(int index)
{
    int rate = 48000;
    switch (index) {
        case 0: rate = 48000; break;
        case 1: rate = 96000; break;
        case 2: rate = 192000; break;
        case 3: rate = 384000; break;
        case 4: rate = 768000; break;
        case 5: rate = 1536000; break;
    }
    set->setSampleRate(rate);
}

void SoapyWidget::updateGainGroupVisibility()
{
    bool isLime = set->getSoapyHardwareKey().contains("LimeSDR", Qt::CaseInsensitive);
    m_limeGainGroup->setVisible(isLime);
    m_overallGainGroup->setVisible(!isLime);
}

void SoapyWidget::onSoapyAntennaListChanged(QStringList list)
{
    m_antennaCombo->blockSignals(true);
    m_antennaCombo->clear();
    m_antennaCombo->addItems(list);
    int idx = list.indexOf(set->getSoapyRxAntenna());
    if (idx >= 0) m_antennaCombo->setCurrentIndex(idx);
    m_antennaCombo->blockSignals(false);
}

void SoapyWidget::onSoapyHardwareKeyChanged(QString /*key*/)
{
    updateGainGroupVisibility();
}

void SoapyWidget::onAntennaComboChanged(int index)
{
    set->setSoapyRxAntenna(m_antennaCombo->itemText(index));
}

void SoapyWidget::onAutoCalToggled(bool enabled)
{
    set->setSoapyAutoCalibrate(enabled);
}

void SoapyWidget::onLnaSliderChanged(int value)
{
    m_lnaSpinBox->blockSignals(true);
    m_lnaSpinBox->setValue(value);
    m_lnaSpinBox->blockSignals(false);
    set->setSoapyLnaGain(value);
}

void SoapyWidget::onLnaSpinBoxChanged(int value)
{
    m_lnaSlider->blockSignals(true);
    m_lnaSlider->setValue(value);
    m_lnaSlider->blockSignals(false);
    set->setSoapyLnaGain(value);
}

void SoapyWidget::onTiaComboChanged(int index)
{
    int gain = (index == 2) ? 12 : (index == 1) ? 9 : 0;
    set->setSoapyTiaGain(gain);
}

void SoapyWidget::onPgaSliderChanged(int value)
{
    m_pgaSpinBox->blockSignals(true);
    m_pgaSpinBox->setValue(value);
    m_pgaSpinBox->blockSignals(false);
    set->setSoapyPgaGain(value);
}

void SoapyWidget::onPgaSpinBoxChanged(int value)
{
    m_pgaSlider->blockSignals(true);
    m_pgaSlider->setValue(value);
    m_pgaSlider->blockSignals(false);
    set->setSoapyPgaGain(value);
}

void SoapyWidget::onOverallGainSliderChanged(int value)
{
    m_overallGainSpinBox->blockSignals(true);
    m_overallGainSpinBox->setValue(value);
    m_overallGainSpinBox->blockSignals(false);
    set->setSoapyOverallGain(value);
}

void SoapyWidget::onOverallGainSpinBoxChanged(int value)
{
    m_overallGainSlider->blockSignals(true);
    m_overallGainSlider->setValue(value);
    m_overallGainSlider->blockSignals(false);
    set->setSoapyOverallGain(value);
}

#endif // HAVE_SOAPYSDR
