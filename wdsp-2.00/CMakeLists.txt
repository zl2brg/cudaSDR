cmake_minimum_required(VERSION 3.16)
project(wdsp C)

# Find dependencies
find_package(PkgConfig REQUIRED)

# Set PKG_CONFIG_PATH to find bundled libraries in wdsp-libs
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../wdsp-libs/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

pkg_check_modules(FFTW3 REQUIRED fftw3)
pkg_check_modules(FFTW3F REQUIRED fftw3f)
pkg_check_modules(RNNOISE REQUIRED rnnoise IMPORTED_TARGET)
pkg_check_modules(SPECLEACH REQUIRED libspecbleach IMPORTED_TARGET)

set(WDSP_SOURCES
    amd.c
    ammod.c
    amsq.c
    analyzer.c
    anf.c
    anr.c
    apfshadow.c
    bandpass.c
    calcc.c
    calculus.c
    cblock.c
    cfcomp.c
    cfir.c
    channel.c
    cmath.c
    compress.c
    delay.c
    dexp.c
    div.c
    doublepole.c
    eer.c
    emnr.c
    emph.c
    eq.c
    extrapolate.c
    fcurve.c
    FDnoiseIQ.c
    fir.c
    firmin.c
    fmd.c
    fmmod.c
    fmsq.c
    gain.c
    gaussian.c
    gen.c
    icfir.c
    iir.c
    impulse_cache.c
    iobuffs.c
    iqc.c
    linux_port.c
    lmath.c
    main.c
    matchedCW.c
    meter.c
    meterlog10.c
    nbp.c
    nob.c
    nobII.c
    nurbs.c
    nurbs_fit.c
    nurbs_spline.c
    osctrl.c
    patchpanel.c
    phrot.c
    resample.c
    reshb.c
    rmatch.c
    rnnr.c
    RXA.c
    sbnr.c
    sender.c
    shift.c
    siphon.c
    slew.c
    snb.c
    snoop.c
    ssql.c
    syncbuffs.c
    TXA.c
    utilities.c
    varsamp.c
    version.c
    wbfm.c
    wcpAGC.c
    wisdom.c
    zetaHat.c
)

add_library(wdsp STATIC ${WDSP_SOURCES})

target_compile_definitions(wdsp PRIVATE _GNU_SOURCE)
target_compile_options(wdsp PRIVATE -pthread -O3 -Wno-parentheses)

target_include_directories(wdsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(wdsp PRIVATE 
    ${FFTW3_INCLUDE_DIRS}
    ${FFTW3F_INCLUDE_DIRS}
)

target_link_libraries(wdsp PRIVATE 
    ${FFTW3_LIBRARIES}
    ${FFTW3F_LIBRARIES}
    PkgConfig::RNNOISE
    PkgConfig::SPECLEACH
    m
    pthread
)
