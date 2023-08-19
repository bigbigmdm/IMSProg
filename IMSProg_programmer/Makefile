PKG=ch341prog

prefix = /usr/local
bindir = $(prefix)/bin
sharedir = $(prefix)/share
mandir = $(sharedir)/man
man1dir = $(mandir)/man1

CFLAGS=-std=gnu99 -Wall
$(PKG): main.c ch341a.c ch341a.h
	gcc $(CFLAGS) ch341a.c main.c -o $(PKG)  -lusb-1.0
clean:
	rm *.o $(PKG) -f
install-udev-rule:
	cp 99-ch341a-prog.rules /etc/udev/rules.d/
	udevadm control --reload-rules
.PHONY: clean install-udev-rule

install: $(PKG)
	install $(PKG) $(DESTDIR)$(bindir)
	install -m 0664 99-ch341a-prog.rules $(DESTDIR)/etc/udev/rules.d/99-ch341a-prog.rules

debian/changelog:
	#gbp dch --debian-tag='%(version)s' -S -a --ignore-branch -N '$(VERSION)'
	dch --create -v 1.0-1 --package $(PKG)

deb:
	dpkg-buildpackage -b -us -uc
.PHONY: install debian/changelog deb
