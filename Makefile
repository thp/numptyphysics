
SOURCES=Path.cpp Canvas.cpp Levels.cpp Game.cpp 

ARCH=$(shell uname -m)
BINDIR=$(ARCH)
TARGET=$(ARCH)/Game

OBJECTS=$(SOURCES:%.cpp=$(BINDIR)/%.o)
CCOPTS=-Wall -I Box2D/Include 
LDOPTS=-L$(BINDIR) -lSDL -lSDL_image 

ifeq ($(ARCH),x86_64)
#CCOPTS+=-g -O3 -D ARCH_x86_64
CCOPTS+=-g -D ARCH_x86_64 -D INSTALL_BASE_PATH=\".\"
LDOPTS+=-g
endif
ifeq ($(ARCH),arm)
CCOPTS+=-O3 -D ARCH_arm -fomit-frame-pointer
CCOPTS+=-mcpu=arm1136j-s -mfpu=vfp -mfloat-abi=softfp 
#LDOPTS+=-lm_vfp
endif

BOX2DLIB=$(BINDIR)/libbox2d.a
BOX2DSOURCES= \
	Box2D/Source/Collision/b2Distance.cpp \
	Box2D/Source/Collision/b2CollideCircle.cpp \
	Box2D/Source/Collision/b2CollidePoly.cpp \
	Box2D/Source/Collision/b2PairManager.cpp \
	Box2D/Source/Collision/b2Shape.cpp \
	Box2D/Source/Collision/b2BroadPhase.cpp \
	Box2D/Source/Dynamics/b2WorldCallbacks.cpp \
	Box2D/Source/Dynamics/Joints/b2PrismaticJoint.cpp \
	Box2D/Source/Dynamics/Joints/b2MouseJoint.cpp \
	Box2D/Source/Dynamics/Joints/b2GearJoint.cpp \
	Box2D/Source/Dynamics/Joints/b2Joint.cpp \
	Box2D/Source/Dynamics/Joints/b2PulleyJoint.cpp \
	Box2D/Source/Dynamics/Joints/b2DistanceJoint.cpp \
	Box2D/Source/Dynamics/Joints/b2RevoluteJoint.cpp \
	Box2D/Source/Dynamics/Contacts/b2CircleContact.cpp \
	Box2D/Source/Dynamics/Contacts/b2PolyAndCircleContact.cpp \
	Box2D/Source/Dynamics/Contacts/b2Contact.cpp \
	Box2D/Source/Dynamics/Contacts/b2PolyContact.cpp \
	Box2D/Source/Dynamics/Contacts/b2ContactSolver.cpp \
	Box2D/Source/Dynamics/Contacts/b2Conservative.cpp \
	Box2D/Source/Dynamics/b2Island.cpp \
	Box2D/Source/Dynamics/b2Body.cpp \
	Box2D/Source/Dynamics/b2ContactManager.cpp \
	Box2D/Source/Dynamics/b2World.cpp \
	Box2D/Source/Common/b2BlockAllocator.cpp \
	Box2D/Source/Common/b2StackAllocator.cpp \
	Box2D/Source/Common/b2Settings.cpp
#BOX2DSOURCENAMES=$(notdir $(BOX2DSOURCES))
BOX2DOBJECTS=$(BOX2DSOURCES:%.cpp=$(BINDIR)/%.o)

all: $(TARGET) 

%_dirstamp:
	@mkdir -p $(@D)
	@echo t > $@

$(BOX2DLIB): $(BOX2DOBJECTS)
	ar r $@ $^
	ranlib $@

$(BINDIR)/%.o: %.cpp $(BINDIR)/%_dirstamp
	g++ -c $(CCOPTS) -MMD -MP -MF $(BINDIR)/$*.d $< -o $@

dummy:
	@cp $(BINDIR)/$*.d $(BINDIR)/$(*F).P; \
         sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
              -e '/^$$/ d' -e 's/$$/ :/' \
              < $(BINDIR)/$*.d >> $(BINDIR)/$*.P; \
        rm -f $(BINDIR)/$*.d

clean:
	rm -fR $(BINDIR)

distclean: clean
	rm *~

$(TARGET): $(OBJECTS) $(BOX2DLIB)
	g++ $^ $(LDOPTS) -o $(TARGET)


-include $(OBJECTS:%.o=%.d)
-include $(BOX2DOBJECTS:%.o=%.d)