#include "cusdr_radiosettingswidget.h"
#include "ui_cusdr_radiosettingswidget.h"

cusdr_radioSettingsWidget::cusdr_radioSettingsWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::cusdr_radioSettingsWidget)
{
    ui->setupUi(this);

#ifdef HAVE_SOAPYSDR
    connect(ui->fullDuplexCheck, &QCheckBox::toggled,
            this, &cusdr_radioSettingsWidget::onFullDuplexToggled);
    connect(ui->iqBalanceCheck, &QCheckBox::toggled,
            this, &cusdr_radioSettingsWidget::onIQBalanceToggled);
    connect(ui->autoCalCheck, &QCheckBox::toggled,
            this, &cusdr_radioSettingsWidget::onAutoCalToggled);
    connect(ui->antennaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &cusdr_radioSettingsWidget::onAntennaComboChanged);
    connect(ui->txAntennaCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &cusdr_radioSettingsWidget::onTxAntennaComboChanged);
    connect(ui->lnaSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onLnaSliderChanged);
    connect(ui->lnaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onLnaSpinBoxChanged);
    connect(ui->tiaSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onTiaSliderChanged);
    connect(ui->pgaSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onPgaSliderChanged);
    connect(ui->pgaSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onPgaSpinBoxChanged);
    connect(ui->overallGainSlider, &QSlider::valueChanged,
            this, &cusdr_radioSettingsWidget::onOverallGainSliderChanged);
    connect(ui->overallGainSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &cusdr_radioSettingsWidget::onOverallGainSpinBoxChanged);
#endif
}

cusdr_radioSettingsWidget::~cusdr_radioSettingsWidget()
{
    delete ui;
}

#ifdef HAVE_SOAPYSDR
QWidget *cusdr_radioSettingsWidget::detachRadioConfigPage()
{
    const int idx = ui->tabWidget->indexOf(ui->radio);
    if (idx < 0)
        return nullptr;

    QWidget *page = ui->tabWidget->widget(idx);
    ui->tabWidget->removeTab(idx);
    page->setParent(nullptr);
    return page;
}

void cusdr_radioSettingsWidget::setTxFullDuplex(bool enabled)
{
    ui->fullDuplexCheck->blockSignals(true);
    ui->fullDuplexCheck->setChecked(enabled);
    ui->fullDuplexCheck->blockSignals(false);
}

void cusdr_radioSettingsWidget::setSoapyIQBalance(bool enabled)
{
    ui->iqBalanceCheck->blockSignals(true);
    ui->iqBalanceCheck->setChecked(enabled);
    ui->iqBalanceCheck->blockSignals(false);
}

void cusdr_radioSettingsWidget::setSoapyAutoCalibrate(bool enabled)
{
    ui->autoCalCheck->blockSignals(true);
    ui->autoCalCheck->setChecked(enabled);
    ui->autoCalCheck->blockSignals(false);
}

void cusdr_radioSettingsWidget::setSoapyLnaGain(int value)
{
    ui->lnaSlider->blockSignals(true);
    ui->lnaSlider->setValue(value);
    ui->lnaSlider->blockSignals(false);

    ui->lnaSpinBox->blockSignals(true);
    ui->lnaSpinBox->setValue(value);
    ui->lnaSpinBox->blockSignals(false);
}

void cusdr_radioSettingsWidget::setSoapyTiaGain(int value)
{
    ui->tiaSlider->blockSignals(true);
    const int tiaIndex = value >= 12 ? 2 : value >= 9 ? 1 : 0;
    ui->tiaSlider->setValue(tiaIndex);
    ui->tiaSlider->blockSignals(false);
    ui->tiaValueLabel->setText(QStringLiteral("%1 dB").arg(value >= 12 ? 12 : value >= 9 ? 9 : 0));
}

void cusdr_radioSettingsWidget::setSoapyPgaGain(int value)
{
    ui->pgaSlider->blockSignals(true);
    ui->pgaSlider->setValue(value);
    ui->pgaSlider->blockSignals(false);

    ui->pgaSpinBox->blockSignals(true);
    ui->pgaSpinBox->setValue(value);
    ui->pgaSpinBox->blockSignals(false);
}

void cusdr_radioSettingsWidget::setSoapyOverallGain(int value)
{
    ui->overallGainSlider->blockSignals(true);
    ui->overallGainSlider->setValue(value);
    ui->overallGainSlider->blockSignals(false);

    ui->overallGainSpinBox->blockSignals(true);
    ui->overallGainSpinBox->setValue(value);
    ui->overallGainSpinBox->blockSignals(false);
}

void cusdr_radioSettingsWidget::setAntennaList(const QStringList& list, const QString& active)
{
    ui->antennaCombo->blockSignals(true);
    ui->antennaCombo->clear();
    ui->antennaCombo->addItems(list);
    int idx = list.indexOf(active);
    if (idx >= 0) ui->antennaCombo->setCurrentIndex(idx);
    ui->antennaCombo->blockSignals(false);
}

void cusdr_radioSettingsWidget::setTxAntennaList(const QStringList& list, const QString& active)
{
    ui->txAntennaCombo->blockSignals(true);
    ui->txAntennaCombo->clear();
    ui->txAntennaCombo->addItems(list);
    int idx = list.indexOf(active);
    if (idx < 0)
        idx = list.indexOf(active);
    if (idx < 0 && !list.isEmpty())
        idx = 0;
    if (idx >= 0)
        ui->txAntennaCombo->setCurrentIndex(idx);
    ui->txAntennaCombo->blockSignals(false);
}

void cusdr_radioSettingsWidget::updateGainGroupVisibility(const QString& hardwareKey)
{
    bool isLime = hardwareKey.contains("LimeSDR", Qt::CaseInsensitive);
    ui->calibrationGroup->setVisible(isLime);
    ui->limeGainGroup->setVisible(isLime);
    ui->overallGainGroup->setVisible(!isLime);
}

void cusdr_radioSettingsWidget::onFullDuplexToggled(bool enabled)
{
    emit txFullDuplexRequested(enabled);
}

void cusdr_radioSettingsWidget::onIQBalanceToggled(bool enabled)
{
    emit soapyIQBalanceRequested(enabled);
}

void cusdr_radioSettingsWidget::onAutoCalToggled(bool enabled)
{
    emit soapyAutoCalibrateRequested(enabled);
}

void cusdr_radioSettingsWidget::onAntennaComboChanged(int index)
{
    emit soapyRxAntennaRequested(ui->antennaCombo->itemText(index));
}

void cusdr_radioSettingsWidget::onTxAntennaComboChanged(int index)
{
    emit soapyTxAntennaRequested(ui->txAntennaCombo->itemText(index));
}

void cusdr_radioSettingsWidget::onLnaSliderChanged(int value)
{
    ui->lnaSpinBox->blockSignals(true);
    ui->lnaSpinBox->setValue(value);
    ui->lnaSpinBox->blockSignals(false);
    emit soapyLnaGainRequested(value);
}

void cusdr_radioSettingsWidget::onLnaSpinBoxChanged(int value)
{
    ui->lnaSlider->blockSignals(true);
    ui->lnaSlider->setValue(value);
    ui->lnaSlider->blockSignals(false);
    emit soapyLnaGainRequested(value);
}

void cusdr_radioSettingsWidget::onTiaSliderChanged(int index)
{
    static const int tiaValues[] = {0, 9, 12};
    const int gain = (index >= 0 && index < 3) ? tiaValues[index] : 0;
    ui->tiaValueLabel->setText(QStringLiteral("%1 dB").arg(gain));
    emit soapyTiaGainRequested(gain);
}

void cusdr_radioSettingsWidget::onPgaSliderChanged(int value)
{
    ui->pgaSpinBox->blockSignals(true);
    ui->pgaSpinBox->setValue(value);
    ui->pgaSpinBox->blockSignals(false);
    emit soapyPgaGainRequested(value);
}

void cusdr_radioSettingsWidget::onPgaSpinBoxChanged(int value)
{
    ui->pgaSlider->blockSignals(true);
    ui->pgaSlider->setValue(value);
    ui->pgaSlider->blockSignals(false);
    emit soapyPgaGainRequested(value);
}

void cusdr_radioSettingsWidget::onOverallGainSliderChanged(int value)
{
    ui->overallGainSpinBox->blockSignals(true);
    ui->overallGainSpinBox->setValue(value);
    ui->overallGainSpinBox->blockSignals(false);
    emit soapyOverallGainRequested(value);
}

void cusdr_radioSettingsWidget::onOverallGainSpinBoxChanged(int value)
{
    ui->overallGainSlider->blockSignals(true);
    ui->overallGainSlider->setValue(value);
    ui->overallGainSlider->blockSignals(false);
    emit soapyOverallGainRequested(value);
}
#endif
