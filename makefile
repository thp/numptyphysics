
APP = numptyphysics

DESTDIR ?=
PREFIX = /opt/numptyphysics

CXXFLAGS += -std=c++11 -DINSTALL_BASE_PATH=\"$(PREFIX)/data\"

SOURCES = $(wildcard *.cpp)

all: $(APP)

# Required modules (uses pkg-config)
PKGS += sdl2 SDL2_image SDL2_ttf

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
LIBS += -lz -framework OpenGL
else
PKGS += zlib gl
endif

CXXFLAGS += $(shell pkg-config --cflags $(PKGS))
LIBS += $(shell pkg-config --libs $(PKGS))

# Box2D Library
CXXFLAGS += -IBox2D/Include
BOX2D_SOURCE := Box2D/Source
BOX2D_LIBRARY := Gen/float/libbox2d.a
LIBS += $(BOX2D_SOURCE)/$(BOX2D_LIBRARY)

$(BOX2D_SOURCE)/$(BOX2D_LIBRARY):
	$(MAKE) -C $(BOX2D_SOURCE) $(BOX2D_LIBRARY)


# Pick the right OS-specific module here
SOURCES += os/OsSDL2.cpp
CXXFLAGS += -I.

# Dependency tracking
DEPENDENCIES = $(SOURCES:.cpp=.d)
CXXFLAGS += -MD
-include $(DEPENDENCIES)

OBJECTS = $(SOURCES:.cpp=.o)

Dialogs.cpp: help_text_html.h

%_html.h: %.html
	xxd -i $< $@

$(APP): $(OBJECTS) $(BOX2D_SOURCE)/$(BOX2D_LIBRARY)
	$(CXX) -o $@ $^ $(LIBS)


clean:
	rm -f $(OBJECTS)
	rm -f $(DEPENDENCIES)
	rm -f help_text_html.h
	$(MAKE) -C Box2D/Source clean

distclean: clean
	rm -f $(APP)

install: $(APP)
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	install -m 755 $(APP) $(DESTDIR)/$(PREFIX)/bin/
	mkdir -p $(DESTDIR)/usr/share/applications
	install -m 644 $(APP).desktop $(DESTDIR)/usr/share/applications/
	mkdir -p $(DESTDIR)/$(PREFIX)/data
	cp -rpv data/*.png data/*.ttf data/*.npz $(DESTDIR)/$(PREFIX)/data/


.PHONY: all clean distclean
.DEFAULT: all

