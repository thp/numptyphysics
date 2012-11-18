
APP = numptyphysics

DESTDIR ?=
PREFIX = app/native

CXXFLAGS += -DINSTALL_BASE_PATH=\"$(PREFIX)/data\"

SOURCES = $(wildcard *.cpp)

all: $(APP)

# Required modules (uses pkg-config)
PKGS = sdl SDL_image

CXXFLAGS += -I/home/thp/pkg/bb/SDL/playbook_prefix/include
CXXFLAGS += -I/home/thp/pkg/bb/SDL/playbook_prefix/include/SDL
LIBS += -L/home/thp/pkg/bb/SDL/playbook_prefix/lib

CXXFLAGS += -I/home/thp/pkg/bb/SDL_image-1.2.12/playbook_prefix/include
LIBS += -L/home/thp/pkg/bb/SDL_image-1.2.12/playbook_prefix/lib

CXXFLAGS += -I/home/thp/pkg/bb/SDL_ttf-2.0.11/playbook_prefix/include
LIBS += -L/home/thp/pkg/bb/SDL_ttf-2.0.11/playbook_prefix/lib

LIBS += -lSDL -lSDL_image -lSDL_ttf -lfreetype -Wl,-rpath,'./app/native/lib'
CXXFLAGS += -D_GNU_SOURCE=1

CXX = arm-unknown-nto-qnx6.5.0eabi-c++

# Box2D Library
CXXFLAGS += -IBox2D/Include
BOX2D_SOURCE := Box2D/Source
BOX2D_LIBRARY := Gen/float/libbox2d.a
LIBS += $(BOX2D_SOURCE)/$(BOX2D_LIBRARY)

# Pick the right OS-specific module here
SOURCES += os/OsFreeDesktop.cpp
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

