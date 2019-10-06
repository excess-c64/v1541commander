v1541commander_PREPROCMODULES:= mainwindow v1541imgwidget
v1541commander_CXXMODULES:= $(v1541commander_PREPROCMODULES) main petsciistr
v1541commander_QRC:= resources
v1541commander_PREPROC:= MOC
v1541commander_PREPROCFLAGS:= -p.
v1541commander_CXXFLAGS?= -fPIC
v1541commander_INCLUDES= -I$(v1541commander_PPSRCDIR)
v1541commander_PKGDEPS:= Qt5Core Qt5Gui	Qt5Widgets 1541img
v1541commander_win32_LDFLAGS:= -mwindows

$(call binrules, v1541commander)
