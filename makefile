
APP = numptyphysics

DESTDIR ?=
PREFIX = /opt/numptyphysics

CXXFLAGS += -I. -Isrc
CXXFLAGS += -std=c++11 -Wall -DINSTALL_BASE_PATH=\"$(PREFIX)/data\"

SOURCES = $(wildcard src/*.cpp)

all: $(APP)

# Required modules (uses pkg-config)
PKGS += sdl2 SDL2_image SDL2_ttf

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

# Other libs
CXXFLAGS += -Iexternal/thp
SOURCES += $(wildcard external/thp/*.cpp)

# Pick the right OS-specific module here
SOURCES += platform/sdl2/OsSDL2.cpp

# OpenGL platform integration code (using Glaserl)
SOURCES += platform/gl/GLRenderer.cpp
CXXFLAGS += -Iplatform/gl

LIBS += $(LOCAL_LIBS)

# Dependency tracking
DEPENDENCIES = $(SOURCES:.cpp=.d)
CXXFLAGS += -MD
-include $(DEPENDENCIES)

OBJECTS = $(SOURCES:.cpp=.o)

$(APP): $(OBJECTS) $(LOCAL_LIBS)
	$(CXX) -o $@ $^ $(LIBS)


clean:
	rm -f $(OBJECTS)
	rm -f $(DEPENDENCIES)

distclean: clean
	$(MAKE) -C $(BOX2D_SOURCE) clean
	$(MAKE) -C $(GLASERL_SOURCE) distclean
	rm -f $(APP)

install: $(APP)
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	install -m 755 $(APP) $(DESTDIR)/$(PREFIX)/bin/
	mkdir -p $(DESTDIR)/$(PREFIX)/data
	cp -rpv data/*.png data/*.ttf data/*.npz $(DESTDIR)/$(PREFIX)/data/


.PHONY: all clean distclean
.DEFAULT: all
