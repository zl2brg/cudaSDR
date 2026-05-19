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

    connect(set, &Settings::soapyAntennaListChanged,
            this, &SoapyWidget::onSoapyAntennaListChanged);
    connect(set, &Settings::soapyHardwareKeyChanged,
            this, &SoapyWidget::onSoapyHardwareKeyChanged);
    connect(m_antennaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SoapyWidget::onAntennaComboChanged);
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

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_antennaGroup);
    mainLayout->addWidget(m_limeGainGroup);
    mainLayout->addWidget(m_overallGainGroup);
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

    m_lnaSlider->setValue(set->getSoapyLnaGain());
    m_lnaSpinBox->setValue(set->getSoapyLnaGain());
    int tia = set->getSoapyTiaGain();
    m_tiaCombo->setCurrentIndex(tia >= 12 ? 2 : tia >= 9 ? 1 : 0);
    m_pgaSlider->setValue(set->getSoapyPgaGain());
    m_pgaSpinBox->setValue(set->getSoapyPgaGain());
    m_overallGainSlider->setValue(set->getSoapyOverallGain());
    m_overallGainSpinBox->setValue(set->getSoapyOverallGain());
    m_autoGainCheck->setChecked(set->getSoapyAutoCalibrate());

    m_lnaSlider->blockSignals(false);
    m_lnaSpinBox->blockSignals(false);
    m_tiaCombo->blockSignals(false);
    m_pgaSlider->blockSignals(false);
    m_pgaSpinBox->blockSignals(false);
    m_overallGainSlider->blockSignals(false);
    m_overallGainSpinBox->blockSignals(false);
    m_autoGainCheck->blockSignals(false);

    bool agc = set->getSoapyAutoCalibrate();
    m_lnaSlider->setEnabled(!agc);
    m_lnaSpinBox->setEnabled(!agc);
    m_tiaCombo->setEnabled(!agc);
    m_pgaSlider->setEnabled(!agc);
    m_pgaSpinBox->setEnabled(!agc);

    QStringList antennas = set->getSoapyAntennaList();
    if (!antennas.isEmpty())
        onSoapyAntennaListChanged(antennas);

    updateGainGroupVisibility();
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
    m_lnaSlider->setEnabled(!enabled);
    m_lnaSpinBox->setEnabled(!enabled);
    m_tiaCombo->setEnabled(!enabled);
    m_pgaSlider->setEnabled(!enabled);
    m_pgaSpinBox->setEnabled(!enabled);
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
    static const int vals[] = {0, 9, 12};
    set->setSoapyTiaGain((index >= 0 && index < 3) ? vals[index] : 0);
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
