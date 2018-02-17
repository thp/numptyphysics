APP := numptyphysics

ifneq ($(shell echo),)
	CMD_EXE = 1
endif

ifdef CMD_EXE
	NULLDEV = nul:
else
	NULLDEV = /dev/null
endif

ifndef VERSION
	VERSION := $(shell git describe --tags HEAD 2> $(NULLDEV) )
	ifneq ($(words $(VERSION)),1)
		VERSION := "N/A"
	endif
endif
$(info VERSION: $(VERSION))

SOURCES := $(wildcard src/*.cpp)
CXXFLAGS += -std=c++11 -Isrc -Wall -Wno-sign-compare -DAPP=\"$(APP)\" -DVERSION=\"$(VERSION)\"

ifdef DEBUG
	CXXFLAGS += -g
endif

all: app

include mk/top.mk

TARGET ?= $(APP)

app: $(TARGET)

$(OBJECTS): $(GENERATED_HEADERS)

$(TARGET): $(OBJECTS) $(LOCAL_LIBS)
	$(SILENTMSG) "\tLD\t$@\n"
	$(SILENTCMD) $(CXX) -o $@ $^ $(LIBS)

clean: $(ADDITIONAL_CLEAN_TARGETS)
	$(SILENTMSG) "\tCLEAN\n"
	$(SILENTCMD) $(RM) $(CLEAN_FILES)

distclean: clean $(ADDITIONAL_DISTCLEAN_TARGETS)
	$(SILENTMSG) "\tDISTCLEAN\n"
	$(SILENTCMD) $(RM) $(APP) $(GENERATED_MAKEFILES)
	$(SILENTCMD) $(RM) $(DISTCLEAN_FILES)

.PHONY: all clean distclean
.DEFAULT: all
