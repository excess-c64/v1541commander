v1541commander_PREPROCMODULES:= v1541commander mainwindow v1541imgwidget \
	cbmdosfsmodel cbmdosfswidget cbmdosfsoptionsdialog cbmdosfilewidget \
	petsciiedit petsciibutton petsciiwindow logwindow aboutbox
v1541commander_CXXMODULES:= $(v1541commander_PREPROCMODULES) main petsciistr \
	petsciiconvert
v1541commander_QRC:= resources
ifneq ($(PLATFORM),win32)
v1541commander_QRC+= icon
endif
v1541commander_PREPROC:= MOC
v1541commander_PREPROCFLAGS:= -p.
v1541commander_posix_CXXFLAGS?= -fPIC
v1541commander_INCLUDES= -I$(v1541commander_PPSRCDIR)
v1541commander_PKGDEPS:= Qt5Core Qt5Gui	Qt5Widgets Qt5Network 1541img
v1541commander_win32_LDFLAGS:= -mwindows
v1541commander_win32_RES:= windres

$(call binrules, v1541commander)
