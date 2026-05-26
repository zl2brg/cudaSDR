#include "FrequencyEntryDialog.h"
#include <QRegularExpressionValidator>
#include <QDebug>

FrequencyEntryDialog::FrequencyEntryDialog(qint64 currentFreq, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Enter Frequency"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_display = new QLineEdit(this);
    m_display->setAlignment(Qt::AlignRight);
    m_display->setFont(QFont("Monospace", 18, QFont::Bold));
    m_display->setPlaceholderText("0.000.000");
    
    // Validator for digits and dots
    QRegularExpression re("[0-9.]+");
    m_display->setValidator(new QRegularExpressionValidator(re, this));

    mainLayout->addWidget(m_display);

    QGridLayout *keypadLayout = new QGridLayout();
    const char *buttons[] = {
        "7", "8", "9",
        "4", "5", "6",
        "1", "2", "3",
        "0", ".", "Clear"
    };

    for (int i = 0; i < 12; ++i) {
        QPushButton *btn = new QPushButton(tr(buttons[i]), this);
        btn->setFixedSize(60, 40);
        if (i < 11) {
            connect(btn, &QPushButton::clicked, this, &FrequencyEntryDialog::onNumberClicked);
        } else {
            connect(btn, &QPushButton::clicked, this, &FrequencyEntryDialog::onClearClicked);
        }
        keypadLayout->addWidget(btn, i / 3, i % 3);
    }

    QVBoxLayout *unitLayout = new QVBoxLayout();
    QStringList units = {"Hz", "kHz", "MHz", "GHz"};
    for (const QString &unit : units) {
        QPushButton *btn = new QPushButton(unit, this);
        btn->setFixedSize(60, 40);
        connect(btn, &QPushButton::clicked, this, &FrequencyEntryDialog::onUnitClicked);
        unitLayout->addWidget(btn);
    }
    keypadLayout->addLayout(unitLayout, 0, 3, 4, 1);

    mainLayout->addLayout(keypadLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QPushButton *backBtn = new QPushButton(tr("Back"), this);
    QPushButton *okBtn = new QPushButton(tr("Enter"), this);
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);

    connect(backBtn, &QPushButton::clicked, this, &FrequencyEntryDialog::onBackspaceClicked);
    connect(okBtn, &QPushButton::clicked, this, &FrequencyEntryDialog::onEnterClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    bottomLayout->addWidget(backBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(cancelBtn);
    bottomLayout->addWidget(okBtn);

    mainLayout->addLayout(bottomLayout);

    updateDisplayFromValue(currentFreq);
    m_display->selectAll();
    m_display->setFocus();
}

qint64 FrequencyEntryDialog::frequency() const {
    return m_frequency;
}

void FrequencyEntryDialog::onNumberClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        m_display->insert(btn->text());
    }
}

void FrequencyEntryDialog::onBackspaceClicked() {
    m_display->backspace();
}

void FrequencyEntryDialog::onClearClicked() {
    m_display->clear();
}

void FrequencyEntryDialog::onEnterClicked() {
    m_frequency = valueFromDisplay();
    accept();
}

void FrequencyEntryDialog::onUnitClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    double val = m_display->text().toDouble();
    QString unit = btn->text();

    if (unit == "kHz") val *= 1000.0;
    else if (unit == "MHz") val *= 1000000.0;
    else if (unit == "GHz") val *= 1000000000.0;

    m_frequency = static_cast<qint64>(val);
    accept();
}

void FrequencyEntryDialog::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        onEnterClicked();
    } else {
        QDialog::keyPressEvent(event);
    }
}

void FrequencyEntryDialog::updateDisplayFromValue(qint64 freq) {
    m_display->setText(QString::number(freq / 1000000.0, 'f', 6));
}

qint64 FrequencyEntryDialog::valueFromDisplay() const {
    // Basic heuristic: if there are dots, we might be entering in MHz format
    // But let's assume the user typed exactly what they wanted in Hz if no units clicked.
    // However, cuSDR usually treats the display as MHz.
    bool ok;
    double val = m_display->text().toDouble(&ok);
    if (!ok) return 0;
    
    // If value is small (e.g. < 6000), assume it's MHz
    if (val < 6000.0) return static_cast<qint64>(val * 1000000.0);
    
    return static_cast<qint64>(val);
}
