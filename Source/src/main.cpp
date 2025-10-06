/**
* @file  main.cpp
* @brief main
* @author Hermann von Hasseln, DL3HVH
* @version 0.2
* @date 2025-09-11
*/

/* * Copyright 2010, 2011, 2012 Hermann von Hasseln, DL3HVH and Contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Util/cusdr_splash.h"
#include "cusdr_settings.h"
#include "fftw3.h"
#include "cusdr_mainWidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <QStyle>
#include <QScreen>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QElapsedTimer>
#include <QLoggingCategory> // NOTE: Added for the updated message handler

#if defined(Q_OS_WIN32)
#include "Util/cusdr_cpuUsage.h"
#elif defined(Q_OS_LINUX)
#include "Util/cusdr_cpuUsage_unix.h"
#include <unistd.h>
#include <errno.h>
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif


// CPU usage
#if defined(Q_OS_WIN32)
DWORD WINAPI WatchItThreadProc(LPVOID lpParam);
CpuUsage usage;

DWORD WINAPI WatchItThreadProc(LPVOID lpParam) {
    Q_UNUSED(lpParam)
    DWORD dummy;
    while (true) {
        short cpuUsage = usage.GetUsage();
        Settings::instance()->setCPULoad(cpuUsage);
        Sleep(1000);
    }
    return dummy;
}
#endif

#define DEBUG
// NOTE: The message handler signature is updated for Qt 6.
void cuSDRMessageHandler(QtMsgType type, const QLoggingCategory &category, const QString &msg) {
    Q_UNUSED(type)
    Q_UNUSED(category) // The context is replaced by category in Qt 6

    QString txt;
    QDateTime date;

    txt = msg;
    txt.prepend(": ");
    txt.prepend(date.currentDateTime().toString());

    QFile outFile("cudaSDR.log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl << Qt::flush;
}

void load_WDSPWisdom() {
    WDSPwisdom(Settings::instance()->cfg_dir.toLocal8Bit().data());
}

int main(int argc, char *argv[]) {

#ifndef DEBUG
    // NOTE: The function name is the same, but it now works with the updated handler signature.
    qInstallMessageHandler(cuSDRMessageHandler);
#endif
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(2, 0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    Settings::instance(&app);

    app.setApplicationName(Settings::instance()->getTitleStr());
    app.setApplicationVersion(Settings::instance()->getVersionStr());

    QPixmap splash_pixmap(":/img/cudaSDRLogo.png");
    CSplashScreen* splash = new CSplashScreen(splash_pixmap);

    splash->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            splash->size(),
            QGuiApplication::primaryScreen()->availableGeometry()));

 splash->show();
    app.setStyleSheet(Settings::instance()->get_appStyleSheet());


    // NOTE: Replaced SleeperThread::msleep() with the modern static QThread::msleep()
    QThread::msleep(500);

    Settings::instance()->setSettingsFilename(QCoreApplication::applicationDirPath() +
                                              "/" + Settings::instance()->getSettingsFilename());

    Settings::instance()->setSettingsLoaded(Settings::instance()->loadSettings() >= 0);

    if (Settings::instance()->getSettingsLoaded()) {
        splash->showMessage(
            "\n      " +
                Settings::instance()->getTitleStr() + " " +
                Settings::instance()->getVersionStr() +
                QObject::tr(":    Settings loaded."),
            Qt::AlignTop | Qt::AlignLeft, Qt::yellow);
            app.processEvents();
        QThread::msleep(100);
    }
    else {
        splash->showMessage(
            "\n      " +
                Settings::instance()->getTitleStr() + " " +
                Settings::instance()->getVersionStr() +
                QObject::tr(":    Settings not loaded."),
            Qt::AlignTop | Qt::AlignLeft, Qt::red);
            app.processEvents();
    }

    // ****************************
    // check for OpenGL
    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    Checking for OpenGL V 2.0 ..."),
        Qt::AlignTop | Qt::AlignLeft, Qt::yellow);
        app.processEvents();
    QThread::msleep(1000);

    QOpenGLContext context;
    if (!context.create()) {
        qDebug() << "Init::\tOpenGL context creation failed!";
        // ... error handling
        return -1;
    }

    QSurfaceFormat surfaceformat = context.format();
    if (surfaceformat.majorVersion() < 2) {
        qDebug() << "Init::\tOpenGL found, but appears to be less than OGL v2.0.";
        splash->showMessage(
            "\n      " +
                Settings::instance()->getTitleStr() + " " +
                Settings::instance()->getVersionStr() +
                QObject::tr(":    found but appears to be less than OGL v2.0"),
            Qt::AlignTop | Qt::AlignLeft, Qt::yellow);
            app.processEvents();
            QThread::msleep(1000);
        splash->hide();

        QMessageBox::critical(nullptr,
                              QApplication::applicationName(),
                              QApplication::applicationName() + "    requires OpenGL v2.0 or later to run.",
                              QMessageBox::Ok);
        return -1;
    }

    qDebug() << "Init::\tOpenGL found.";
    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    OpenGL found."),
        Qt::AlignTop | Qt::AlignLeft, Qt::yellow);
        app.processEvents();
    QThread::msleep(1000);


    // NOTE: Removed obsolete check for QGLFramebufferObject.
    // FBOs are a core part of OpenGL 2.0+ contexts, so this check is no longer needed.
    qDebug() << "Init::\tFramebuffer Objects assumed present.";
    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    OpenGL Frame Buffer support found."),
        Qt::AlignTop | Qt::AlignLeft, Qt::yellow);
    Settings::instance()->setFBOPresence(true);
    QThread::msleep(100);

    // cpu usage
#if defined(Q_OS_WIN32)
    CreateThread(NULL, 0, WatchItThreadProc, NULL, 0, NULL);
#endif

    // setup main window
    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    setting up main window .."),
        Qt::AlignTop | Qt::AlignLeft, Qt::yellow);

    qDebug() << "Init::\tmain window setup ...";
    MainWindow mainWindow;
    mainWindow.setup();
    qDebug() << "Init::\tmain window setup done.";

    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    Displaying main window .."),
        Qt::AlignTop | Qt::AlignLeft, Qt::yellow);
    QThread::msleep(300);
    splash->finish(&mainWindow);
    delete splash;

    //*************************************************************************
    load_WDSPWisdom();

    mainWindow.show();
    mainWindow.update();
    mainWindow.setFocus();

#if defined(Q_OS_LINUX)
    cusdr_cpuUsage *cpu_load = new cusdr_cpuUsage();
    cpu_load->start();
#endif

    qDebug() << "Init::\trunning application ...\n";
    return app.exec();
}
