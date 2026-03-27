/**
* @file  cusdr_splash.cpp
* @brief splash screen class for cuSDR
* @author Hermann von Hasseln, DL3HVH
* @version 0.2
* @date 2025-09-11
*/

/*
 *	Based on: Qt Developer Network; QSplashScreen replacement for semitransparent images
 *
 *	http://developer.qt.nokia.com/wiki/QSplashScreen_replacement_for_semitransparent_images
 *
 *	License: http://creativecommons.org/licenses/by-sa/2.5/legalcode
 */

#include "cusdr_splash.h"
#include <QPainter>
#include <QDateTime>

// NOTE: Inheriting from QWidget is more direct than QFrame for a custom-painted widget.
// The constructor also now initializes all members and takes a dynamic copyright string.
CSplashScreen::CSplashScreen(const QPixmap& thePixmap)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , itsPixmap(thePixmap)
    , itsMessage()
    , itsColor(Qt::black)
    , itsAlignment(Qt::AlignLeft)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFixedSize(itsPixmap.size());

    // Dynamically create the copyright string to always show the current year.
    itsMessage = QString("(C) 2011-2018 DL3HVH, 2020-%i ZL2BRG").arg(QDate::currentDate().year());
};

void CSplashScreen::clearMessage() {
    itsMessage.clear();
    repaint();
}

void CSplashScreen::finish(QWidget *mainWin) {
    if (mainWin) {
        connect(mainWin, &QWidget::destroyed, this, &QWidget::close);
    }
    hide();
}

void CSplashScreen::showMessage(const QString& theMessage, int theAlignment, const QColor& theColor) {
    itsMessage   = theMessage;
    itsAlignment = theAlignment;
    itsColor     = theColor;
    update();
}

// NOTE: Added the C++ 'override' specifier for type safety and clarity.
void CSplashScreen::paintEvent(QPaintEvent* pe) {
    Q_UNUSED(pe)

    // Define drawing rectangles within the paint event
    QRectF textRect = rect().adjusted(5, 5, -10, -10);
    QRectF copyrightRect = rect().adjusted(0, -10, -10, -5);

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Draw the main splash image
    painter.drawPixmap(rect(), itsPixmap);

    // Draw the status message
    painter.setPen(itsColor);
    painter.drawText(textRect, itsAlignment, itsMessage);

    // Draw the copyright message
    painter.setPen(Qt::black);
    painter.drawText(copyrightRect, Qt::AlignBottom | Qt::AlignRight, itsMessage);
}
