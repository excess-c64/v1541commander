include zimk/zimk.mk
$(call zinc,src/bin/v1541commander/v1541commander.mk)
ifeq ($(PLATFORM),win32)
$(call zinc,src/bin/setup/setup.mk)
endif
