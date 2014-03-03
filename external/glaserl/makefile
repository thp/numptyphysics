TARGET := libglaserl.a

CFLAGS += -g -O2

SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(AR) rcs $@ $^

clean:
	rm -f $(OBJECTS)

distclean: clean
	rm -f $(TARGET)

.PHONY: all clean distclean
.DEFAULT: all
