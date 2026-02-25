cmake_minimum_required(VERSION 3.16)
project(cudaSDR LANGUAGES CXX)

# --- Basic Setup ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PREFIX_PATH "~/Qt/6.9.3/gcc_64")

# --- Debug Configuration ---
# Enable debug symbols and disable optimizations for Debug builds
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3 -O0 -DDEBUG")
    message(STATUS "Debug build with symbols enabled")
endif()

# Optional: Enable AddressSanitizer (uncomment to use)
set(SANITIZE_FLAGS "-fsanitize=address")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZE_FLAGS}")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} ${SANITIZE_FLAGS}")

# Optional: Enable profiling with gprof (uncomment to use)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg")

# --- Find Dependencies ---
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Multimedia Network OpenGL OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)

# --- Project Configuration ---
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(WDSP_DIR "/usr/local/include")

# --- Define Source Files ---
set(SOURCES
    # Main application
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/cusdr_settings.cpp
    ${SRC_DIR}/cusdr_hamDatabase.cpp
    ${SRC_DIR}/cusdr_fonts.cpp
    # Audio Engine
    ${SRC_DIR}/AudioEngine/cusdr_audio_settingsdialog.cpp
    ${SRC_DIR}/AudioEngine/cwramp.cpp
    ${SRC_DIR}/AudioEngine/cusdr_iambic.cpp
    ${SRC_DIR}/AudioEngine/cusdr_audio_input.cpp
    ${SRC_DIR}/AudioEngine/audiooutputmanager.cpp

#    ${SRC_DIR}/AudioEngine/cusdr_audio_utils.cpp
#    ${SRC_DIR}/AudioEngine/cusdr_audio_waveform.cpp
#    ${SRC_DIR}/AudioEngine/cusdr_audio_wavfile.cpp
#    ${SRC_DIR}/AudioEngine/cusdr_fspectrum.cpp
# CL (OpenCL?)
 #   ${SRC_DIR}/CL/cusdr_clWdspAudioProcessor.cpp
 #   ${SRC_DIR}/CL/cusdr_clWdspFftProcessor.cpp
 #   ${SRC_DIR}/CL/cusdr_clWdspReceiver.cpp
    
    # Data Engine
    ${SRC_DIR}/DataEngine/cusdr_audioReceiver.cpp
    ${SRC_DIR}/DataEngine/cusdr_dataEngine.cpp
    ${SRC_DIR}/DataEngine/cusdr_dataIO.cpp
    ${SRC_DIR}/DataEngine/cusdr_discoverer.cpp
    ${SRC_DIR}/DataEngine/cusdr_receiver.cpp
    ${SRC_DIR}/DataEngine/receiveraudiooutput.cpp
    ${SRC_DIR}/DataEngine/cusdr_transmitter.cpp
    ${SRC_DIR}/DataEngine/soundout.cpp
    ${SRC_DIR}/DataEngine/fractresampler.cpp
    ${SRC_DIR}/DataEngine/cusdr_WidebandProcessor.cpp
    ${SRC_DIR}/DataEngine/Protocol1FrameDecoder.cpp
    ${SRC_DIR}/DataEngine/Protocol2Handler.cpp
    ${SRC_DIR}/DataEngine/SoapySDRHandler.cpp


    # GL (OpenGL)
    ${SRC_DIR}/GL/cusdr_oglDisplayPanel.cpp
    ${SRC_DIR}/GL/cusdr_oglDistancePanel.cpp
    ${SRC_DIR}/GL/cusdr_oglInfo.cpp
    ${SRC_DIR}/GL/cusdr_oglReceiverPanel.cpp
    ${SRC_DIR}/GL/cusdr_oglText.cpp
    ${SRC_DIR}/GL/cusdr_oglWidebandPanel.cpp
    ${SRC_DIR}/GL/cusdr_ogl3DPanel.cpp
    ${SRC_DIR}/GL/cusdr_meshGeneratorWorker.cpp

    # QtWDSP
    ${SRC_DIR}/QtWDSP/qtwdsp_dspEngine.cpp
    ${WDSP_DIR}/wdsp.h

    # Util
    ${SRC_DIR}/Util/cusdr_buttons.cpp
    ${SRC_DIR}/Util/cusdr_colorTriangle.cpp
    ${SRC_DIR}/Util/cusdr_cpuUsage.cpp
    ${SRC_DIR}/Util/cusdr_cpuUsage_unix.cpp
    ${SRC_DIR}/Util/cusdr_image.cpp
    ${SRC_DIR}/Util/cusdr_imageblur.cpp
    ${SRC_DIR}/Util/cusdr_led.cpp
    ${SRC_DIR}/Util/cusdr_splash.cpp
    ${SRC_DIR}/Util/cusdr_highResTimer.cpp
    ${SRC_DIR}/Util/cusdr_painter.cpp

    # Main Widget Classes
    ${SRC_DIR}/cusdr_alexAntennaWidget.cpp
    ${SRC_DIR}/cusdr_alexFilterWidget.cpp
    ${SRC_DIR}/cusdr_alexTabWidget.cpp
    ${SRC_DIR}/cusdr_displayWidget.cpp
    ${SRC_DIR}/cusdr_colorsWidget.cpp
    ${SRC_DIR}/cusdr_3DOptionsWidget.cpp
    ${SRC_DIR}/cusdr_hpsdrTabWidget.cpp
    ${SRC_DIR}/cusdr_hpsdrWidget.cpp
    ${SRC_DIR}/cusdr_networkWidget.cpp
    ${SRC_DIR}/cusdr_mainWidget.cpp
    ${SRC_DIR}/cusdr_extCtrlWidget.cpp
    ${SRC_DIR}/cusdr_radioTabWidget.cpp
    ${SRC_DIR}/cusdr_radioWidget.cpp
    ${SRC_DIR}/cusdr_agcWidget.cpp
    ${SRC_DIR}/cusdr_displayTabWidget.cpp
    ${SRC_DIR}/cusdr_radioPopupWidget.cpp
    ${SRC_DIR}/cusdr_receiverWidget.cpp
   #${SRC_DIR}/cusdr_server.cpp
    ${SRC_DIR}/cusdr_serverWidget.cpp
    ${SRC_DIR}/cusdr_transmitTabWidget.cpp
    ${SRC_DIR}/cusdr_transmitOptionsWidget.cpp
    ${SRC_DIR}/cusdr_transmitPAWidget.cpp
    ${SRC_DIR}/UI/noisefilterwidget.cpp
    ${SRC_DIR}/UI/tx_settings_dialog.cpp
    ${SRC_DIR}/UI/cusdr_setupwidget.cpp

  )

# --- Define Header Files ---
set(HEADERS
    # Audio Engine
    ${SRC_DIR}/AudioEngine/cusdr_audio_settingsdialog.h
    ${SRC_DIR}/AudioEngine/cusdr_iambic.h
    ${SRC_DIR}/AudioEngine/cusdr_audio_input.h
    ${SRC_DIR}/AudioEngine/audiooutputmanager.h

    # Data Engine
    ${SRC_DIR}/DataEngine/cusdr_dataIO.h
    ${SRC_DIR}/DataEngine/cusdr_receiver.h
    ${SRC_DIR}/DataEngine/cusdr_transmitter.h
    ${SRC_DIR}/DataEngine/soundout.h
    ${SRC_DIR}/DataEngine/IHardwareIO.h
    ${SRC_DIR}/DataEngine/IDeviceDiscoverer.h
    ${SRC_DIR}/DataEngine/IFrameDecoder.h
    ${SRC_DIR}/DataEngine/Protocol1FrameDecoder.h
    ${SRC_DIR}/DataEngine/Protocol2Handler.h
    ${SRC_DIR}/DataEngine/SoapySDRHandler.h

    #Widgets
    ${SRC_DIR}/cusdr_hpsdrTabWidget.h
    ${SRC_DIR}/cusdr_hpsdrWidget.h
    ${SRC_DIR}/cusdr_networkWidget.h
    ${SRC_DIR}/UI/noisefilterwidget.h
    ${SRC_DIR}/UI/tx_settings_dialog.h
    ${SRC_DIR}/UI/cusdr_setupwidget.h


    # GL
    ${SRC_DIR}/GL/cusdr_oglDisplayPanel.h
    ${SRC_DIR}/GL/cusdr_oglDistancePanel.h
    ${SRC_DIR}/GL/cusdr_oglInfo.h
    ${SRC_DIR}/GL/cusdr_oglReceiverPanel.h
    ${SRC_DIR}/GL/cusdr_oglText.h
    ${SRC_DIR}/GL/cusdr_oglUtils.h
    ${SRC_DIR}/GL/cusdr_ogl3DPanel.h
    ${SRC_DIR}/GL/cusdr_meshGeneratorWorker.h
    

    # Util
    ${SRC_DIR}/Util/cusdr_buttons.h
    ${SRC_DIR}/Util/cusdr_colorTriangle.h
    ${SRC_DIR}/Util/cusdr_cpuUsage.h
    ${SRC_DIR}/Util/cusdr_cpuUsage_unix.h
    ${SRC_DIR}/Util/cusdr_splash.h
    ${SRC_DIR}/Util/cusdr_highResTimer.h
    ${SRC_DIR}/Util/cusdr_led.h
    ${SRC_DIR}/Util/cusdr_image.h
    ${SRC_DIR}/Util/cusdr_imageblur.h
    ${SRC_DIR}/Util/cusdr_painter.h

    # Main Headers
    ${SRC_DIR}/cusdr_settings.h
    ${SRC_DIR}/cusdr_displayTabWidget.h
    ${SRC_DIR}/cusdr_displayWidget.h
    ${SRC_DIR}/cusdr_3DOptionsWidget.h
    ${SRC_DIR}/cusdr_mainWidget.h
    ${SRC_DIR}/cusdr_radioPopupWidget.h
    ${SRC_DIR}/cusdr_radioTabWidget.h
    ${SRC_DIR}/cusdr_radioWidget.h
    
    # Shared headers
    ${SRC_DIR}/Util/cusdr_queue.h
)

# --- Define UI Files ---
set(UI_SOURCES
    ${SRC_DIR}/UI/tx_settings_dialog.ui
    ${SRC_DIR}/UI/noisefilterwidget.ui
    ${SRC_DIR}/UI/cusdr_display.ui
    ${SRC_DIR}/UI/setupwidget.ui
)

# --- Create Executable ---
qt6_wrap_ui(UI_HEADERS ${UI_SOURCES})
qt_add_executable(cudasdr 
    ${SOURCES} 
    ${HEADERS}  
    res/cusdr.qrc 
    ${UI_SOURCES} 
    ${UI_HEADERS}

)

# --- Configure Include Directories ---
target_include_directories(cudasdr PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SRC_DIR}
    ${SRC_DIR}/AudioEngine
    ${SRC_DIR}/CL
    ${SRC_DIR}/DataEngine
    ${SRC_DIR}/GL
    ${SRC_DIR}/QtDSP
    ${SRC_DIR}/QtWDSP
    ${SRC_DIR}/Util
    ${SRC_DIR}/UI
    "/usr/local/include" # WDSP_DIR
)

# --- Link Libraries ---
target_link_libraries(cudasdr PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Network
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    # System libraries
    -lm
    -lstdc++
    GL
    asound
    wdsp

)
target_link_libraries(cudasdr PRIVATE Qt6::Core)

# --- Compiler Options ---
target_compile_features(cudasdr PRIVATE cxx_std_17)
target_compile_options(cudasdr PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O3>
    -Wall
    -Wextra
    -fPIC
)

target_compile_definitions(cudasdr PRIVATE 
    -D_FILE_OFFSET_BITS=64
    QT_CORE_LIB
    QT_GUI_LIB
    QT_WIDGETS_LIB
    QT_NETWORK_LIB
    QT_MULTIMEDIA_LIB
    QT_OPENGL_LIB
    QT_OPENGLWIDGETS_LIB
)

# --- Installation ---
install(TARGETS cudasdr
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
