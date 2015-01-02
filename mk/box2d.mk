# Box2D Library
CXXFLAGS += -Iexternal/Box2D/Include
BOX2D_SOURCE := external/Box2D/Source
BOX2D_LIBRARY := Gen/float/libbox2d.a

LOCAL_LIBS += $(BOX2D_SOURCE)/$(BOX2D_LIBRARY)
$(BOX2D_SOURCE)/$(BOX2D_LIBRARY):
	$(MAKE) -C $(BOX2D_SOURCE) $(BOX2D_LIBRARY)

ADDITIONAL_DISTCLEAN_TARGETS += box2d_distclean
box2d_distclean:
	$(SILENTCMD) $(MAKE) -C $(BOX2D_SOURCE) clean
.PHONY: box2d_distclean
