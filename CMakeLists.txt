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
find_package(Qt6 6.11.0 EXACT REQUIRED COMPONENTS Widgets Core Gui GuiPrivate ShaderTools Multimedia Network OpenGL OpenGLWidgets Test)
message(STATUS "Using Qt ${Qt6_VERSION} from ${Qt6_DIR}")
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)

# Set PKG_CONFIG_PATH to find bundled libraries in wdsp-libs
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

# --- Find liquid-dsp (local) ---
find_path(LIQUID_INCLUDE_DIR NAMES liquid/liquid.h PATHS ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs/include NO_DEFAULT_PATH)
find_library(LIQUID_LIBRARY NAMES liquid PATHS ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs/lib NO_DEFAULT_PATH)

if(LIQUID_INCLUDE_DIR AND LIQUID_LIBRARY)
    message(STATUS "liquid-dsp found: lib=${LIQUID_LIBRARY}, include=${LIQUID_INCLUDE_DIR}")
    add_definitions(-DHAVE_LIQUID)
else()
    message(FATAL_ERROR "liquid-dsp not found in wdsp-libs! Please build and install it first.")
endif()

pkg_check_modules(FFTW3 REQUIRED fftw3)
pkg_check_modules(FFTW3F REQUIRED fftw3f)
pkg_check_modules(RNNOISE REQUIRED rnnoise IMPORTED_TARGET)
pkg_check_modules(SPECLEACH REQUIRED libspecbleach IMPORTED_TARGET)
pkg_check_modules(SOAPYSDR SoapySDR IMPORTED_TARGET)

if(SOAPYSDR_FOUND)
    message(STATUS "SoapySDR found - generic hardware support enabled")
    add_definitions(-DHAVE_SOAPYSDR)
else()
    message(WARNING "SoapySDR not found - generic hardware support will be disabled")
endif()

# --- Find Codec2 ---
find_library(CODEC2_LIBRARY codec2)
find_path(CODEC2_INCLUDE_DIR codec2/codec2.h)

if(CODEC2_LIBRARY AND CODEC2_INCLUDE_DIR)
    message(STATUS "Codec2 found: lib=${CODEC2_LIBRARY}, include=${CODEC2_INCLUDE_DIR}")
    set(CODEC2_FOUND TRUE)
else()
    message(WARNING "Codec2 not found - FreeDV integration will be disabled")
    set(CODEC2_FOUND FALSE)
endif()

# --- Project Configuration ---
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(WDSP_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-1.29)
set(WDSP_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs)

# --- Add Subdirectories ---
add_subdirectory(wdsp-1.29)

# --- Define Source Files ---
set(SOURCES
    # Main application
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/Models/SliceModel.cpp
    ${SRC_DIR}/Models/RadioModel.cpp
    ${SRC_DIR}/Models/RadioTelemetry.cpp
    ${SRC_DIR}/Controllers/RadioController.cpp
    ${SRC_DIR}/cusdr_settings.cpp
    ${SRC_DIR}/cusdr_hamDatabase.cpp
    ${SRC_DIR}/cusdr_fonts.cpp
    # Audio Engine
    ${SRC_DIR}/AudioEngine/cusdr_audio_settingsdialog.cpp
    ${SRC_DIR}/AudioEngine/cwramp.cpp
    ${SRC_DIR}/AudioEngine/cusdr_iambic.cpp
    ${SRC_DIR}/AudioEngine/cusdr_audio_input.cpp
    ${SRC_DIR}/AudioEngine/audiooutputmanager.cpp
    ${SRC_DIR}/AudioEngine/cusdr_codec2processor.cpp
    ${SRC_DIR}/AudioEngine/cusdr_freedvprocessor.cpp

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
    ${SRC_DIR}/DataEngine/CProtocol1.cpp
    ${SRC_DIR}/DataEngine/CProtocol2.cpp
    ${SRC_DIR}/DataEngine/cusdr_sliceProcessor.cpp
    ${SRC_DIR}/DataEngine/receiveraudiooutput.cpp
    ${SRC_DIR}/DataEngine/cusdr_transmitter.cpp
    ${SRC_DIR}/DataEngine/soundout.cpp
    ${SRC_DIR}/DataEngine/fractresampler.cpp
    ${SRC_DIR}/DataEngine/cusdr_WidebandProcessor.cpp


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
    ${SRC_DIR}/Util/cusdr_image.cpp
    ${SRC_DIR}/Util/cusdr_imageblur.cpp
    ${SRC_DIR}/Util/cusdr_led.cpp
    ${SRC_DIR}/Util/cusdr_splash.cpp
    ${SRC_DIR}/Util/cusdr_highResTimer.cpp
    ${SRC_DIR}/Util/cusdr_painter.cpp
    ${SRC_DIR}/Util/cusdr_rigctlserver.cpp
    ${SRC_DIR}/Util/AudioDeviceService.cpp

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
    ${SRC_DIR}/UI/FrequencyEntryDialog.cpp
    ${SRC_DIR}/UI/DeviceSelectionDialog.cpp
    ${SRC_DIR}/Settings/NetworkConfig.cpp
    ${SRC_DIR}/Settings/DisplayConfig.cpp
    ${SRC_DIR}/Settings/HardwareConfig.cpp
    ${SRC_DIR}/Settings/AudioConfig.cpp
    ${SRC_DIR}/Settings/CWConfig.cpp
    ${SRC_DIR}/Settings/ReceiverConfig.cpp
    ${SRC_DIR}/UI/MainWindow/MainWindowUI.cpp
    ${SRC_DIR}/GL/WaterfallRenderer.cpp
    ${SRC_DIR}/GL/PanadapterRenderer.cpp
    ${SRC_DIR}/GL/OverlayRenderer.cpp
    ${SRC_DIR}/Util/CPUMonitor.cpp

  )

# --- Define Header Files ---
set(HEADERS
    # Audio Engine
    ${SRC_DIR}/AudioEngine/cusdr_audio_settingsdialog.h
    ${SRC_DIR}/AudioEngine/cusdr_iambic.h
    ${SRC_DIR}/AudioEngine/cusdr_audio_input.h
    ${SRC_DIR}/AudioEngine/audiooutputmanager.h
    ${SRC_DIR}/AudioEngine/cusdr_codec2processor.h
    ${SRC_DIR}/AudioEngine/cusdr_freedvprocessor.h

    # Data Engine
    ${SRC_DIR}/DataEngine/cusdr_dataIO.h
    ${SRC_DIR}/DataEngine/cusdr_sliceProcessor.h
    ${SRC_DIR}/DataEngine/cusdr_transmitter.h
    ${SRC_DIR}/DataEngine/soundout.h

    #Widgets
    ${SRC_DIR}/cusdr_hpsdrTabWidget.h
    ${SRC_DIR}/cusdr_hpsdrWidget.h
    ${SRC_DIR}/cusdr_networkWidget.h
    ${SRC_DIR}/UI/noisefilterwidget.h
    ${SRC_DIR}/UI/tx_settings_dialog.h
    ${SRC_DIR}/UI/cusdr_setupwidget.h
    ${SRC_DIR}/UI/FrequencyEntryDialog.h
    ${SRC_DIR}/UI/DeviceSelectionDialog.h


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
    ${SRC_DIR}/Util/cusdr_image.h

    ${SRC_DIR}/Util/cusdr_highResTimer.h
    ${SRC_DIR}/Util/cusdr_led.h
    ${SRC_DIR}/Util/cusdr_image.h
    ${SRC_DIR}/Util/cusdr_imageblur.h
    ${SRC_DIR}/Util/cusdr_painter.h
    ${SRC_DIR}/Util/cusdr_rigctlserver.h
    ${SRC_DIR}/Util/AudioDeviceService.h

    # Main Headers
    ${SRC_DIR}/cusdr_settings.h
    ${SRC_DIR}/Settings/NetworkConfig.h
    ${SRC_DIR}/Settings/DisplayConfig.h
    ${SRC_DIR}/Settings/HardwareConfig.h
    ${SRC_DIR}/Settings/AudioConfig.h
    ${SRC_DIR}/Settings/CWConfig.h
    ${SRC_DIR}/Settings/ReceiverConfig.h
    ${SRC_DIR}/UI/MainWindow/MainWindowUI.h
    ${SRC_DIR}/GL/WaterfallRenderer.h
    ${SRC_DIR}/GL/PanadapterRenderer.h
    ${SRC_DIR}/GL/OverlayRenderer.h
    ${SRC_DIR}/Util/CPUMonitor.h
    ${SRC_DIR}/cusdr_displayTabWidget.h
    ${SRC_DIR}/cusdr_displayWidget.h
    ${SRC_DIR}/cusdr_3DOptionsWidget.h
    ${SRC_DIR}/cusdr_mainWidget.h
    ${SRC_DIR}/cusdr_radioPopupWidget.h
    ${SRC_DIR}/cusdr_radioWidget.h
    
    # Shared headers
    ${SRC_DIR}/Util/cusdr_queue.h
)

if(SOAPYSDR_FOUND)
    list(APPEND SOURCES ${SRC_DIR}/DataEngine/SoapySDRDataSource.cpp)
    list(APPEND SOURCES ${SRC_DIR}/UI/cusdr_radiosettingswidget.cpp)
    list(APPEND HEADERS ${SRC_DIR}/UI/cusdr_radiosettingswidget.h)
endif()

# --- Define UI Files ---
set(UI_SOURCES
    ${SRC_DIR}/UI/tx_settings_dialog.ui
    ${SRC_DIR}/UI/noisefilterwidget.ui
    ${SRC_DIR}/UI/cusdr_display.ui
    ${SRC_DIR}/UI/setupwidget.ui
)
if(SOAPYSDR_FOUND)
    list(APPEND UI_SOURCES ${SRC_DIR}/UI/cusdr_radiosettingswidget.ui)
endif()

# --- Create Executable ---
qt6_wrap_ui(UI_HEADERS ${UI_SOURCES})
qt_add_executable(cudasdr 
    ${SOURCES} 
    ${HEADERS}  
    ${CMAKE_CURRENT_SOURCE_DIR}/res/cusdr.qrc 
    ${UI_SOURCES} 
    ${UI_HEADERS}
)

qt6_add_shaders(cudasdr "panadapter_shaders"
    PREFIX "/shaders"
    FILES
        src/GL/shaders/panadapter.vert
        src/GL/shaders/panadapter.frag
    OUTPUTS
        panadapter.vert.qsb
        panadapter.frag.qsb
)

# --- Configure Include Directories ---
target_include_directories(cudasdr PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SRC_DIR}
    ${SRC_DIR}/AudioEngine
    ${SRC_DIR}/DataEngine
    ${SRC_DIR}/GL
    ${SRC_DIR}/QtDSP
    ${SRC_DIR}/Models
    ${SRC_DIR}/QtWDSP
    ${SRC_DIR}/Util
    ${SRC_DIR}/UI
    ${WDSP_DIR}
    ${LIQUID_INCLUDE_DIR}
    ${RNNOISE_INCLUDE_DIRS}
    ${SPECLEACH_INCLUDE_DIRS}
    ${FFTW3_INCLUDE_DIRS}
    ${FFTW3F_INCLUDE_DIRS}
)

if(CODEC2_FOUND)
    target_include_directories(cudasdr PRIVATE ${CODEC2_INCLUDE_DIR})
endif()

# --- Link Libraries ---
target_link_libraries(cudasdr PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Multimedia
    Qt6::Network
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    wdsp
    ${LIQUID_LIBRARY}
    # System libraries
    -lm
    OpenGL::GL
)

if(SOAPYSDR_FOUND)
    target_link_libraries(cudasdr PRIVATE PkgConfig::SOAPYSDR)
endif()

if(CODEC2_FOUND)
    target_link_libraries(cudasdr PRIVATE ${CODEC2_LIBRARY})
    target_compile_definitions(cudasdr PRIVATE HAVE_CODEC2)
endif()

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

# --- Tests ---
include(CTest)
if(BUILD_TESTING)
    qt_add_executable(protocol_boundary_tests
        tests/protocol_boundary_tests.cpp
    )
    target_include_directories(protocol_boundary_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(protocol_boundary_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME protocol_boundary_tests COMMAND protocol_boundary_tests)

    qt_add_executable(protocol2_format_init_tests
        tests/protocol2_format_init_tests.cpp
    )
    target_include_directories(protocol2_format_init_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(protocol2_format_init_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME protocol2_format_init_tests COMMAND protocol2_format_init_tests)

    qt_add_executable(device_identity_tests
        tests/device_identity_tests.cpp
    )
    target_include_directories(device_identity_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(device_identity_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::Test
    )
    add_test(NAME device_identity_tests COMMAND device_identity_tests)
endif()
