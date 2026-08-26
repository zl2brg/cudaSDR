cmake_minimum_required(VERSION 3.16)
project(cudaSDR LANGUAGES CXX)

# --- Build parallelism cap ---
# Detect available cores so builds don't run unbounded. NOTE: the *top-level*
# job count is chosen by the build invocation (see CMakePresets.json / the
# CMAKE_BUILD_PARALLEL_LEVEL env var); this value bounds anything CMake launches
# itself (e.g. the RADE/RNNoise ExternalProjects) and is reported for reference.
include(ProcessorCount)
ProcessorCount(CUDASDR_HOST_CORES)
if(CUDASDR_HOST_CORES EQUAL 0)
    set(CUDASDR_HOST_CORES 1)
endif()
set(CUDASDR_PARALLEL_JOBS ${CUDASDR_HOST_CORES} CACHE STRING
    "Max parallel compile jobs for cudaSDR and its bundled dependencies")
message(STATUS "cudaSDR: capping parallel build jobs at ${CUDASDR_PARALLEL_JOBS} (host cores: ${CUDASDR_HOST_CORES})")
message(STATUS "cudaSDR: build with 'cmake --build build -j ${CUDASDR_PARALLEL_JOBS}' or use the bundled CMakePresets")

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

option(CUDASDR_ENABLE_COVERAGE "Build with gcov/lcov coverage instrumentation" OFF)
if(CUDASDR_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage)
        add_link_options(--coverage)
        message(STATUS "Coverage instrumentation enabled (gcov/lcov)")
    else()
        message(WARNING "CUDASDR_ENABLE_COVERAGE is only supported with GCC or Clang")
    endif()
endif()

# --- Find Dependencies ---
find_package(Qt6 6.11.0 REQUIRED COMPONENTS Widgets Core Gui GuiPrivate ShaderTools Multimedia Network OpenGL OpenGLWidgets WebSockets Test)
message(STATUS "Using Qt ${Qt6_VERSION} from ${Qt6_DIR}")
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)

# Set PKG_CONFIG_PATH to find bundled libraries in wdsp-libs
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

# --- Find liquid-dsp (local) ---
find_path(LIQUID_INCLUDE_DIR NAMES liquid/liquid.h PATHS ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs/include NO_DEFAULT_PATH)
find_library(LIQUID_LIBRARY NAMES liquid PATHS ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs/lib NO_DEFAULT_PATH)

if(NOT LIQUID_INCLUDE_DIR OR NOT LIQUID_LIBRARY)
    message(FATAL_ERROR "liquid-dsp not found in wdsp-libs! Please build and install it first.")
endif()
message(STATUS "liquid-dsp found: lib=${LIQUID_LIBRARY}, include=${LIQUID_INCLUDE_DIR}")

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
set(WDSP_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-2.00)
set(WDSP_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/wdsp-libs)

# --- Add Subdirectories ---
add_subdirectory(wdsp-2.00)
add_subdirectory(deps/freedv-backend)

# --- Define Source Files ---
set(SOURCES
    # Main application
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/Models/SliceModel.cpp
    ${SRC_DIR}/Models/RadioModel.cpp
    ${SRC_DIR}/Models/TransmitModel.cpp
    ${SRC_DIR}/Models/RadioTelemetry.cpp
    ${SRC_DIR}/Models/BandPlanManager.cpp
    ${SRC_DIR}/Controllers/RadioController.cpp
    ${SRC_DIR}/Controllers/RadioSettingsController.cpp
    ${SRC_DIR}/Controllers/NetworkSettingsController.cpp
    ${SRC_DIR}/Controllers/HpsdrSettingsController.cpp
    ${SRC_DIR}/Controllers/ExtCtrlSettingsController.cpp
    ${SRC_DIR}/Controllers/AlexSettingsController.cpp
    ${SRC_DIR}/Controllers/TransmitSettingsController.cpp
    ${SRC_DIR}/Controllers/DisplaySettingsController.cpp
    ${SRC_DIR}/Controllers/RadioPopupController.cpp
    ${SRC_DIR}/Controllers/ServerSettingsController.cpp
    ${SRC_DIR}/Controllers/SetupController.cpp
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
    ${SRC_DIR}/AudioEngine/cusdr_radeprocessor.cpp
    ${SRC_DIR}/AudioEngine/CwDecoder.cpp

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
    ${SRC_DIR}/GL/cusdr_spectrumBinWorker.cpp
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
    ${SRC_DIR}/Util/cusdr_tciserver.cpp
    ${SRC_DIR}/Util/TciRoutingState.cpp
    ${SRC_DIR}/Util/TciCommandHandler.cpp
    ${SRC_DIR}/Util/AudioDeviceService.cpp
    ${SRC_DIR}/Util/DxClusterClient.cpp

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
    ${SRC_DIR}/UI/eq_curve_plot.cpp
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
    ${SRC_DIR}/Settings/AlexConfig.cpp
    ${SRC_DIR}/Settings/TransmitConfig.cpp
    ${SRC_DIR}/Settings/FreeDVConfig.cpp
    ${SRC_DIR}/Settings/WindowConfig.cpp
    ${SRC_DIR}/Settings/TciConfig.cpp
    ${SRC_DIR}/Settings/SoapyConfig.cpp
    ${SRC_DIR}/UI/MainWindow/MainWindowUI.cpp
    ${SRC_DIR}/GL/WaterfallRenderer.cpp
    ${SRC_DIR}/GL/PanadapterRenderer.cpp
    ${SRC_DIR}/GL/OverlayRenderer.cpp
    ${SRC_DIR}/GL/GridRenderer.cpp
    ${SRC_DIR}/GL/TraceRenderer.cpp
    ${SRC_DIR}/GL/HudRenderer.cpp
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
    ${SRC_DIR}/AudioEngine/cusdr_radeprocessor.h
    ${SRC_DIR}/AudioEngine/CwDecoder.h

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
    ${SRC_DIR}/UI/eq_curve_plot.h
    ${SRC_DIR}/UI/tx_settings_dialog.h
    ${SRC_DIR}/UI/cusdr_setupwidget.h
    ${SRC_DIR}/UI/FrequencyEntryDialog.h
    ${SRC_DIR}/UI/DeviceSelectionDialog.h


    # GL
    ${SRC_DIR}/GL/cusdr_oglDisplayPanel.h
    ${SRC_DIR}/GL/cusdr_oglDistancePanel.h
    ${SRC_DIR}/GL/cusdr_oglInfo.h
    ${SRC_DIR}/GL/cusdr_oglReceiverPanel.h
    ${SRC_DIR}/GL/cusdr_spectrumBinWorker.h
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
    ${SRC_DIR}/Util/cusdr_tciserver.h
    ${SRC_DIR}/Util/TciRoutingState.h
    ${SRC_DIR}/Util/TciCommandHandler.h
    ${SRC_DIR}/Util/AudioDeviceService.h
    ${SRC_DIR}/Util/DxClusterClient.h

    # Main Headers
    ${SRC_DIR}/cusdr_settings.h
    ${SRC_DIR}/Settings/NetworkConfig.h
    ${SRC_DIR}/Settings/DisplayConfig.h
    ${SRC_DIR}/Settings/HardwareConfig.h
    ${SRC_DIR}/Settings/AudioConfig.h
    ${SRC_DIR}/Settings/CWConfig.h
    ${SRC_DIR}/Settings/ReceiverConfig.h
    ${SRC_DIR}/Settings/AlexConfig.h
    ${SRC_DIR}/Settings/TransmitConfig.h
    ${SRC_DIR}/Settings/FreeDVConfig.h
    ${SRC_DIR}/Settings/WindowConfig.h
    ${SRC_DIR}/Settings/TciConfig.h
    ${SRC_DIR}/Settings/SoapyConfig.h
    ${SRC_DIR}/UI/MainWindow/MainWindowUI.h
    ${SRC_DIR}/GL/WaterfallRenderer.h
    ${SRC_DIR}/GL/PanadapterRenderer.h
    ${SRC_DIR}/GL/OverlayRenderer.h
    ${SRC_DIR}/GL/GridRenderer.h
    ${SRC_DIR}/GL/TraceRenderer.h
    ${SRC_DIR}/GL/HudRenderer.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/freedv-backend/src/pipeline
    ${CMAKE_CURRENT_BINARY_DIR}/deps/freedv-backend
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
    Qt6::WebSockets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    wdsp
    ${LIQUID_LIBRARY}
    -lm
    OpenGL::GL
    fdv_audio_pipeline
    rade
    opus
)

target_compile_definitions(cudasdr PRIVATE HAVE_RADE)

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
    set(CUDASDR_SETTINGS_TEST_SOURCES
        ${SRC_DIR}/cusdr_settings.cpp
        ${SRC_DIR}/cusdr_hamDatabase.cpp
        ${SRC_DIR}/Settings/NetworkConfig.cpp
        ${SRC_DIR}/Settings/DisplayConfig.cpp
        ${SRC_DIR}/Settings/HardwareConfig.cpp
        ${SRC_DIR}/Settings/AudioConfig.cpp
        ${SRC_DIR}/Settings/CWConfig.cpp
        ${SRC_DIR}/Settings/ReceiverConfig.cpp
        ${SRC_DIR}/Settings/AlexConfig.cpp
        ${SRC_DIR}/Settings/TransmitConfig.cpp
        ${SRC_DIR}/Settings/FreeDVConfig.cpp
        ${SRC_DIR}/Settings/WindowConfig.cpp
        ${SRC_DIR}/Settings/TciConfig.cpp
        ${SRC_DIR}/Settings/SoapyConfig.cpp
        ${SRC_DIR}/Models/RadioModel.cpp
        ${SRC_DIR}/Models/SliceModel.cpp
        ${SRC_DIR}/Models/TransmitModel.cpp
        ${SRC_DIR}/Models/RadioTelemetry.cpp
        ${SRC_DIR}/Models/BandPlanManager.cpp
        ${SRC_DIR}/Util/DxClusterClient.cpp
    )

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

    qt_add_executable(alex_encoding_tests
        tests/alex_encoding_tests.cpp
    )
    target_include_directories(alex_encoding_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(alex_encoding_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME alex_encoding_tests COMMAND alex_encoding_tests)

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

    qt_add_executable(tci_protocol_utils_tests
        tests/tci_protocol_utils_tests.cpp
    )
    target_include_directories(tci_protocol_utils_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(tci_protocol_utils_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME tci_protocol_utils_tests COMMAND tci_protocol_utils_tests)

    qt_add_executable(tci_routing_tests
        tests/tci_routing_tests.cpp
        ${SRC_DIR}/Util/TciRoutingState.cpp
        ${SRC_DIR}/Util/TciCommandHandler.cpp
    )
    target_include_directories(tci_routing_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(tci_routing_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME tci_routing_tests COMMAND tci_routing_tests)

    qt_add_executable(display_utils_tests
        tests/display_utils_tests.cpp
    )
    target_include_directories(display_utils_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(display_utils_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME display_utils_tests COMMAND display_utils_tests)

    qt_add_executable(rigctl_protocol_utils_tests
        tests/rigctl_protocol_utils_tests.cpp
    )
    target_include_directories(rigctl_protocol_utils_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(rigctl_protocol_utils_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME rigctl_protocol_utils_tests COMMAND rigctl_protocol_utils_tests)

    qt_add_executable(settings_utils_tests
        tests/settings_utils_tests.cpp
    )
    target_include_directories(settings_utils_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(settings_utils_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME settings_utils_tests COMMAND settings_utils_tests)

    qt_add_executable(ham_database_utils_tests
        tests/ham_database_utils_tests.cpp
        ${SRC_DIR}/cusdr_hamDatabase.cpp
    )
    target_include_directories(ham_database_utils_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(ham_database_utils_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME ham_database_utils_tests COMMAND ham_database_utils_tests)

    qt_add_executable(settings_persistence_tests
        tests/settings_persistence_tests.cpp
        ${CUDASDR_SETTINGS_TEST_SOURCES}
    )
    target_include_directories(settings_persistence_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(settings_persistence_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::WebSockets
        Qt6::Test
    )
    add_test(NAME settings_persistence_tests COMMAND settings_persistence_tests)
    set_tests_properties(settings_persistence_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    qt_add_executable(settings_signal_tests
        tests/settings_signal_tests.cpp
        ${CUDASDR_SETTINGS_TEST_SOURCES}
    )
    target_include_directories(settings_signal_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(settings_signal_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::WebSockets
        Qt6::Test
    )
    add_test(NAME settings_signal_tests COMMAND settings_signal_tests)
    set_tests_properties(settings_signal_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    qt_add_executable(tci_server_ws_tests
        tests/tci_server_ws_tests.cpp
        ${SRC_DIR}/Util/cusdr_tciserver.cpp
        ${SRC_DIR}/Util/TciRoutingState.cpp
        ${SRC_DIR}/Util/TciCommandHandler.cpp
        ${CUDASDR_SETTINGS_TEST_SOURCES}
    )
    target_include_directories(tci_server_ws_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(tci_server_ws_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::WebSockets
        Qt6::Test
    )
    add_test(NAME tci_server_ws_tests COMMAND tci_server_ws_tests)
    set_tests_properties(tci_server_ws_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    qt_add_executable(models_tests
        tests/models_tests.cpp
        ${CUDASDR_SETTINGS_TEST_SOURCES}
    )
    target_include_directories(models_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(models_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::WebSockets
        Qt6::Test
    )
    add_test(NAME models_tests COMMAND models_tests)
    set_tests_properties(models_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    qt_add_executable(config_json_tests
        tests/config_json_tests.cpp
        ${CUDASDR_SETTINGS_TEST_SOURCES}
    )
    target_include_directories(config_json_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(config_json_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::WebSockets
        Qt6::Test
    )
    add_test(NAME config_json_tests COMMAND config_json_tests)
    set_tests_properties(config_json_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    qt_add_executable(dx_cluster_tests
        tests/dx_cluster_tests.cpp
        ${SRC_DIR}/Util/DxClusterClient.cpp
        ${SRC_DIR}/Models/BandPlanManager.cpp
    )
    target_include_directories(dx_cluster_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(dx_cluster_tests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::WebSockets
        Qt6::Test
    )
    add_test(NAME dx_cluster_tests COMMAND dx_cluster_tests)
    set_tests_properties(dx_cluster_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    qt_add_executable(cw_decoder_tests
        tests/cw_decoder_tests.cpp
        ${SRC_DIR}/AudioEngine/CwDecoder.cpp
    )
    target_include_directories(cw_decoder_tests PRIVATE
        ${SRC_DIR}
    )
    target_link_libraries(cw_decoder_tests PRIVATE
        Qt6::Core
        Qt6::Test
    )
    add_test(NAME cw_decoder_tests COMMAND cw_decoder_tests)
    set_tests_properties(cw_decoder_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )

    # Convenience target: build every unit test without the full cudasdr app.
    add_custom_target(build_tests DEPENDS
        protocol_boundary_tests
        alex_encoding_tests
        protocol2_format_init_tests
        device_identity_tests
        tci_protocol_utils_tests
        tci_routing_tests
        display_utils_tests
        rigctl_protocol_utils_tests
        settings_utils_tests
        ham_database_utils_tests
        settings_persistence_tests
        settings_signal_tests
        tci_server_ws_tests
        models_tests
        config_json_tests
        dx_cluster_tests
        cw_decoder_tests
    )

    # Headless Qt for any test that links Gui/Widgets (CI has no display).
    get_property(_cudasdr_tests DIRECTORY PROPERTY TESTS)
    if(_cudasdr_tests)
        set_tests_properties(${_cudasdr_tests} PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        )
    endif()
endif()
