# Dependency tracking
DEPENDENCIES := $(SOURCES:.cpp=.d)
CXXFLAGS += -MD
-include $(DEPENDENCIES)
CLEAN_FILES += $(DEPENDENCIES)
