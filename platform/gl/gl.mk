# OpenGL library
ifeq ($(shell uname),Darwin)
LIBS += -framework OpenGL
else ifneq ($(PLATFORM),mingw)
ifeq ($(PLATFORM_GL_GLES),1)
$(eval $(call add_pkgconfig,glesv2))
CFLAGS += -DUSE_OPENGL_ES
CXXFLAGS += -DUSE_OPENGL_ES
else
$(eval $(call add_pkgconfig,gl))
endif
else
LIBS += -lopengl32
endif
