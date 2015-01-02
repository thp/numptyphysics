
APP = numptyphysics

DESTDIR ?=
PREFIX = /opt/numptyphysics

PLATFORM ?= sdl2

CXXFLAGS += -I. -Isrc -Wno-sign-compare
CXXFLAGS += -std=c++11 -Wall -DINSTALL_BASE_PATH=\"$(PREFIX)/data\"

GENERATED_MAKEFILES :=

ifeq ($(SILENT),0)
    SILENTMSG := @true
    SILENTCMD :=
else
    SILENTMSG := @echo
    SILENTCMD := @
endif

ifeq ($(V),1)
    SILENTMSG := @true
    SILENTCMD :=
endif

SOURCES = $(wildcard src/*.cpp)

all: $(APP)

define add_pkgconfig
PKGS += $(1)
endef

define add_platform
CXXFLAGS += -Iplatform/$(1)
SOURCES += $(wildcard platform/$(1)/*.cpp)
endef

define add_external
CXXFLAGS += -Iexternal/$(1)
SOURCES += $(wildcard external/$(1)/*.cpp)
endef

define include_makefile
GENERATED_MAKEFILES += $(1)
-include $(1)
endef

%.mk: %.in
	$(SILENTMSG) "\tSED\t$@"
	$(SILENTCMD) sed -e 's/^\([^ ]*\)(\(.*\))$$/$$(eval $$(call \1,\2))/g' $< >$@ || rm -f $@

$(eval $(call add_platform,$(PLATFORM)))
$(eval $(call include_makefile,platform/$(PLATFORM)/platform.mk))
$(eval $(call include_makefile,external.mk))

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
LIBS += -framework OpenGL
else
PKGS += gl
endif

CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LIBS += $(shell pkg-config --libs $(PKGS))

# Box2D Library
CXXFLAGS += -Iexternal/Box2D/Include
BOX2D_SOURCE := external/Box2D/Source
BOX2D_LIBRARY := Gen/float/libbox2d.a
LOCAL_LIBS += $(BOX2D_SOURCE)/$(BOX2D_LIBRARY)

$(BOX2D_SOURCE)/$(BOX2D_LIBRARY):
	$(MAKE) -C $(BOX2D_SOURCE) $(BOX2D_LIBRARY)

# Glaserl Library
CXXFLAGS += -Iexternal/glaserl
GLASERL_SOURCE := external/glaserl
GLASERL_LIBRARY := libglaserl.a
LOCAL_LIBS += $(GLASERL_SOURCE)/$(GLASERL_LIBRARY)

$(GLASERL_SOURCE)/$(GLASERL_LIBRARY):
	$(MAKE) -C $(GLASERL_SOURCE) $(GLASERL_LIBRARY)

LIBS += $(LOCAL_LIBS)

# Dependency tracking
DEPENDENCIES = $(SOURCES:.cpp=.d)
CXXFLAGS += -MD
-include $(DEPENDENCIES)

OBJECTS = $(SOURCES:.cpp=.o)

$(APP): $(OBJECTS) $(LOCAL_LIBS)
	$(SILENTMSG) "\tLD\t$@"
	$(SILENTCMD) $(CXX) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(SILENTMSG) "\tCXX\t$@"
	$(SILENTCMD) $(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	$(SILENTMSG) "\tCLEAN"
	$(SILENTCMD) rm -f $(OBJECTS)
	$(SILENTCMD) rm -f $(DEPENDENCIES)

distclean: clean
	$(SILENTMSG) "\tDISTCLEAN"
	$(SILENTCMD) $(MAKE) -C $(BOX2D_SOURCE) clean
	$(SILENTCMD) $(MAKE) -C $(GLASERL_SOURCE) distclean
	$(SILENTCMD) rm -f $(APP)
	$(SILENTCMD) rm -f $(GENERATED_MAKEFILES)

install: $(APP)
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	install -m 755 $(APP) $(DESTDIR)/$(PREFIX)/bin/
	mkdir -p $(DESTDIR)/$(PREFIX)/data
	cp -rpv data/*.png data/*.ttf $(DESTDIR)/$(PREFIX)/data/


.PHONY: all clean distclean
.DEFAULT: all
