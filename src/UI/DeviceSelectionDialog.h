#ifndef DEVICESELECTIONDIALOG_H
#define DEVICESELECTIONDIALOG_H

#include <QDialog>
#include <QList>
#include <QVariant>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "cusdr_settings.h"
#include "Util/cusdr_buttons.h"

class DeviceSelectionDialog : public QDialog {
    Q_OBJECT

public:
    explicit DeviceSelectionDialog(const QList<QVariant> &devices, QWidget *parent = nullptr);
    QVariant selectedDevice() const;

private slots:
    void okBtnClicked();

private:
    QComboBox *m_deviceComboBox;
    QList<QVariant> m_deviceItems;
    QVariant m_selectedDevice;
};

#endif // DEVICESELECTIONDIALOG_H
