#include "DeviceSelectionDialog.h"

DeviceSelectionDialog::DeviceSelectionDialog(const QList<QVariant> &devices, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Select SDR Device"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumWidth(350);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel(tr("Multiple devices discovered. Please select one:"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    m_deviceComboBox = new QComboBox(this);
    for (const QVariant &dev : devices) {
        if (dev.canConvert<TNetworkDevicecard>()) {
            TNetworkDevicecard card = dev.value<TNetworkDevicecard>();
            QString label = QString("%1 (%2)").arg(card.boardName).arg(card.ip_address.toString());
            m_deviceComboBox->addItem(label);
            m_deviceItems.append(dev);
        }
#ifdef HAVE_SOAPYSDR
        else if (dev.canConvert<TSoapyDevice>()) {
            TSoapyDevice soapy = dev.value<TSoapyDevice>();
            QString label = QString("[Soapy] %1").arg(soapy.label);
            m_deviceComboBox->addItem(label);
            m_deviceItems.append(dev);
        }
#endif
    }
    layout->addWidget(m_deviceComboBox);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    AeroButton *okBtn = new AeroButton("Select", this);
    okBtn->setFixedSize(80, 25);
    connect(okBtn, &AeroButton::clicked, this, &DeviceSelectionDialog::okBtnClicked);
    btnLayout->addWidget(okBtn);

    AeroButton *cancelBtn = new AeroButton("Cancel", this);
    cancelBtn->setFixedSize(80, 25);
    connect(cancelBtn, &AeroButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    layout->addLayout(btnLayout);
}

QVariant DeviceSelectionDialog::selectedDevice() const {
    return m_selectedDevice;
}

void DeviceSelectionDialog::okBtnClicked() {
    const int idx = m_deviceComboBox->currentIndex();
    if (idx >= 0 && idx < m_deviceItems.size())
        m_selectedDevice = m_deviceItems.at(idx);
    else
        m_selectedDevice.clear();
    accept();
}
