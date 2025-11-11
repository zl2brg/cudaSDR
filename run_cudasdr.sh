#!/bin/bash
# Test script for cudasdr with proper environment

export QT_QPA_PLATFORM=xcb
export QT_QPA_PLATFORM_PLUGIN_PATH=/home/simon/Qt/5.15.2/gcc_64/plugins/platforms  
export LD_LIBRARY_PATH=/home/simon/Qt/5.15.2/gcc_64/lib:$LD_LIBRARY_PATH
export QT_XCB_GL_INTEGRATION=xcb_egl
export WAYLAND_DISPLAY=""

echo "Starting cudasdr with X11 platform..."
echo "Environment variables:"
echo "  QT_QPA_PLATFORM=$QT_QPA_PLATFORM"
echo "  QT_QPA_PLATFORM_PLUGIN_PATH=$QT_QPA_PLATFORM_PLUGIN_PATH"
echo "  LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

exec ./build/Debug/cudasdr "$@"
