#include "basewidget.h"

baseWidget::baseWidget(QWidget *parent, int rx) : QWidget(parent)
  , m_receiver(rx)
  , set(Settings::instance())
  , m_minimumWidgetWidth(set->getMinimumWidgetWidth())
  , m_minimumGroupBoxWidth(set->getMinimumGroupBoxWidth())
  , m_btnSpacing(5)
  , m_mouseOver(false)

{
    m_receiver=0;
    m_receiverDataList = set->getReceiverDataList();
    m_dspMode = m_receiverDataList.at(m_receiver).dspMode;
    m_dspModeList = m_receiverDataList.at(m_receiver).dspModeList;
    m_filterLo = m_receiverDataList.at(m_receiver).filterLo;
    m_filterHi = m_receiverDataList.at(m_receiver).filterHi;
    m_lastVfoFrequencyList = m_receiverDataList.at(m_receiver).lastVfoFrequencyList;
    m_lastCtrFrequencyList = m_receiverDataList.at(m_receiver).lastCenterFrequencyList;

    m_hamBand = m_receiverDataList.at(m_receiver).hamBand;



    setMouseTracking(true);
    setContentsMargins(4, 4, 4, 4);
    setWindowOpacity(0.9);
    setFocusPolicy(Qt::StrongFocus);


}


void baseWidget::setupModeBtn(AeroButton *btn)
{
        btn->setRoundness(0);
        btn->setFixedSize(btn_widths, btn_height);
        btn->setStyleSheet(set->getMiniButtonStyle());
        m_mode_btnList.append(btn);
        CHECKED_CONNECT(btn, SIGNAL(clicked()),this, SLOT(ModebtnCallback()));
 }

void baseWidget::setupFilterBtn(AeroButton *btn)
{
        btn->setRoundness(0);
        btn->setFixedSize(btn_widths, btn_height);
        btn->setStyleSheet(set->getMiniButtonStyle());
        m_filter_btnList.append(btn);
        CHECKED_CONNECT(btn, SIGNAL(clicked()),this, SLOT(FilterbtnCallback()));
 }

void baseWidget::setupBandBtn(AeroButton *btn)
{
        btn->setRoundness(0);
        btn->setFixedSize(btn_widths, btn_height);
        btn->setStyleSheet(set->getMiniButtonStyle());
        m_band_btnList.append(btn);
        CHECKED_CONNECT(btn, SIGNAL(clicked()),this, SLOT(BandbtnCallback()));
 }


QSize baseWidget::sizeHint() const {
    return QSize(m_minimumWidgetWidth, height());
}

QSize baseWidget::minimumSizeHint() const {
    return QSize(m_minimumWidgetWidth, height());
}


baseWidget::~baseWidget(){

}


