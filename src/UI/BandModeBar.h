#ifndef BAND_MODE_BAR_H
#define BAND_MODE_BAR_H

#include <QWidget>
#include <QList>

#include "cusdr_hamDatabase.h"

class AeroButton;
class Settings;
class SliceModel;

/** Compact band + DSP-mode strip matching the radio popup (no extra modes). */
class BandModeBar : public QWidget
{
    Q_OBJECT

public:
    BandModeBar(Settings *settings, QWidget *parent = nullptr);

    void setReceiver(int rx);
    void setHamBand(HamBand band);
    void setDSPMode(DSPMode mode);
    void setIARURegion(IARURegion region);

private:
    AeroButton *makeButton(const QString &text, const QFont &font);
    void highlightExclusive(const QList<AeroButton *> &buttons, int index);
    void bindSlice();
    void refreshFromModel();
    void onBandClicked();
    void onModeClicked();

    Settings *m_settings = nullptr;
    SliceModel *m_slice = nullptr;
    int m_rx = 0;
    QList<AeroButton *> m_bandButtons;
    QList<AeroButton *> m_modeButtons;
};

#endif
