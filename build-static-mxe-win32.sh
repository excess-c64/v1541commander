#!/bin/sh

MXE=/mnt/storage/git/mxe
TARGET=i686-w64-mingw32.static
MAKE=gmake

PATH=${MXE}/usr/bin:${PATH} ${MAKE} \
	CROSS_COMPILE=${TARGET}- \
	DEFINES=-DQT_STATICPLUGIN \
	CC=gcc CXX=g++ \
	PKGCONFIG=${TARGET}-pkg-config \
	RCC=${MXE}/usr/${TARGET}/qt5/bin/rcc \
	MOC=${MXE}/usr/${TARGET}/qt5/bin/moc \
	v1541commander_win32_LDFLAGS="-mwindows -L${MXE}/usr/${TARGET}/qt5/plugins/platforms -L${MXE}/usr/${TARGET}/qt5/plugins/styles" \
	v1541commander_win32_LIBS="qwindows qwindowsvistastyle wtsapi32 Qt5EventDispatcherSupport Qt5ThemeSupport Qt5FontDatabaseSupport Qt5WindowsUIAutomationSupport Qt5PlatformCompositorSupport" \
	${1:-strip}

