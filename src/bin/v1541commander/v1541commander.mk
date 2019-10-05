v1541commander_PREPROCMODULES:= mainwindow
v1541commander_CXXMODULES:= $(v1541commander_PREPROCMODULES) main
v1541commander_PREPROC:= MOC
v1541commander_PREPROCFLAGS:= -p.
v1541commander_CXXFLAGS?= -fPIC
v1541commander_INCLUDES= -I$(v1541commander_PPSRCDIR)
v1541commander_PKGDEPS:= Qt5Core Qt5Gui	Qt5Widgets

$(call binrules, v1541commander)
