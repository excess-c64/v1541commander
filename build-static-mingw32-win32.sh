#!/bin/sh

QT5STATIC=/mingw32/qt5-static
LOCALLIB=/usr/local/lib
MAKE=make
MAKEFLAGS=-j4

PKG_CONFIG_PATH=${QT5STATIC}/lib/pkgconfig:${LOCALLIB}/pkgconfig:${PKG_CONFIG_PATH} ${MAKE} ${MAKEFLAGS} \
	DEFINES="-DQT_STATICPLUGIN -DSTATIC_1541IMG" \
	CC=gcc CXX=g++ \
	v1541commander_win32_LDFLAGS="-mwindows -static -static-libstdc++ -static-libgcc -L${QT5STATIC}/share/qt5/plugins/platforms -L${QT5STATIC}/share/qt5/plugins/styles" \
	v1541commander_win32_LIBS="Qt5Widgets qwindows qwindowsvistastyle wtsapi32 Qt5EventDispatcherSupport Qt5ThemeSupport Qt5FontDatabaseSupport Qt5WindowsUIAutomationSupport Qt5PlatformCompositorSupport Qt5VulkanSupport Qt5Gui Qt5core png freetype harfbuzz bz2 graphite2 usp10 fontconfig expat qtpcre2 z zstd pthread 1541img rpcrt4 uxtheme kernel32 ole32 oleaut32 shell32 uuid version userenv winmm dwmapi netapi32 imm32 ws2_32" \
	${1:-strip}

