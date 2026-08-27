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
#include <QMutex>
#include <QTextStream>
#include <QPixmap>
#include <QStyle>
#include <QScreen>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QByteArray>
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
        // Called from the GUI, data IO, data processor and DSP threads.
        QMutexLocker locker(&m_mutex);
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
        QMutexLocker locker(&m_mutex);
        if (m_outFile.isOpen()) {
            m_stream.flush();
            m_outFile.close();
        }
    }

    QMutex m_mutex;
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
    // WDSPwisdom() concatenates the filename directly onto the directory
    // string — ensure a trailing separator so plans land in ~/.cudaSDR/
    // (otherwise the file becomes ~/.cudaSDRwdspWisdom01).
    QString wisdomDir = Settings::instance()->cfg_dir;
    if (!wisdomDir.endsWith(QLatin1Char('/')))
        wisdomDir.append(QLatin1Char('/'));

    const QString newWisdom = wisdomDir + QStringLiteral("wdspWisdom01");
    if (!QFile::exists(newWisdom)) {
        // Migrate legacy juxtaposed path from older builds.
        const QString legacy = Settings::instance()->cfg_dir + QStringLiteral("wdspWisdom01");
        if (QFile::exists(legacy) && QFile::copy(legacy, newWisdom))
            qInfo() << "Migrated WDSP wisdom" << legacy << "->" << newWisdom;
    }

    qInfo() << "Loading WDSP FFT wisdom from" << wisdomDir;
    const int rebuilt = WDSPwisdom(wisdomDir.toLocal8Bit().data());
    if (rebuilt)
        qInfo() << "WDSP FFT wisdom rebuilt (first run or missing wdspWisdom01); subsequent starts will be faster";
}

static bool requestingNativeWayland(int argc, char **argv)
{
    const QByteArray env = qgetenv("QT_QPA_PLATFORM");
    if (env.contains("wayland"))
        return true;
    for (int i = 1; i < argc; ++i) {
        const QByteArray arg(argv[i]);
        if (arg == "-platform" || arg == "--platform") {
            if (i + 1 < argc && QByteArray(argv[i + 1]).contains("wayland"))
                return true;
        } else if (arg.startsWith("-platform=") || arg.startsWith("--platform=")) {
            if (arg.contains("wayland"))
                return true;
        }
    }
    return false;
}

static void preferNvidiaEglForNativeWayland()
{
    const QString nvidiaIcd = QStringLiteral("/usr/share/glvnd/egl_vendor.d/10_nvidia.json");
    if (!qEnvironmentVariableIsSet("__EGL_VENDOR_LIBRARY_FILENAMES") && QFile::exists(nvidiaIcd)) {
        // Stop Mesa libEGL from probing the NVIDIA node (dri2 screen fails,
        // driver (null), then a software fallback can silently eat 300% CPU).
        qputenv("__EGL_VENDOR_LIBRARY_FILENAMES", nvidiaIcd.toUtf8());
        qInfo() << "Native Wayland: pinning EGL vendor to NVIDIA" << nvidiaIcd;
    }
    if (!qEnvironmentVariableIsSet("GBM_BACKEND"))
        qputenv("GBM_BACKEND", QByteArrayLiteral("nvidia-drm"));
}

static bool isSoftwareOpenGLRenderer(const QByteArray &renderer)
{
    const QByteArray r = renderer.toLower();
    return r.contains("llvmpipe") || r.contains("softpipe")
        || r.contains("swrast") || r.contains("software rasterizer");
}

int main(int argc, char *argv[]) {

#ifndef DEBUG
    // NOTE: The function name is the same, but it now works with the updated handler signature.
    qInstallMessageHandler(cuSDRMessageHandler);
#endif
#if defined(Q_OS_LINUX)
    // Prefer xcb by default. Native Wayland + NVIDIA frequently fails EGL dri2/vsync
    // and busy-spins (200%+ CPU). Users can still force Wayland via QT_QPA_PLATFORM.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("xcb"));
        if (qEnvironmentVariableIsSet("WAYLAND_DISPLAY"))
            qInfo() << "Wayland session detected; using QT_QPA_PLATFORM=xcb (XWayland)."
                    << "Export QT_QPA_PLATFORM=wayland to force native Wayland.";
    }
    // NVIDIA GLX/EGL often busy-spins waiting for vsync. usleep instead of a spin.
    if (!qEnvironmentVariableIsSet("__GL_YIELD"))
        qputenv("__GL_YIELD", QByteArrayLiteral("USLEEP"));
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
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setAlphaBufferSize(0); // opaque surfaces: Wayland must not blend this window with whatever is underneath
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    // Native Wayland + NVIDIA busy-waits in eglSwapBuffers when vsync is on (~250% CPU).
    // Flicker is handled by opaque surfaces + restoring the QOpenGLWidget FBO after
    // QOpenGLFramebufferObject::release(), so swapInterval(0) is safe on Wayland.
    const bool nativeWayland = requestingNativeWayland(argc, argv);
    if (nativeWayland)
        preferNvidiaEglForNativeWayland();
    format.setSwapInterval(nativeWayland ? 0 : 1);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    qInfo() << "Qt platform:" << app.platformName()
            << "GL swapInterval:" << format.swapInterval();
    if (app.platformName().contains(QLatin1String("wayland"), Qt::CaseInsensitive)
            && format.swapInterval() != 0) {
        qWarning() << "Native Wayland with vsync on: NVIDIA EGL may busy-wait (~250% CPU)."
                   << "GL widgets will request swapInterval(0).";
    }

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

    Settings::instance()->setSettingsLoaded(Settings::instance()->loadPersistentSettings() >= 0);

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
    // check for OpenGL 3.3 Core (required by the panadapter / display path)
    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    Checking for OpenGL 3.3 Core ..."),
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
    const bool hasCore33 =
        surfaceformat.majorVersion() > 3
        || (surfaceformat.majorVersion() == 3 && surfaceformat.minorVersion() >= 3);
    if (!hasCore33) {
        qDebug() << "Init::\tOpenGL found, but appears to be less than OGL 3.3 Core."
                 << "Got" << surfaceformat.majorVersion() << "." << surfaceformat.minorVersion();
        splash->showMessage(
            "\n      " +
                Settings::instance()->getTitleStr() + " " +
                Settings::instance()->getVersionStr() +
                QObject::tr(":    found but appears to be less than OGL 3.3 Core"),
            Qt::AlignTop | Qt::AlignLeft, Qt::yellow);
            app.processEvents();
            QThread::msleep(1000);
        splash->hide();

        QMessageBox::critical(nullptr,
                              QApplication::applicationName(),
                              QApplication::applicationName() + "    requires OpenGL 3.3 Core or later to run.",
                              QMessageBox::Ok);
        return -1;
    }

    qDebug() << "Init::\tOpenGL 3.3+ found.";

    QOffscreenSurface probeSurface;
    probeSurface.setFormat(context.format());
    probeSurface.create();
    QByteArray glRenderer;
    if (probeSurface.isValid() && context.makeCurrent(&probeSurface)) {
        QOpenGLFunctions *gl = context.functions();
        const char *vendor = reinterpret_cast<const char *>(gl->glGetString(GL_VENDOR));
        const char *renderer = reinterpret_cast<const char *>(gl->glGetString(GL_RENDERER));
        const char *version = reinterpret_cast<const char *>(gl->glGetString(GL_VERSION));
        glRenderer = renderer ? QByteArray(renderer) : QByteArray();
        qInfo() << "Init::\tGL vendor:" << (vendor ? vendor : "?")
                << "renderer:" << (renderer ? renderer : "?")
                << "version:" << (version ? version : "?");
        context.doneCurrent();
    } else {
        qWarning() << "Init::\tcould not make probe context current; GL renderer unknown.";
    }
    if (app.platformName().contains(QLatin1String("wayland"), Qt::CaseInsensitive)
            && isSoftwareOpenGLRenderer(glRenderer)) {
        qWarning() << "Init::\tnative Wayland is using a software OpenGL renderer."
                   << "That typically follows Mesa 'egl: failed to create dri2 screen' on NVIDIA"
                   << "and will pin several CPU cores. Use the default XWayland path"
                   << "(unset QT_QPA_PLATFORM) or install NVIDIA EGL/Wayland (libnvidia-egl-wayland).";
    }
    splash->showMessage(
        "\n      " +
            Settings::instance()->getTitleStr() + " " +
            Settings::instance()->getVersionStr() +
            QObject::tr(":    OpenGL 3.3 Core found."),
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



    // FFTW PATIENT plans inside OpenChannel (TX member of DataEngine, RX
    // channels, etc.) are only cheap once wisdom is imported. MainWindow::setup
    // constructs DataEngine → Transmitter → OpenChannel, so load wisdom first.
    load_WDSPWisdom();

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
    Settings::instance()->syncTransmitWithSettings();
    tciServer.bindSlices(&radioModel);
    MainWindow mainWindow(&radioModel, Settings::instance());
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

    mainWindow.show();
    mainWindow.update();
    mainWindow.setFocus();

    CPUMonitor *cpu_monitor = new CPUMonitor(&app);
    QObject::connect(cpu_monitor, &CPUMonitor::cpuLoadChanged, Settings::instance(), &Settings::setCPULoad);
    cpu_monitor->start();

    qDebug() << "Init::\trunning application ...\n";
    return app.exec();
}
