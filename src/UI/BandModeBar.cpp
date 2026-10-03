#include "BandModeBar.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Models/SliceModel.h"
#include "Util/cusdr_buttons.h"
#include "cusdr_fonts.h"
#include "cusdr_settings.h"

namespace {
constexpr int kBtnWidth = 48;
constexpr int kBtnHeight = 21;

const char *kBandLabels[] = {
    "2200m", "630 m", "160 m", "80 m", "60 m", "40 m", "30 m",
    "20 m", "17 m", "15 m", "12 m", "10 m", "6 m", "2 m",
    "125 cm", "70 cm", "33 cm", "23 cm", "13 cm", "10 cm", "5 cm",
    "Gen"
};
static_assert(int(sizeof(kBandLabels) / sizeof(kBandLabels[0])) == int(gen) + 1,
              "band buttons must match HamBand");

const char *kModeLabels[] = {
    "LSB", "USB", "DSB", "CWL", "CWU", "FMN",
    "AM", "DIGU", "SPEC", "DIGL", "SAM", "FreeDV"
};
static_assert(int(sizeof(kModeLabels) / sizeof(kModeLabels[0])) == int(FDV) + 1,
              "mode buttons must match DSPMode");

const char *kGroupStyle =
    "QGroupBox {"
    "  border: 1px solid rgb(140, 140, 140);"
    "  border-radius: 3px;"
    "  margin-top: 8px;"
    "  padding: 2px 3px 1px 3px;"
    "  color: rgb(210, 210, 210);"
    "  background-color: rgb(18, 18, 18);"
    "}"
    "QGroupBox::title {"
    "  subcontrol-origin: margin;"
    "  subcontrol-position: top left;"
    "  left: 8px;"
    "  padding: 0 4px;"
    "}";
}

BandModeBar::BandModeBar(Settings *settings, QWidget *parent)
    : QWidget(parent)
    , m_settings(settings)
    , m_rx(settings ? settings->getCurrentReceiver() : 0)
{
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    setObjectName(QStringLiteral("BandModeBar"));
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0, 0, 0));
    setPalette(pal);

    CFonts fonts(this);
    const QFont btnFont = fonts.getFonts().normalFont;

    auto *bandBox = new QGroupBox(tr("Band"), this);
    bandBox->setStyleSheet(QString::fromLatin1(kGroupStyle));
    auto *bandLayout = new QVBoxLayout(bandBox);
    bandLayout->setContentsMargins(3, 4, 3, 2);
    bandLayout->setSpacing(2);

    auto *modeBox = new QGroupBox(tr("Mode"), this);
    modeBox->setStyleSheet(QString::fromLatin1(kGroupStyle));
    auto *modeLayout = new QVBoxLayout(modeBox);
    modeLayout->setContentsMargins(3, 4, 3, 2);
    modeLayout->setSpacing(2);

    auto addRow = [this, btnFont](QVBoxLayout *box, int first, int last, QList<AeroButton *> *list,
                                  void (BandModeBar::*slot)()) {
        auto *row = new QHBoxLayout();
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(2);
        for (int i = first; i <= last; ++i) {
            AeroButton *btn = makeButton(list == &m_bandButtons ? kBandLabels[i] : kModeLabels[i], btnFont);
            list->append(btn);
            connect(btn, &AeroButton::clicked, this, slot);
            row->addWidget(btn);
        }
        row->addStretch(1);
        box->addLayout(row);
    };

    addRow(bandLayout, 0, 6, &m_bandButtons, &BandModeBar::onBandClicked);
    addRow(bandLayout, 7, 13, &m_bandButtons, &BandModeBar::onBandClicked);
    addRow(bandLayout, 14, 20, &m_bandButtons, &BandModeBar::onBandClicked);
    addRow(bandLayout, 21, 21, &m_bandButtons, &BandModeBar::onBandClicked);
    addRow(modeLayout, 0, 5, &m_modeButtons, &BandModeBar::onModeClicked);
    addRow(modeLayout, 6, 11, &m_modeButtons, &BandModeBar::onModeClicked);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(2, 0, 2, 0);
    root->setSpacing(6);
    root->addWidget(bandBox, 0, Qt::AlignTop);
    root->addWidget(modeBox, 0, Qt::AlignTop);

    if (m_settings) {
        setIARURegion(m_settings->getIARURegion());
        bindSlice();
        refreshFromModel();

        connect(m_settings, &Settings::hamBandChanged, this, [this](int r, bool, HamBand band) {
            if (r == m_rx)
                setHamBand(band);
        });
        connect(m_settings, &Settings::dspModeChanged, this, [this](int r, DSPMode mode) {
            if (r == m_rx)
                setDSPMode(mode);
        });
        connect(m_settings, &Settings::iaruRegionChanged, this, &BandModeBar::setIARURegion);
        connect(m_settings, &Settings::currentReceiverChanged, this, &BandModeBar::setReceiver);
    }
}

AeroButton *BandModeBar::makeButton(const QString &text, const QFont &font)
{
    auto *btn = new AeroButton(text, this);
    btn->setFont(font);
    btn->setRoundness(10);
    btn->setFixedSize(kBtnWidth, kBtnHeight);
    btn->setTextColor(QColor(230, 230, 230));
    btn->setTextOnColor(QColor(0, 255, 0));
    btn->setBtnState(AeroButton::OFF);
    return btn;
}

void BandModeBar::highlightExclusive(const QList<AeroButton *> &buttons, int index)
{
    for (int i = 0; i < buttons.size(); ++i) {
        buttons.at(i)->setBtnState(i == index ? AeroButton::ON : AeroButton::OFF);
        buttons.at(i)->update();
    }
}

void BandModeBar::bindSlice()
{
    if (m_slice) {
        disconnect(m_slice, &SliceModel::dspModeChanged, this, &BandModeBar::setDSPMode);
        m_slice = nullptr;
    }
    if (!m_settings)
        return;
    m_slice = m_settings->sliceModel(m_rx);
    if (m_slice)
        connect(m_slice, &SliceModel::dspModeChanged, this, &BandModeBar::setDSPMode);
}

void BandModeBar::refreshFromModel()
{
    if (!m_settings)
        return;
    setHamBand(m_settings->getCurrentHamBand(m_rx));
    setDSPMode(m_slice ? m_slice->dspMode() : m_settings->getDSPMode(m_rx));
}

void BandModeBar::setReceiver(int rx)
{
    if (m_rx == rx && m_slice)
        return;
    m_rx = rx;
    bindSlice();
    refreshFromModel();
}

void BandModeBar::setHamBand(HamBand band)
{
    highlightExclusive(m_bandButtons, static_cast<int>(band));
}

void BandModeBar::setDSPMode(DSPMode mode)
{
    highlightExclusive(m_modeButtons, static_cast<int>(mode));
}

void BandModeBar::setIARURegion(IARURegion region)
{
    const bool has125cm = (region == region2);
    const bool has33cm = (region == region2);
    if (cm125 < m_bandButtons.size())
        m_bandButtons.at(cm125)->setEnabled(has125cm);
    if (cm33 < m_bandButtons.size())
        m_bandButtons.at(cm33)->setEnabled(has33cm);
}

void BandModeBar::onBandClicked()
{
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    const int index = m_bandButtons.indexOf(button);
    if (index < 0 || !m_settings)
        return;
    highlightExclusive(m_bandButtons, index);
    m_settings->applyHamBand(m_rx, static_cast<HamBand>(index));
}

void BandModeBar::onModeClicked()
{
    AeroButton *button = qobject_cast<AeroButton *>(sender());
    const int index = m_modeButtons.indexOf(button);
    if (index < 0)
        return;
    const DSPMode mode = static_cast<DSPMode>(index);
    highlightExclusive(m_modeButtons, index);
    if (m_slice)
        m_slice->setDspMode(mode);
    if (m_settings)
        m_settings->setDSPMode(m_rx, mode);
}
