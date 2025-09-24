# Qt6 Migration - Detailed Changes for cudaSDR

## 1. CMakeLists.txt Changes (CRITICAL - Do this first)

### File: `Source/CMakeLists.txt`

**Replace lines 3-4:**
```cmake
# OLD:
set(QT_DIR "/home/simon/Qt/5.15.2/")
set(CMAKE_PREFIX_PATH "/home/simon/Qt/5.15.2/gcc_64/lib/cmake")

# NEW:
set(QT_DIR "/home/simon/Qt/6.8.0/")
set(CMAKE_PREFIX_PATH "/home/simon/Qt/6.8.0/gcc_64/lib/cmake")
```

**Replace lines 18-22:**
```cmake
# OLD:
find_package(Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt5Gui 5.0 REQUIRED )
find_package(Qt5Multimedia 5.0 REQUIRED)
find_package(Qt5Network 5.0 REQUIRED)
#find_package(QT5OpenGL)

# NEW:
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Multimedia Network OpenGL OpenGLWidgets)
```

**Replace lines 183-189:**
```cmake
# OLD:
include_directories( ${Qt5Widgets_INCLUDE_DIRS})
include_directories( ${Qt5Network_INCLUDE_DIRS})
include_directories( ${Qt5Multimedia_INCLUDE_DIRS})
include_directories( ${Qt5Gui_INCLUDE_DIRS})
include_directories( ${CMAKE_SOURCE_DIR})
include_directories( ${Qt5Gui_INCLUDE_DIRS})
#include_directories( ${QT5OpenGL_INCLUDE_DIRS})

# NEW:
include_directories( ${Qt6Widgets_INCLUDE_DIRS})
include_directories( ${Qt6Network_INCLUDE_DIRS})
include_directories( ${Qt6Multimedia_INCLUDE_DIRS})
include_directories( ${Qt6Gui_INCLUDE_DIRS})
include_directories( ${CMAKE_SOURCE_DIR})
include_directories( ${Qt6OpenGL_INCLUDE_DIRS})
include_directories( ${Qt6OpenGLWidgets_INCLUDE_DIRS})
```

**Replace lines 200-204:**
```cmake
# OLD:
QT5_WRAP_UI( UI_HEADERS ${UI_SOURCES} )
ADD_EXECUTABLE(cudasdr ${SOURCES} ${HEADERS}  res/cusdr.qrc ${UI_SOURCES} ${UI_HEADERS})

qt5_use_modules(cudasdr Core Gui Widgets OpenGL)
target_link_libraries(cudasdr -lm -lstdc++ Qt5::Widgets Qt5::Network Qt5::Multimedia GL asound wdsp portaudio)

# NEW:
qt6_wrap_ui( UI_HEADERS ${UI_SOURCES} )
ADD_EXECUTABLE(cudasdr ${SOURCES} ${HEADERS}  res/cusdr.qrc ${UI_SOURCES} ${UI_HEADERS})

target_link_libraries(cudasdr -lm -lstdc++ Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::OpenGL Qt6::OpenGLWidgets GL asound wdsp portaudio)
```

---

## 2. OpenGL Code Changes (HIGH IMPACT)

### File: `Source/src/main.cpp`

**Replace includes (lines around 51):**
```cpp
// OLD:
//#include <QtOpenGL/QGLFramebufferObject>

// NEW:
#include <QtOpenGL/QOpenGLFramebufferObject>
```

**Replace OpenGL checks (lines 209-229):**
```cpp
// OLD:
if (!QGLFormat::hasOpenGL() && QGLFormat::OpenGL_Version_2_0) {
    // ... error handling
}

if (!(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0)) {
    // ... error handling  
}

// NEW:
QOpenGLContext context;
if (!context.create()) {
    qDebug() << "Init::\tOpenGL context creation failed!";
    // ... same error handling
    return -1;
}

QSurfaceFormat format = context.format();
if (format.majorVersion() < 2) {
    qDebug() << "Init::\tOpenGL found, but appears to be less than OGL v2.0.";
    // ... same error handling
    return -1;
}
```

**Replace framebuffer check (line 261):**
```cpp
// OLD:
if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
    // ... error handling
}

// NEW:
if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects()) {
    // ... same error handling  
}
```

### File: `Source/src/GL/cusdr_oglUtils.h`

**Replace include (line 35):**
```cpp
// OLD:
#include <QGLFramebufferObject>

// NEW:
#include <QtOpenGL/QOpenGLFramebufferObject>
```

### Files: OpenGL Panel Headers
**In all GL/*.h files, replace:**
```cpp
// OLD:
//#include <QtOpenGL/QOpenGLWidget>
//#include <QGLFramebufferObject>

// NEW:
#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtOpenGL/QOpenGLFramebufferObject>
```

**Replace variable types in headers:**
```cpp
// OLD:
QGLFramebufferObject* m_frequencyScaleFBO;
QGLFramebufferObject* m_dBmScaleFBO;
QGLFramebufferObject* m_gridFBO;

// NEW:
QOpenGLFramebufferObject* m_frequencyScaleFBO;
QOpenGLFramebufferObject* m_dBmScaleFBO;
QOpenGLFramebufferObject* m_gridFBO;
```

### Files: OpenGL Panel CPP files
**Replace includes:**
```cpp
// OLD:
//#include <QGLFramebufferObject>

// NEW:
#include <QtOpenGL/QOpenGLFramebufferObject>
```

**Replace object creation:**
```cpp
// OLD:
m_dBmScaleFBO = new QGLFramebufferObject(width, height);
m_frequencyScaleFBO = new QGLFramebufferObject(width, height);
m_gridFBO = new QGLFramebufferObject(width, height);

// NEW:
m_dBmScaleFBO = new QOpenGLFramebufferObject(width, height);
m_frequencyScaleFBO = new QOpenGLFramebufferObject(width, height);
m_gridFBO = new QOpenGLFramebufferObject(width, height);
```

---

## 3. Audio System Changes (HIGH IMPACT)

### File: `Source/src/DataEngine/soundout.h`

**Replace member variables (lines 48-49):**
```cpp
// OLD:
QList<QAudioDeviceInfo> m_OutDevices;
QAudioDeviceInfo  m_OutDeviceInfo;

// NEW:
QList<QAudioDevice> m_OutDevices;
QAudioDevice  m_OutDeviceInfo;
```

### File: `Source/src/DataEngine/soundout.cpp`

**Replace includes (add at top):**
```cpp
// NEW - ADD:
#include <QMediaDevices>
```

**Replace device enumeration (lines 139-157):**
```cpp
// OLD:
QAudioDeviceInfo  DeviceInfo;
// ...
m_OutDevices = DeviceInfo.availableDevices(QAudio::AudioOutput);
// ...
if (-1 == OutDevIndx) m_OutDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
else m_OutDeviceInfo = m_OutDevices.at(OutDevIndx);

foreach (const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
     qDebug() << "l:" << deviceInfo.deviceName();
}

QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
qDebug() << "res:" << info.deviceName();

// NEW:
m_OutDevices = QMediaDevices::audioOutputs();

if (-1 == OutDevIndx) m_OutDeviceInfo = QMediaDevices::defaultAudioOutput();
else m_OutDeviceInfo = m_OutDevices.at(OutDevIndx);

foreach (const QAudioDevice &deviceInfo, QMediaDevices::audioOutputs()) {
     qDebug() << "l:" << deviceInfo.description();
}

QAudioDevice info = QMediaDevices::defaultAudioOutput();
qDebug() << "res:" << info.description();
```

**Replace audio format setup (lines 162-170):**
```cpp
// OLD:
m_OutAudioFormat.setCodec("audio/pcm");
m_OutAudioFormat.setSampleRate(SOUNDCARD_RATE);
m_OutAudioFormat.setSampleSize(16);
m_OutAudioFormat.setSampleType(QAudioFormat::SignedInt);
m_OutAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
if(m_StereoOut)
    m_OutAudioFormat.setChannelCount(2);

// NEW:
m_OutAudioFormat.setSampleRate(SOUNDCARD_RATE);
m_OutAudioFormat.setSampleFormat(QAudioFormat::Int16);
if(m_StereoOut)
    m_OutAudioFormat.setChannelCount(2);
else
    m_OutAudioFormat.setChannelCount(1);
```

### File: `Source/src/AudioEngine/cusdr_audio_settingsdialog.h`

**Replace includes (line ~26):**
```cpp
// OLD:
#include <QAudioDeviceInfo>

// NEW:
#include <QAudioDevice>
#include <QMediaDevices>
```

**Replace constructor and member variables (lines 66-76):**
```cpp
// OLD:
SettingsDialog(const QList<QAudioDeviceInfo> &availableInputDevices,
               const QList<QAudioDeviceInfo> &availableOutputDevices,
               QWidget *parent = 0);

const QAudioDeviceInfo& inputDevice() const     { return m_inputDevice; }
const QAudioDeviceInfo& outputDevice() const    { return m_outputDevice; }

QAudioDeviceInfo m_inputDevice;
QAudioDeviceInfo m_outputDevice;

// NEW:
SettingsDialog(const QList<QAudioDevice> &availableInputDevices,
               const QList<QAudioDevice> &availableOutputDevices,
               QWidget *parent = 0);

const QAudioDevice& inputDevice() const     { return m_inputDevice; }
const QAudioDevice& outputDevice() const    { return m_outputDevice; }

QAudioDevice m_inputDevice;
QAudioDevice m_outputDevice;
```

### File: `Source/src/AudioEngine/cusdr_audio_settingsdialog.cpp`

**Replace all QAudioDeviceInfo → QAudioDevice throughout the file**
**Replace device.deviceName() → device.description() throughout the file**

---

## 4. QDesktopWidget Replacement (MEDIUM IMPACT)

### File: `Source/src/cusdr_radioPopupWidget.cpp`

**Add include (top of file):**
```cpp
// NEW - ADD:
#include <QScreen>
#include <QGuiApplication>
```

**Replace QDesktopWidget usage (lines 2539-2542):**
```cpp
// OLD:
QDesktopWidget *desktop = QApplication::desktop();
QRect desktopRect = desktop->availableGeometry(desktop->screenNumber());

// NEW:
QScreen *screen = QGuiApplication::primaryScreen();
QRect desktopRect = screen->availableGeometry();
```

**Same replacement around line 2651:**
```cpp
// OLD:
QDesktopWidget *desktop = QApplication::desktop();
// Replace with same QScreen code as above

// NEW:
QScreen *screen = QGuiApplication::primaryScreen();
```

---

## 5. QFontMetrics width() Method (MEDIUM IMPACT)

**Search and replace throughout all .cpp files:**
```cpp
// OLD:
fontMetrics.width(text)
QFontMetrics::width(text)

// NEW:
fontMetrics.horizontalAdvance(text)
QFontMetrics::horizontalAdvance(text)
```

---

## 6. Text Stream endl/flush (LOW IMPACT)

**Search and replace in files using QTextStream:**
```cpp
// OLD:
ts << txt << endl << flush;

// NEW:
ts << txt << Qt::endl << Qt::flush;
```

---

## 7. Header Include Updates

### Throughout project, replace includes:
```cpp
// OLD:
#include <QtOpenGL/QOpenGLWidget>

// NEW:
#include <QtOpenGLWidgets/QOpenGLWidget>
```

```cpp
// OLD:
#include <QGLFramebufferObject>

// NEW:
#include <QtOpenGL/QOpenGLFramebufferObject>
```

---

## 8. Additional Qt6 Compatibility

### Remove deprecated qt5_use_modules
Already covered in CMakeLists.txt changes above.

### Update moc/uic tools usage
CMake should handle this automatically with Qt6.

---

## Migration Order (IMPORTANT)

1. **CMakeLists.txt** (Must be first)
2. **OpenGL code** (High impact - may break compilation)
3. **Audio system** (High impact - may break functionality)
4. **QDesktopWidget** (Medium impact)
5. **QFontMetrics** (Medium impact)
6. **Text stream** (Low impact)
7. **Test build and functionality**

## Estimated Time
- **Experienced Qt developer**: 1-2 weeks
- **Learning curve**: Add 1 additional week

## Risk Assessment
- **High Risk**: Audio system changes (complex API differences)
- **Medium Risk**: OpenGL changes (well documented migration path)
- **Low Risk**: Widget/UI changes (mostly straightforward replacements)

The project structure is modern (CMake-based) which makes the migration easier than older qmake projects.
