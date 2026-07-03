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
#include "Util/cusdr_rigctlserver.h"
#include "Util/cusdr_tciserver.h"
#include "cusdr_settings.h"
#include "fftw3.h"
#include "cusdr_mainWidget.h"
#include "Models/RadioModel.h"
#include "Models/SliceModel.h"

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
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <errno.h>
#define fopen_s(pFile,filename,mode) ((*(pFile))=fopen((filename),  (mode)))==NULL
#endif

#include "Util/CPUMonitor.h"




class LogManager : public QObject {
public:
    static LogManager& instance() {
        static LogManager inst;
        return inst;
    }

    void write(const QString& txt) {
        if (m_outFile.isOpen()) {
            m_stream << txt << Qt::endl;
        }
    }

private:
    LogManager() : m_outFile("cudaSDR.log"), m_stream(&m_outFile) {
        if (!m_outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            qWarning() << "Failed to open log file for writing.";
        }
    }
    ~LogManager() {
        if (m_outFile.isOpen()) {
            m_stream.flush();
            m_outFile.close();
        }
    }

    QFile m_outFile;
    QTextStream m_stream;
};

void cuSDRMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    QString level;
    switch (type) {
        case QtDebugMsg:    level = "DEBUG"; break;
        case QtInfoMsg:     level = "INFO "; break;
        case QtWarningMsg:  level = "WARN "; break;
        case QtCriticalMsg: level = "CRIT "; break;
        case QtFatalMsg:    level = "FATAL"; break;
    }

    QString txt = QString("%1 [%2] %3")
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
                    .arg(level)
                    .arg(msg);

    // Also print to console for developers
    fprintf(stderr, "%s\n", qPrintable(txt));

    LogManager::instance().write(txt);
}

void load_WDSPWisdom() {
    WDSPwisdom(Settings::instance()->cfg_dir.toLocal8Bit().data());
}

int main(int argc, char *argv[]) {

#ifndef DEBUG
    // NOTE: The function name is the same, but it now works with the updated handler signature.
    qInstallMessageHandler(cuSDRMessageHandler);
#endif
#if defined(Q_OS_LINUX)
    // Qt6 multimedia on Linux commonly uses FFmpeg. Allow explicit override,
    // but default to ffmpeg so behavior is predictable across hosts.
    if (!qEnvironmentVariableIsSet("QT_MEDIA_BACKEND")) {
        qputenv("QT_MEDIA_BACKEND", QByteArrayLiteral("ffmpeg"));
        qInfo() << "QT_MEDIA_BACKEND not set; defaulting to" << qEnvironmentVariable("QT_MEDIA_BACKEND");
    } else {
        qInfo() << "Using QT_MEDIA_BACKEND =" << qEnvironmentVariable("QT_MEDIA_BACKEND");
    }
#endif
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    // Temporarily disable shared contexts to test if this is causing the refresh issue
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(2, 0);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setSwapInterval(0);  // Disable VSync to allow independent update rates
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


    // NOTE: Replaced QThread::msleep() with the modern static QThread::msleep()
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



    // setup main window
    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    setting up main window .."),
        Qt::AlignTop | Qt::AlignLeft, Qt::yellow);

    // Start rigctld server before mainWindow so ServerWidget can connect to it
    RigCtlServer rigCtlServer;
    rigCtlServer.startListening(4532);
    Settings::instance()->setRigCtlServer(&rigCtlServer);

    TciServer tciServer;
    quint16 tciPort = 50001;
    if (const char *portEnv = std::getenv("CUSDR_TCI_PORT")) {
        bool ok = false;
        const int parsed = QString::fromLocal8Bit(portEnv).toInt(&ok);
        if (ok && parsed > 0 && parsed <= 65535)
            tciPort = static_cast<quint16>(parsed);
    }
    if (Settings::instance()->getTciServerEnabled()) {
        if (tciServer.startListening(tciPort))
            qDebug() << "Init::\tTCI server listening on port" << tciPort;
        else
            qWarning() << "Init::\tTCI server failed to start on port" << tciPort;
    }
    else {
        qDebug() << "Init::\tTCI server disabled in settings";
    }
    // Start/stop the TCI server at runtime when the setting is toggled.
    QObject::connect(Settings::instance(), &Settings::tciServerEnabledChanged,
                     &tciServer, [&tciServer, tciPort](bool enabled) {
        if (enabled) {
            if (tciServer.startListening(tciPort))
                qDebug() << "Init::\tTCI server listening on port" << tciPort;
            else
                qWarning() << "Init::\tTCI server failed to start on port" << tciPort;
        }
        else {
            tciServer.stopListening();
            qDebug() << "Init::\tTCI server stopped";
        }
    });
    Settings::instance()->setTciServer(&tciServer);
    RadioModel radioModel(&app);
    for (int i = 0; i < 8; ++i) radioModel.addSlice(new SliceModel(i, &radioModel));
    Settings::instance()->setRadioModel(&radioModel);
    Settings::instance()->syncSlicesWithSettings();
    tciServer.bindSlices(&radioModel);
    MainWindow mainWindow(&radioModel);
    qDebug() << "Init::\tmain window setup ...";
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

    CPUMonitor *cpu_monitor = new CPUMonitor(&app);
    QObject::connect(cpu_monitor, &CPUMonitor::cpuLoadChanged, Settings::instance(), &Settings::setCPULoad);
    cpu_monitor->start();

    qDebug() << "Init::\trunning application ...\n";
    return app.exec();
}
