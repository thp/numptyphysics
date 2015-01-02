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
