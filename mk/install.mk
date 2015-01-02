DESTDIR ?=
PREFIX ?= /usr

install: $(APP)
	mkdir -p $(DESTDIR)/$(PREFIX)/bin
	install -m 755 $(APP) $(DESTDIR)/$(PREFIX)/bin/
	mkdir -p $(DESTDIR)/$(PREFIX)/share/$(APP)
	cp -rpv data $(DESTDIR)/$(PREFIX)/share/$(APP)/

.PHONY: install
