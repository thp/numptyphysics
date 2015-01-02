install_freedesktop:
	install -D -m 644 platform/freedesktop/$(APP).desktop $(DESTDIR)/$(PREFIX)/share/applications/$(APP).desktop
	install -D -m 644 platform/freedesktop/$(APP).png $(DESTDIR)/$(PREFIX)/share/icons/hicolor/256x256/apps/$(APP).png
	install -D -m 644 platform/freedesktop/$(APP).6 $(DESTDIR)/$(PREFIX)/share/man/man6/$(APP).6
ADDITIONAL_INSTALL_TARGETS += install_freedesktop
.PHONY: install_freedesktop
