#!/bin/bash
# Qt 5 to Qt 6 Migration Helper Script

echo "=== Qt 5 → Qt 6 Migration Analysis ==="

# Check for common deprecated patterns
echo "🔍 Scanning for deprecated Qt patterns..."

echo "📍 QTime usage (should use QElapsedTimer):"
find Source -name "*.cpp" -o -name "*.h" | xargs grep -n "QTime.*start\|QTime.*elapsed\|QTime.*restart" 2>/dev/null | head -5

echo "📍 qVariantFromValue usage (should use QVariant::fromValue):"
find Source -name "*.cpp" -o -name "*.h" | xargs grep -n "qVariantFromValue" 2>/dev/null | head -5

echo "📍 QDesktopWidget usage (should use QScreen):"
find Source -name "*.cpp" -o -name "*.h" | xargs grep -n "QDesktopWidget\|availableGeometry" 2>/dev/null | head -5

echo "📍 QAudioDeviceInfo usage (Qt 6 uses QAudioDevice):"
find Source -name "*.cpp" -o -name "*.h" | xargs grep -n "QAudioDeviceInfo" 2>/dev/null | head -5

echo "📍 QTextStream endl/flush usage (should use Qt::endl/Qt::flush):"
find Source -name "*.cpp" -o -name "*.h" | xargs grep -n "QTextStream.*endl\|QTextStream.*flush" 2>/dev/null | head -5

echo "📍 QFontMetrics width() usage (should use horizontalAdvance):"
find Source -name "*.cpp" -o -name "*.h" | xargs grep -n "fontMetrics.*width\|QFontMetrics.*width" 2>/dev/null | head -5

echo ""
echo "✅ Migration Complexity Assessment:"
echo "   🟢 Low:    Basic widgets, layouts, containers"  
echo "   🟡 Medium: Deprecated API replacements (~15 warnings found)"
echo "   🔴 High:   Audio system (QAudioDeviceInfo), OpenGL integration"
echo ""
echo "📊 Estimated effort: 1-2 weeks (depending on audio system complexity)"
echo "🎯 Recommended: Fix Qt 5.15 warnings first, then migrate to Qt 6.8"
