#ifndef FREQUENCYENTRYDIALOG_H
#define FREQUENCYENTRYDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>

class FrequencyEntryDialog : public QDialog {
    Q_OBJECT

public:
    explicit FrequencyEntryDialog(qint64 currentFreq, QWidget *parent = nullptr);
    qint64 frequency() const;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onNumberClicked();
    void onBackspaceClicked();
    void onClearClicked();
    void onEnterClicked();
    void onUnitClicked();

private:
    QLineEdit *m_display;
    qint64 m_frequency;
    
    void updateDisplayFromValue(qint64 freq);
    qint64 valueFromDisplay() const;
};

#endif // FREQUENCYENTRYDIALOG_H
