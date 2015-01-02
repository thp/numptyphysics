ifneq ($(PKGS),)
CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LIBS += $(shell pkg-config --libs $(PKGS))
endif
