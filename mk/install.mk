DESTDIR ?=
PREFIX ?= /usr

install: $(APP) $(ADDITIONAL_INSTALL_TARGETS)
	install -D -m 755 $(APP) $(DESTDIR)/$(PREFIX)/bin/$(APP)
	mkdir -p $(DESTDIR)/$(PREFIX)/share/$(APP)
	cp -rpv data $(DESTDIR)/$(PREFIX)/share/$(APP)/

.PHONY: install
