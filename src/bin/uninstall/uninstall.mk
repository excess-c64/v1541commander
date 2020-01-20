uninstall_MODULES:= main
uninstall_LDFLAGS:= -mwindows
uninstall_LIBS:= gdi32 comctl32
uninstall_win32_RES:= windres

$(call binrules, uninstall)

