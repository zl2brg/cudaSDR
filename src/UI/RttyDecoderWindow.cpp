/**
 * @file  RttyDecoderWindow.cpp
 * @brief Independent movable desktop window for the RTTY demodulator, oscilloscope, and terminal.
 * @author Simon Eatough <simon.eatough@gmail.com>
 * @date 2026-09-22
 */

#include "RttyDecoderWindow.h"
#include "Models/SliceModel.h"
#include "cusdr_settings.h"

#include <QPainter>
#include <QPaintEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QPushButton>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QActionGroup>
#include <QLabel>
#include <QTextCursor>
#include <QtMath>

// ============================================================================
// RttyScopeWidget Implementation
// ============================================================================

RttyScopeWidget::RttyScopeWidget(SliceModel *slice, QWidget *parent)
    : QWidget(parent)
    , m_sliceModel(slice)
{
    setMinimumSize(110, 130);
    setMaximumWidth(130);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setToolTip(tr("Lissajous audio tuning scope. Mark and Space tones form orthogonal ellipses when tuned."));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);
    layout->addStretch();

    QHBoxLayout *nudgeLayout = new QHBoxLayout();
    nudgeLayout->setContentsMargins(0, 0, 0, 0);
    nudgeLayout->setSpacing(4);

    m_nudgeLeftBtn = new QPushButton(QStringLiteral("\u25C0"), this);
    m_nudgeLeftBtn->setToolTip(tr("Nudge tone frequency -5 Hz"));
    m_nudgeLeftBtn->setFixedSize(22, 18);
    m_nudgeLeftBtn->setStyleSheet(
        "QPushButton { background: #1b242e; color: #c5d1de; border: 1px solid #334455; border-radius: 3px; font-size: 9px; }"
        "QPushButton:hover { background: #2f455d; color: #ffffff; }"
        "QPushButton:pressed { background: #131b22; }"
    );

    m_nudgeRightBtn = new QPushButton(QStringLiteral("\u25B6"), this);
    m_nudgeRightBtn->setToolTip(tr("Nudge tone frequency +5 Hz"));
    m_nudgeRightBtn->setFixedSize(22, 18);
    m_nudgeRightBtn->setStyleSheet(
        "QPushButton { background: #1b242e; color: #c5d1de; border: 1px solid #334455; border-radius: 3px; font-size: 9px; }"
        "QPushButton:hover { background: #2f455d; color: #ffffff; }"
        "QPushButton:pressed { background: #131b22; }"
    );

    nudgeLayout->addStretch();
    nudgeLayout->addWidget(m_nudgeLeftBtn);
    nudgeLayout->addWidget(m_nudgeRightBtn);
    nudgeLayout->addStretch();

    layout->addLayout(nudgeLayout);

    connect(m_nudgeLeftBtn, &QPushButton::clicked, this, [this]() {
        if (m_sliceModel) {
            const float next = qBound(300.0f, m_sliceModel->rttyCenterFreq() - 5.0f, 4000.0f);
            m_sliceModel->setRttyCenterFreq(next);
            update();
        }
    });

    connect(m_nudgeRightBtn, &QPushButton::clicked, this, [this]() {
        if (m_sliceModel) {
            const float next = qBound(300.0f, m_sliceModel->rttyCenterFreq() + 5.0f, 4000.0f);
            m_sliceModel->setRttyCenterFreq(next);
            update();
        }
    });

    if (m_sliceModel) {
        connect(m_sliceModel, &SliceModel::rttyScopeTraceChanged, this, qOverload<>(&QWidget::update));
        connect(m_sliceModel, &SliceModel::rttyMarkFreqChanged, this, [this](float) { update(); });
        connect(m_sliceModel, &SliceModel::rttyToneLockedChanged, this, [this](bool) { update(); });
    }
}

void RttyScopeWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int w = width();
    const int h = height();
    const int scopeH = qMax(40, h - 26);
    const int size = qMin(w - 4, scopeH);
    const int x0 = (w - size) / 2;
    const int y0 = 2;
    const QRect scopeRect(x0, y0, size, size);

    // Background & border
    p.fillRect(scopeRect, QColor(8, 12, 14));
    p.setPen(QColor(38, 43, 48));
    p.drawRect(scopeRect.adjusted(0, 0, -1, -1));

    const float cx = scopeRect.center().x();
    const float cy = scopeRect.center().y();

    // Crosshairs
    p.setPen(QPen(QColor(120, 130, 140, 110), 1.0f));
    p.drawLine(x0 + 1, qRound(cy), x0 + size - 1, qRound(cy));
    p.drawLine(qRound(cx), y0 + 1, qRound(cx), y0 + size - 1);

    // Lissajous audio trace
    if (m_sliceModel) {
        const QVector<float> xs = m_sliceModel->rttyScopeXs();
        const QVector<float> ys = m_sliceModel->rttyScopeYs();
        const int n = qMin(xs.size(), ys.size());
        if (n >= 2) {
            float frameMax = 0.0f;
            for (int i = 0; i < n; ++i) {
                frameMax = qMax(frameMax, qAbs(xs.at(i)));
                frameMax = qMax(frameMax, qAbs(ys.at(i)));
            }
            if (frameMax > m_peak)
                m_peak = frameMax;
            else
                m_peak = m_peak * 0.95f + frameMax * 0.05f;

            const float scale = (float(size) * 0.5f * 0.88f) / (m_peak + 1.0e-6f);
            QPolygonF poly;
            poly.reserve(n);
            for (int i = 0; i < n; ++i) {
                poly << QPointF(cx + xs.at(i) * scale, cy - ys.at(i) * scale);
            }
            const bool locked = m_sliceModel->rttyToneLocked();
            p.setPen(QPen(locked ? QColor(74, 222, 128, 240) : QColor(50, 180, 160, 200), 1.25f));
            p.drawPolyline(poly);
        }
    }

    // "TUNING" label top left
    QFont tiny = font();
    tiny.setPointSize(7);
    p.setFont(tiny);
    p.setPen(QColor(110, 120, 130));
    p.drawText(x0 + 4, y0 + 10, QStringLiteral("TUNING"));

    // Tone frequency center text
    if (m_sliceModel) {
        float markHz = m_sliceModel->rttyMarkFreq();
        if (markHz < 100.0f) {
            const float half = m_sliceModel->rttyShiftHz() * 0.5f;
            markHz = m_sliceModel->rttyCenterFreq() + (m_sliceModel->rttyReverse() ? half : -half);
        }
        QFont f = font();
        f.setPointSize(8);
        f.setBold(true);
        p.setFont(f);
        p.setPen(QColor(197, 209, 222));
        const QString hzStr = QStringLiteral("%1 Hz").arg(qRound(markHz));
        p.drawText(QRect(x0, y0 + size - 14, size, 12), Qt::AlignCenter, hzStr);
    }
}

void RttyScopeWidget::contextMenuEvent(QContextMenuEvent *event) {
    if (RttyDecoderWindow *win = qobject_cast<RttyDecoderWindow*>(window())) {
        win->showConfigMenu(event->globalPos());
        event->accept();
    }
}

// ============================================================================
// RttyDecoderWindow Implementation
// ============================================================================

RttyDecoderWindow::RttyDecoderWindow(SliceModel *slice, Settings *settings, int rx, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint)
    , m_sliceModel(slice)
    , m_settings(settings)
    , m_rx(rx)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setWindowTitle(tr("cudaSDR - RTTY Decoder (RX %1)").arg(m_rx + 1));
    setupUi();

    if (m_sliceModel) {
        // Restore saved desktop geometry
        const QPoint pos = m_sliceModel->rttyWindowPos();
        const QSize sz = m_sliceModel->rttyWindowSize();
        m_isInternalGeometryChange = true;
        if (sz.isValid() && sz.width() >= 320 && sz.height() >= 140) {
            resize(sz);
        } else {
            resize(680, 260);
        }
        if (!pos.isNull() && pos.x() >= -3000 && pos.y() >= -3000) {
            move(pos);
        }
        m_isInternalGeometryChange = false;

        updateScopeVisibility(m_sliceModel->rttyScopeVisible());
        updateStatusBadge();
        onDecodedTextChanged(m_sliceModel->rttyDecodedText());

        connect(m_sliceModel, &SliceModel::rttyDecodeEnabledChanged, this, [this](bool enabled) {
            if (!enabled) {
                hide();
            } else if (m_sliceModel->rttyFloating()) {
                show();
                raise();
            }
        });

        connect(m_sliceModel, &SliceModel::rttyFloatingChanged, this, [this](bool floating) {
            if (floating && m_sliceModel->rttyDecodeEnabled()) {
                show();
                raise();
            } else {
                hide();
            }
        });

        connect(m_sliceModel, &SliceModel::rttyDecodedTextChanged, this, &RttyDecoderWindow::onDecodedTextChanged);
        connect(m_sliceModel, &SliceModel::rttyToneLockedChanged, this, [this](bool) { updateStatusBadge(); });
        connect(m_sliceModel, &SliceModel::rttySnrDbChanged, this, [this](float) { updateStatusBadge(); });
        connect(m_sliceModel, &SliceModel::rttyShiftHzChanged, this, [this](float) { updateStatusBadge(); });
        connect(m_sliceModel, &SliceModel::rttyBaudRateChanged, this, [this](float) { updateStatusBadge(); });
        connect(m_sliceModel, &SliceModel::rttyReverseChanged, this, [this](bool) { updateStatusBadge(); });
        connect(m_sliceModel, &SliceModel::rttyAutoDetectChanged, this, [this](bool autoDet) {
            if (m_autoBtn) {
                const QSignalBlocker blocker(m_autoBtn);
                m_autoBtn->setChecked(autoDet);
            }
            updateStatusBadge();
        });
        connect(m_sliceModel, &SliceModel::rttyScopeVisibleChanged, this, &RttyDecoderWindow::updateScopeVisibility);
    }
}

void RttyDecoderWindow::setupUi() {
    setStyleSheet(
        "QWidget { background-color: #121820; color: #d8e2ec; font-family: sans-serif; font-size: 11px; }"
        "QPushButton { background: #1c2633; color: #d8e2ec; border: 1px solid #334455; border-radius: 3px; padding: 3px 8px; font-weight: bold; }"
        "QPushButton:hover { background: #283749; color: #64b5f6; border-color: #446688; }"
        "QPushButton:pressed { background: #141c25; }"
        "QPushButton:checked { background: #264263; color: #ffffff; border-color: #5588bb; }"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // Top Header / Toolbar
    QHBoxLayout *topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 0, 0, 0);
    topBar->setSpacing(6);

    // Badge button
    m_badgeBtn = new QPushButton(this);
    m_badgeBtn->setToolTip(tr("RTTY settings and audio status. Click to open configuration menu."));
    m_badgeBtn->setStyleSheet(
        "QPushButton { background: #18222e; color: #e0e8f0; border: 1px solid #3a5068; border-radius: 3px; padding: 3px 10px; font-weight: bold; }"
        "QPushButton:hover { background: #243548; color: #64b5f6; border-color: #4a7090; }"
    );
    connect(m_badgeBtn, &QPushButton::clicked, this, [this]() {
        showConfigMenu(m_badgeBtn->mapToGlobal(QPoint(0, m_badgeBtn->height())));
    });
    topBar->addWidget(m_badgeBtn);

    // Auto Detect toggle button
    m_autoBtn = new QPushButton(tr("Auto"), this);
    m_autoBtn->setCheckable(true);
    m_autoBtn->setToolTip(tr("Toggle real-time automatic shift and baud rate classification"));
    m_autoBtn->setChecked(m_sliceModel ? m_sliceModel->rttyAutoDetect() : false);
    connect(m_autoBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (m_sliceModel) m_sliceModel->setRttyAutoDetect(checked);
    });
    topBar->addWidget(m_autoBtn);

    // Scope toggle button
    m_scopeToggleBtn = new QPushButton(tr("Scope"), this);
    m_scopeToggleBtn->setCheckable(true);
    m_scopeToggleBtn->setChecked(m_sliceModel ? m_sliceModel->rttyScopeVisible() : true);
    m_scopeToggleBtn->setToolTip(tr("Show or hide the audio Lissajous tuning scope"));
    connect(m_scopeToggleBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (m_sliceModel) m_sliceModel->setRttyScopeVisible(checked);
    });
    topBar->addWidget(m_scopeToggleBtn);

    topBar->addStretch();

    // Clear text button
    m_clearBtn = new QPushButton(tr("Clear"), this);
    m_clearBtn->setToolTip(tr("Clear decoded text buffer"));
    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        if (m_sliceModel) m_sliceModel->setRttyDecodedText(QString());
    });
    topBar->addWidget(m_clearBtn);

    // Dock back into panadapter button
    m_dockBtn = new QPushButton(QStringLiteral("\u2913 Dock"), this);
    m_dockBtn->setToolTip(tr("Dock RTTY decoder back into the panadapter display"));
    m_dockBtn->setStyleSheet(
        "QPushButton { background: #1a2a3a; color: #90caf9; border: 1px solid #2e4a6a; border-radius: 3px; padding: 3px 8px; font-weight: bold; }"
        "QPushButton:hover { background: #223c56; color: #ffffff; border-color: #42709c; }"
    );
    connect(m_dockBtn, &QPushButton::clicked, this, [this]() {
        if (m_sliceModel) m_sliceModel->setRttyFloating(false);
    });
    topBar->addWidget(m_dockBtn);

    mainLayout->addLayout(topBar);

    // Body Area: Scope + Terminal
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(6);

    // Scope container
    m_scopeContainer = new QWidget(this);
    QVBoxLayout *scopeContLayout = new QVBoxLayout(m_scopeContainer);
    scopeContLayout->setContentsMargins(0, 0, 0, 0);
    scopeContLayout->setSpacing(0);
    m_scopeWidget = new RttyScopeWidget(m_sliceModel, m_scopeContainer);
    scopeContLayout->addWidget(m_scopeWidget);
    bodyLayout->addWidget(m_scopeContainer);

    // Terminal container (with floating Jump to Latest button)
    QWidget *terminalContainer = new QWidget(this);
    QVBoxLayout *termContLayout = new QVBoxLayout(terminalContainer);
    termContLayout->setContentsMargins(0, 0, 0, 0);
    termContLayout->setSpacing(0);

    m_terminal = new QPlainTextEdit(terminalContainer);
    m_terminal->setReadOnly(true);
    m_terminal->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_terminal->setStyleSheet(
        "QPlainTextEdit {"
        "  background-color: #0c0e10;"
        "  color: #c5d1de;"
        "  border: 1px solid #2d4155;"
        "  border-radius: 3px;"
        "  font-family: 'DejaVu Sans Mono', 'Courier New', monospace;"
        "  font-size: 11px;"
        "  padding: 4px;"
        "  selection-background-color: #2b4c7e;"
        "  selection-color: #ffffff;"
        "}"
        "QScrollBar:vertical {"
        "  background: #101418;"
        "  width: 10px;"
        "  margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #334455;"
        "  min-height: 20px;"
        "  border-radius: 2px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #4a627d;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    );

    termContLayout->addWidget(m_terminal);

    // Jump to latest button overlay
    m_jumpLatestBtn = new QPushButton(QStringLiteral("\u2193 latest"), m_terminal);
    m_jumpLatestBtn->setToolTip(tr("Jump to latest received text and resume auto-scrolling"));
    m_jumpLatestBtn->setStyleSheet(
        "QPushButton { background: rgba(35, 55, 80, 220); color: #ffffff; border: 1px solid #4a7090; border-radius: 3px; padding: 2px 8px; font-size: 10px; font-weight: bold; }"
        "QPushButton:hover { background: rgba(45, 75, 110, 240); color: #64b5f6; }"
    );
    m_jumpLatestBtn->hide();
    connect(m_jumpLatestBtn, &QPushButton::clicked, this, &RttyDecoderWindow::scrollToBottom);

    bodyLayout->addWidget(terminalContainer, 1);
    mainLayout->addLayout(bodyLayout, 1);

    connect(m_terminal->verticalScrollBar(), &QScrollBar::valueChanged, this, &RttyDecoderWindow::onScrollValueChanged);
}

void RttyDecoderWindow::updateScopeVisibility(bool visible) {
    if (m_scopeContainer)
        m_scopeContainer->setVisible(visible);
    if (m_scopeToggleBtn) {
        const QSignalBlocker blocker(m_scopeToggleBtn);
        m_scopeToggleBtn->setChecked(visible);
    }
}

void RttyDecoderWindow::updateStatusBadge() {
    if (!m_sliceModel || !m_badgeBtn) return;

    const bool autoDetect = m_sliceModel->rttyAutoDetect();
    const bool weather = m_sliceModel->rttyWeatherProfile();
    const float baud = m_sliceModel->rttyBaudRate();
    const float shift = m_sliceModel->rttyShiftHz();
    const bool rev = m_sliceModel->rttyReverse();
    const bool locked = m_sliceModel->rttyToneLocked();
    const float snr = m_sliceModel->rttySnrDb();

    const QString modePrefix = autoDetect ? QStringLiteral("AUTO")
        : (weather ? QStringLiteral("WX") : QStringLiteral("RTTY"));
    const QString polStr = rev ? QStringLiteral("REV") : QStringLiteral("NOR");

    QString badgeText = QStringLiteral("%1 %2/%3 [%4]").arg(modePrefix).arg(qRound(baud)).arg(qRound(shift)).arg(polStr);
    if (locked && snr > 0.0f) {
        badgeText.append(QStringLiteral(" %1dB").arg(qRound(snr)));
    }
    badgeText.append(QStringLiteral(" \u2699"));

    m_badgeBtn->setText(badgeText);

    // Styling with Tone Lock color cue
    const QString lockBorder = locked ? QStringLiteral("#32f096") : QStringLiteral("#3a5068");
    m_badgeBtn->setStyleSheet(QString(
        "QPushButton { background: #18222e; color: #e0e8f0; border: 1px solid %1; border-radius: 3px; padding: 3px 10px; font-weight: bold; }"
        "QPushButton:hover { background: #243548; color: #64b5f6; border-color: #4a7090; }"
    ).arg(lockBorder));
}

void RttyDecoderWindow::onDecodedTextChanged(const QString &text) {
    if (!m_terminal) return;

    if (text.isEmpty()) {
        m_terminal->clear();
        m_lastRenderedText.clear();
        return;
    }

    if (text.startsWith(m_lastRenderedText)) {
        const QString appendPart = text.mid(m_lastRenderedText.length());
        if (!appendPart.isEmpty()) {
            QTextCursor cursor = m_terminal->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(appendPart);
        }
    } else {
        m_terminal->setPlainText(text);
    }
    m_lastRenderedText = text;

    if (m_followLatest) {
        scrollToBottom();
    }
}

void RttyDecoderWindow::onScrollValueChanged(int value) {
    if (!m_terminal) return;
    const int max = m_terminal->verticalScrollBar()->maximum();
    m_followLatest = (value >= max - 4);
    if (m_jumpLatestBtn) {
        m_jumpLatestBtn->setVisible(!m_followLatest);
        if (!m_followLatest) {
            // Position button at bottom-right inside terminal viewport
            const int bw = m_jumpLatestBtn->width() > 0 ? m_jumpLatestBtn->width() : 60;
            const int bh = m_jumpLatestBtn->height() > 0 ? m_jumpLatestBtn->height() : 22;
            m_jumpLatestBtn->move(m_terminal->width() - bw - 18, m_terminal->height() - bh - 6);
        }
    }
}

void RttyDecoderWindow::scrollToBottom() {
    if (!m_terminal) return;
    m_terminal->verticalScrollBar()->setValue(m_terminal->verticalScrollBar()->maximum());
    m_followLatest = true;
    if (m_jumpLatestBtn)
        m_jumpLatestBtn->hide();
}

void RttyDecoderWindow::closeEvent(QCloseEvent *event) {
    if (m_sliceModel) {
        m_sliceModel->setRttyDecodeEnabled(false);
    }
    event->accept();
}

void RttyDecoderWindow::moveEvent(QMoveEvent *event) {
    QWidget::moveEvent(event);
    if (!m_isInternalGeometryChange && m_sliceModel && isVisible()) {
        m_sliceModel->setRttyWindowPos(pos());
    }
}

void RttyDecoderWindow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (!m_isInternalGeometryChange && m_sliceModel && isVisible()) {
        m_sliceModel->setRttyWindowSize(size());
    }
    if (m_jumpLatestBtn && m_jumpLatestBtn->isVisible() && m_terminal) {
        const int bw = m_jumpLatestBtn->width() > 0 ? m_jumpLatestBtn->width() : 60;
        const int bh = m_jumpLatestBtn->height() > 0 ? m_jumpLatestBtn->height() : 22;
        m_jumpLatestBtn->move(m_terminal->width() - bw - 18, m_terminal->height() - bh - 6);
    }
}

void RttyDecoderWindow::contextMenuEvent(QContextMenuEvent *event) {
    showConfigMenu(event->globalPos());
    event->accept();
}

void RttyDecoderWindow::showConfigMenu(const QPoint &globalPos) {
    QMenu *menu = createConfigMenu(this, m_sliceModel, m_settings, m_rx, [this]() {
        updateStatusBadge();
    });
    if (menu) {
        menu->exec(globalPos);
        delete menu;
    }
}

// ============================================================================
// Shared Config Menu
// ============================================================================

QMenu* RttyDecoderWindow::createConfigMenu(QWidget *parent, SliceModel *slice, Settings *settings, int rx, std::function<void()> updateCb) {
    if (!slice) return nullptr;

    QMenu *menu = new QMenu(parent);
    menu->setStyleSheet(
        "QMenu {"
        "  background-color: #1a222d;"
        "  color: #d8e2ec;"
        "  border: 1px solid #334455;"
        "  padding: 4px;"
        "}"
        "QMenu::item {"
        "  padding: 5px 24px 5px 20px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #2b3b4c;"
        "  color: #64b5f6;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background: #334455;"
        "  margin: 4px 6px;"
        "}"
    );

    QAction *titleAction = menu->addAction(QStringLiteral("RTTY Decoder (RX %1)").arg(rx + 1));
    QFont boldFont = titleAction->font();
    boldFont.setBold(true);
    titleAction->setFont(boldFont);
    titleAction->setEnabled(false);
    menu->addSeparator();

    // Auto Detect
    QAction *autoAct = menu->addAction(QStringLiteral("Auto Detect (Shift & Baud)"));
    autoAct->setCheckable(true);
    autoAct->setChecked(slice->rttyAutoDetect());
    QObject::connect(autoAct, &QAction::toggled, parent, [slice, updateCb](bool checked) {
        slice->setRttyAutoDetect(checked);
        if (updateCb) updateCb();
    });

    menu->addSeparator();

    // Weather Profile
    QAction *wxAct = menu->addAction(QStringLiteral("Weather Profile (50 baud, 450 Hz)"));
    wxAct->setCheckable(true);
    wxAct->setChecked(slice->rttyWeatherProfile());
    QObject::connect(wxAct, &QAction::triggered, parent, [slice, updateCb](bool checked) {
        slice->setRttyWeatherProfile(checked);
        if (updateCb) updateCb();
    });

    menu->addSeparator();

    // Shift submenu
    QMenu *shiftMenu = menu->addMenu(QStringLiteral("Shift (%1 Hz)").arg(qRound(slice->rttyShiftHz())));
    QActionGroup *shiftGroup = new QActionGroup(shiftMenu);
    const struct { const char *label; float shift; } shifts[] = {
        {"170 Hz (Standard Amateur)", 170.0f},
        {"200 Hz", 200.0f},
        {"425 Hz (Commercial / Nav)", 425.0f},
        {"450 Hz (Commercial / Weather)", 450.0f},
        {"850 Hz (Wide)", 850.0f}
    };
    for (const auto &item : shifts) {
        QAction *act = shiftMenu->addAction(item.label);
        act->setCheckable(true);
        shiftGroup->addAction(act);
        if (qAbs(slice->rttyShiftHz() - item.shift) < 10.0f) {
            act->setChecked(true);
        }
        const float sVal = item.shift;
        QObject::connect(act, &QAction::triggered, parent, [slice, sVal, updateCb]() {
            slice->setRttyShiftHz(sVal);
            if (updateCb) updateCb();
        });
    }

    // Audio Tone Pair / Center Frequency submenu
    QMenu *toneMenu = menu->addMenu(QStringLiteral("Audio Tone Pair (%1 Hz)").arg(qRound(slice->rttyCenterFreq())));
    QActionGroup *toneGroup = new QActionGroup(toneMenu);
    const struct { const char *label; float freq; } tonePairs[] = {
        {"2210 Hz (Standard High Tones: 2125/2295 Hz)", 2210.0f},
        {"1360 Hz (Standard Low Tones: 1275/1445 Hz)", 1360.0f},
        {"1750 Hz (Commercial / Weather)", 1750.0f}
    };
    for (const auto &item : tonePairs) {
        QAction *act = toneMenu->addAction(item.label);
        act->setCheckable(true);
        toneGroup->addAction(act);
        if (qAbs(slice->rttyCenterFreq() - item.freq) < 15.0f) {
            act->setChecked(true);
        }
        const float fVal = item.freq;
        QObject::connect(act, &QAction::triggered, parent, [slice, fVal, updateCb]() {
            slice->setRttyCenterFreq(fVal);
            if (updateCb) updateCb();
        });
    }

    // Baud submenu
    QMenu *baudMenu = menu->addMenu(QStringLiteral("Baud Rate (%1 Baud)").arg(QString::number(slice->rttyBaudRate(), 'f', 1)));
    QActionGroup *baudGroup = new QActionGroup(baudMenu);
    const struct { const char *label; float baud; } bauds[] = {
        {"45.45 Baud (Standard Amateur)", 45.4545f},
        {"50.00 Baud", 50.0f},
        {"75.00 Baud", 75.0f},
        {"100.0 Baud", 100.0f}
    };
    for (const auto &item : bauds) {
        QAction *act = baudMenu->addAction(item.label);
        act->setCheckable(true);
        baudGroup->addAction(act);
        if (qAbs(slice->rttyBaudRate() - item.baud) < 1.0f) {
            act->setChecked(true);
        }
        const float bVal = item.baud;
        QObject::connect(act, &QAction::triggered, parent, [slice, bVal, updateCb]() {
            slice->setRttyBaudRate(bVal);
            if (updateCb) updateCb();
        });
    }

    // Reverse Polarity
    QAction *revAct = menu->addAction(QStringLiteral("Reverse Polarity (REV)"));
    revAct->setCheckable(true);
    revAct->setChecked(slice->rttyReverse());
    QObject::connect(revAct, &QAction::toggled, parent, [slice, updateCb](bool checked) {
        slice->setRttyReverse(checked);
        if (updateCb) updateCb();
    });

    // AFC Tracking
    QAction *afcAct = menu->addAction(QStringLiteral("AFC (Auto Frequency Tracking)"));
    afcAct->setCheckable(true);
    afcAct->setChecked(slice->rttyAfc());
    QObject::connect(afcAct, &QAction::toggled, parent, [slice, updateCb](bool checked) {
        slice->setRttyAfc(checked);
        if (updateCb) updateCb();
    });

    // Squelch submenu
    QMenu *squelchMenu = menu->addMenu(QStringLiteral("Squelch Sensitivity"));
    QActionGroup *squelchGroup = new QActionGroup(squelchMenu);
    const struct { const char *label; float thresh; } squelches[] = {
        {"Off (0.00)", 0.0f},
        {"Low (0.20)", 0.20f},
        {"Medium (0.35)", 0.35f},
        {"High (0.50)", 0.50f}
    };
    for (const auto &item : squelches) {
        QAction *act = squelchMenu->addAction(item.label);
        act->setCheckable(true);
        squelchGroup->addAction(act);
        if (qAbs(slice->rttySquelch() - item.thresh) < 0.08f) {
            act->setChecked(true);
        }
        const float sqVal = item.thresh;
        QObject::connect(act, &QAction::triggered, parent, [slice, sqVal, updateCb]() {
            slice->setRttySquelch(sqVal);
            if (updateCb) updateCb();
        });
    }

    menu->addSeparator();

    // Log to file
    QAction *logAct = menu->addAction(QStringLiteral("Log Decoded Text to File"));
    logAct->setCheckable(true);
    logAct->setChecked(slice->rttyLogToFile());
    QObject::connect(logAct, &QAction::toggled, parent, [slice, updateCb](bool checked) {
        slice->setRttyLogToFile(checked);
        if (updateCb) updateCb();
    });

    // Console Debug Diagnostics
    QAction *dbgAct = menu->addAction(QStringLiteral("Console Diagnostics (Terminal)"));
    dbgAct->setCheckable(true);
    dbgAct->setChecked(qEnvironmentVariableIsSet("CUDASDR_RTTY_DEBUG"));
    QObject::connect(dbgAct, &QAction::toggled, parent, [](bool checked) {
        if (checked) {
            qputenv("CUDASDR_RTTY_DEBUG", "1");
            qInfo("[RTTY] Console debug diagnostics enabled. Streaming live demodulator & decoder metrics to terminal.");
        } else {
            qunsetenv("CUDASDR_RTTY_DEBUG");
            qInfo("[RTTY] Console debug diagnostics disabled.");
        }
    });

    menu->addSeparator();

    // Clear Decoded Text
    QAction *clearAct = menu->addAction(QStringLiteral("Clear Decoded Text"));
    QObject::connect(clearAct, &QAction::triggered, parent, [slice, updateCb]() {
        slice->setRttyDecodedText(QString());
        if (updateCb) updateCb();
    });

    // Dock / Detach toggle
    const QString dockLabel = slice->rttyFloating()
        ? QStringLiteral("Dock into Panadapter (\u2913)")
        : QStringLiteral("Detach to Desktop Window (\u29C9)");
    QAction *dockAct = menu->addAction(dockLabel);
    QObject::connect(dockAct, &QAction::triggered, parent, [slice, settings, rx, updateCb]() {
        const bool nextFloating = !slice->rttyFloating();
        slice->setRttyFloating(nextFloating);
        if (settings) {
            settings->setRttyFloating(rx, nextFloating);
        }
        if (updateCb) updateCb();
    });

    menu->addSeparator();

    // Close Decoder
    QAction *closeAct = menu->addAction(QStringLiteral("Close RTTY Decoder"));
    QObject::connect(closeAct, &QAction::triggered, parent, [slice, settings, rx, updateCb]() {
        if (settings) {
            settings->setRttyDecode(rx, false);
        } else {
            slice->setRttyDecodeEnabled(false);
        }
        if (updateCb) updateCb();
    });

    return menu;
}
