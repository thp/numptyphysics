# Glaserl Library
CXXFLAGS += -Iexternal/glaserl
GLASERL_SOURCE := external/glaserl
GLASERL_LIBRARY := libglaserl.a

LOCAL_LIBS += $(GLASERL_SOURCE)/$(GLASERL_LIBRARY)
$(GLASERL_SOURCE)/$(GLASERL_LIBRARY):
	CFLAGS='$(CFLAGS)' $(MAKE) -C $(GLASERL_SOURCE) $(GLASERL_LIBRARY)

ADDITIONAL_DISTCLEAN_TARGETS += glaserl_distclean
glaserl_distclean:
	$(SILENTCMD) $(MAKE) -C $(GLASERL_SOURCE) distclean
.PHONY: glaserl_distclean
