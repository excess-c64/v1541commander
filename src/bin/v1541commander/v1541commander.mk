v1541commander_PREPROCMODULES:= v1541commander mainwindow v1541imgwidget \
	cbmdosfsmodel cbmdosfswidget cbmdosfsoptionsdialog cbmdosfilewidget \
	cbmdosfsstatuswidget petsciiedit petsciibutton petsciiwindow \
	logwindow aboutbox cbmdosfslistview cbmdosfilemimedata
v1541commander_CXXMODULES:= $(v1541commander_PREPROCMODULES) main petsciistr \
	petsciiconvert settings
v1541commander_QRC:= resources
ifneq ($(PLATFORM),win32)
v1541commander_QRC+= icon
endif
v1541commander_PREPROC:= MOC
v1541commander_PREPROCFLAGS:= -p.
v1541commander_posix_CXXFLAGS?= -fPIC
v1541commander_INCLUDES= -I$(v1541commander_PPSRCDIR)
v1541commander_CXXFLAGS= -fno-exceptions -fno-rtti
v1541commander_PKGDEPS:= Qt5Core Qt5Gui	Qt5Widgets Qt5Network \
	1541img >= 1.0 1541img < 2
v1541commander_ICONSIZES:= 16x16 32x32 48x48 256x256
v1541commander_MIMEICONS:= v1541commander-d64 v1541commander-zipcode \
	v1541commander-lynx v1541commander-prg
v1541commander_DESKTOPFILE:= v1541commander
v1541commander_SHAREDMIMEINFO:= v1541commander
v1541commander_DOCS:= README.md BUILDING.md LICENSE.txt LICENSE-font.txt
v1541commander_win32_LDFLAGS:= -mwindows
v1541commander_win32_RES:= windres
ifneq ($(EMBEDQTL10N),)
v1541commander_QRC+= qtl10n
endif

$(call binrules, v1541commander)

ifneq ($(EMBEDQTL10N),)
$(v1541commander_OBJDIR)$(PSEP)qtl10n_qrc.cpp: \
	$(v1541commander_SRCDIR)$(PSEP)qtl10n.qrc
	cp $(EMBEDQTL10N)$(PSEP)qtbase_de.qm $(v1541commander_SRCDIR)
	$(RCC) -o $@ --name qtl10n $<
	$(RMF) $(v1541commander_SRCDIR)$(PSEP)qtbase_de.qm

endif


v1541commander_LANGS:= de

$(v1541commander_OBJDIR)$(PSEP)resources_qrc.cpp: $(addsuffix .qm,$(addprefix \
	$(v1541commander_SRCDIR)$(PSEP)qm$(PSEP)v1541commander-, \
	$(v1541commander_LANGS)))

LRELEASE?= lrelease

$(v1541commander_SRCDIR)$(PSEP)qm$(PSEP)%.qm: \
	$(v1541commander_SRCDIR)$(PSEP)%.ts
	@$(MDP) $(v1541commander_SRCDIR)$(PSEP)qm
	$(LRELEASE) $< -qm $@

clean::
	rm -fr $(v1541commander_SRCDIR)$(PSEP)qm

