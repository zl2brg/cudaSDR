cmake_minimum_required(VERSION 3.10)
project(cudaSDR)
set(QT_DIR "/home/simon/Qt/5.13.2/")
set(CMAKE_PREFIX_PATH "/home/simon/Qt/5.13.2/gcc_64/lib/cmake")
set(DCMAKE_INSTALL_PREFIX:PATH "/usr/local")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set (QT_USE_QTOPENGL TRUE)
add_definitions(-O4 -D_FILE_OFFSET_BITS=64 -fPIC -g)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "-O4 -Wall -Wextra -g")
set (CMAKE_BUILD_TYPE Debug)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(PkgConfig)
find_package(Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt5Gui 5.0 REQUIRED )
find_package(Qt5Multimedia 5.0 REQUIRED)
find_package(Qt5Network 5.0 REQUIRED)
#find_package(QT5OpenGL)
find_package(OpenGL REQUIRED)

SET(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

SET(AUDIOENGINE_DIR ${CMAKE_SOURCE_DIR}/AudioEngine)
SET(CL_DIR ${CMAKE_SOURCE_DIR}/CL)
SET(DATAENGINE_DIR ${CMAKE_SOURCE_DIR}/DataEngine)
SET(GL_DIR ${CMAKE_SOURCE_DIR}/GL)
SET(QTDSP_DIR ${CMAKE_SOURCE_DIR}/QtDSP)
SET(UTIL_DIR ${CMAKE_SOURCE_DIR}/Utility_source)
SET(QTWDSP_DIR ${CMAKE_SOURCE_DIR}/QtWDSP)
SET(UI_DIR ${CMAKE_SOURCE_DIR}/UI)
SET(WDSP_DIR "/usr/local/include")

set (SOURCES
        ${CMAKE_SOURCE_DIR}/Util/cusdr_buttons.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_colorTriangle.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_highResTimer.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_image.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_imageblur.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_led.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_painter.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_splash.cpp 
	${CMAKE_SOURCE_DIR}/Util/cusdr_cpuUsage.cpp 
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_settingsdialog.cpp
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_utils.cpp
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_waveform.cpp 
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_wavfile.cpp 
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_fspectrum.cpp 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_audioReceiver.cpp 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_dataEngine.cpp
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_dataIO.cpp 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_discoverer.cpp 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_receiver.cpp 
	${CMAKE_SOURCE_DIR}/DataEngine/soundout.cpp 
    ${CMAKE_SOURCE_DIR}/DataEngine/fractresampler.cpp
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_WidebandProcessor.cpp
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglDisplayPanel.cpp
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglDistancePanel.cpp 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglInfo.cpp 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglReceiverPanel.cpp 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglText.cpp 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglWidebandPanel.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_alexAntennaWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_alexFilterWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_alexTabWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_displayWidget.cpp
	${CMAKE_SOURCE_DIR}/cusdr_colorsWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_hamDatabase.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_hpsdrTabWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_hpsdrWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_networkWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_mainWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_extCtrlWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_radioTabWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_radioWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_agcWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_displayTabWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_radioPopupWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_receiverWidget.cpp 
	#${CMAKE_SOURCE_DIR}/cusdr_server.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_serverWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_settings.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_fonts.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_transmitTabWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_transmitOptionsWidget.cpp 
	${CMAKE_SOURCE_DIR}/cusdr_transmitPAWidget.cpp 
	${CMAKE_SOURCE_DIR}/main.cpp 
    ${CMAKE_SOURCE_DIR}/Util/cusdr_cpuUsage_unix.cpp
    ${CMAKE_SOURCE_DIR}/QtWDSP/qtwdsp_dspEngine.cpp
	${CMAKE_SOURCE_DIR}/UI/noisefilterwidget.cpp
    ${CMAKE_SOURCE_DIR}/UI/setupwidget.cpp
#    ${CMAKE_SOURCE_DIR}/UI/radio_widget.cpp
    ${CMAKE_SOURCE_DIR}/UI/tx_settings_dialog.cpp
    ${CMAKE_SOURCE_DIR}/UI/cusdr_setupwidget.cpp
    ${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_iambic.cpp
    ${CMAKE_SOURCE_DIR}/AudioEngine/cwramp.cpp

src/DataEngine/cusdr_transmitter.cpp src/DataEngine/cusdr_transmitter.h src/AudioEngine/cusdr_audio_input.cpp src/AudioEngine/cusdr_audio_input.h)

    
set (HEADERS 
    ${CMAKE_SOURCE_DIR}/Util/cusdr_buttons.h
	${CMAKE_SOURCE_DIR}/Util/cusdr_colorTriangle.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_highResTimer.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_image.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_imageblur.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_led.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_painter.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_queue.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_splash.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_styles.h 
	${CMAKE_SOURCE_DIR}/Util/cusdr_cpuUsage.h 
	${CMAKE_SOURCE_DIR}/Util/qcircularbuffer.h 
        ${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_settingsdialog.h
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_utils.h
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_waveform.h 
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_audio_wavfile.h 
	${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_fspectrum.h 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_audioReceiver.h 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_dataEngine.h
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_dataIO.h 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_discoverer.h 
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_receiver.h 
	${CMAKE_SOURCE_DIR}/DataEngine/soundout.h 
    ${CMAKE_SOURCE_DIR}/DataEngine/fractresampler.h
	${CMAKE_SOURCE_DIR}/DataEngine/datatypes.h
	${CMAKE_SOURCE_DIR}/DataEngine/cusdr_WidebandProcessor.h
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglDisplayPanel.h
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglDistancePanel.h 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglInfo.h 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglReceiverPanel.h 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglText.h 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglUtils.h 
	${CMAKE_SOURCE_DIR}/GL/cusdr_oglWidebandPanel.h 
	${CMAKE_SOURCE_DIR}/cusdr_alexAntennaWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_alexFilterWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_alexTabWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_displayWidget.h
	${CMAKE_SOURCE_DIR}/cusdr_colorsWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_hamDatabase.h 
	${CMAKE_SOURCE_DIR}/cusdr_hpsdrTabWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_hpsdrWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_networkWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_mainWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_extCtrlWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_radioTabWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_radioWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_agcWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_displayTabWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_radioPopupWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_receiverWidget.h 
	#.${CMAKE_SOURCE_DIR}/cusdr_server.h 
	${CMAKE_SOURCE_DIR}/cusdr_serverWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_settings.h 
	${CMAKE_SOURCE_DIR}/cusdr_fonts.h 
	${CMAKE_SOURCE_DIR}/cusdr_transmitTabWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_transmitOptionsWidget.h 
	${CMAKE_SOURCE_DIR}/cusdr_transmitPAWidget.h 
    ${CMAKE_SOURCE_DIR}/Util/cusdr_cpuUsage_unix.h
    ${CMAKE_SOURCE_DIR}/QtWDSP/qtwdsp_dspEngine.h
	#${CMAKE_SOURCE_DIR}cusdr_NoiseFilterWidget.h
	${CMAKE_SOURCE_DIR}/UI/noisefilterwidget.h
	${CMAKE_SOURCE_DIR}/UI/setupwidget.h
    ${CMAKE_SOURCE_DIR}/UI/tx_settings_dialog.h
    ${CMAKE_SOURCE_DIR}/UI/cusdr_setupwidget.h
    ${CMAKE_SOURCE_DIR}/AudioEngine/cusdr_iambic.h
    ${WDSP_DIR}/wdsp.h
src/DataEngine/cusdr_transmitter.cpp src/DataEngine/cusdr_transmitter.h)

set(UI_SOURCES
        ${CMAKE_SOURCE_DIR}/UI/tx_settings_dialog.ui
        ${CMAKE_SOURCE_DIR}/UI/noisefilterwidget.ui
        ${CMAKE_SOURCE_DIR}/UI/cusdr_display.ui
        ${CMAKE_SOURCE_DIR}/UI/setupwidget.ui
    )


add_definitions( ${QT_DEFINITIONS} )
include_directories( ${CMAKE_BINARY_DIR} )
include_directories( ${Qt5Widgets_INCLUDE_DIRS})
include_directories( ${Qt5Network_INCLUDE_DIRS})
include_directories( ${Qt5Multimedia_INCLUDE_DIRS})
include_directories( ${Qt5Gui_INCLUDE_DIRS})
include_directories( ${CMAKE_SOURCE_DIR})
include_directories( ${Qt5Gui_INCLUDE_DIRS})
#include_directories( ${QT5OpenGL_INCLUDE_DIRS})
include_directories( ${OpenGL_INCLUDE_DIR})
include_directories( ${DATAENGINE_DIR})
include_directories( ${UI_DIR})
include_directories( ${AUTOGEN_BUILD_DIR})
include_directories( ${WDSP_DIR})

set ( MOC_HEADERS
 ..h
)

QT5_WRAP_UI( UI_HEADERS ${UI_SOURCES} )
ADD_EXECUTABLE(cudasdr ${SOURCES} ${HEADERS}  res/cusdr.qrc ${UI_SOURCES} ${UI_HEADERS})

qt5_use_modules(cudasdr Core Gui Widgets OpenGL)
target_link_libraries(cudasdr -lm -lstdc++ Qt5::Widgets Qt5::Network Qt5::Multimedia GL asound wdsp portaudio)
set_target_properties(cudasdr PROPERTIES LINKER_LANGUAGE CXX)
