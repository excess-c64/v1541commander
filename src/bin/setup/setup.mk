setup_MODULES:= main
setup_LDFLAGS:= -mwindows
setup_LIBS:= gdi32 comctl32 shlwapi
setup_win32_RES:= windres

$(call binrules, setup)

