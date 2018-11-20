ifneq ($(PKGS),)
CXXFLAGS += $(shell $(CROSS_COMPILE)pkg-config --cflags $(PKGS))
LIBS += $(shell $(CROSS_COMPILE)pkg-config --libs $(PKGS))
endif
