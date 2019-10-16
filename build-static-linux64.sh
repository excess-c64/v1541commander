#!/bin/sh

LCROSS=/mnt/storage/git/xlin
TARGET=x86_64-unknown-linux-gnu
MAKE=gmake
QT5STATIC=${LCROSS}/${TARGET}/qt5-static

PKG_CONFIG_PATH=${LCROSS}/${TARGET}/lib/pkgconfig:${QT5STATIC}/lib/pkgconfig \
	PATH=${LCROSS}/${TARGET}/bin:${PATH} ${MAKE} \
	CROSS_COMPILE=${TARGET}- \
	DEFINES=-DQT_STATICPLUGIN \
	CC=gcc CXX=g++ \
	v1541commander_posix_LDFLAGS="-static -static-libstdc++ -static-libgcc -L${LCROSS}/lib -L${LCROSS}/lib64 -L${QT5STATIC}/lib -L${QT5STATIC}/plugins/platforms" \
	v1541commander_posix_LIBS="Qt5Widgets qxcb Qt5XcbQpa Qt5ServiceSupport Qt5ThemeSupport Qt5FontDatabaseSupport xcb-static xcb xkbcommon-x11 xkbcommon Xau Qt5Gui Qt5EdidSupport Qt5DBus Qt5Core png freetype qtharfbuzz fontconfig expat uuid qtpcre2 z pthread dl 1541img c" \
	${1:-strip}

