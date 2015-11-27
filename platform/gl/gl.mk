# OpenGL library
ifeq ($(shell uname),Darwin)
LIBS += -framework OpenGL
else
ifneq ($(wildcard $(patsubst %,%/libGL.so.1,/usr/lib /usr/lib64 /usr/lib/* /usr/lib/*/mesa)),)
$(eval $(call add_pkgconfig,gl))
else
$(eval $(call add_pkgconfig,glesv2))
CFLAGS += -DUSE_OPENGL_ES
CXXFLAGS += -DUSE_OPENGL_ES
endif
endif
