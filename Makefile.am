ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = simactivityservice.pc

SUBDIRS= src \
         include

cleantar:
	@(rm -f simactivityservice*.tar.gz)


EXTRA_DIST = \
 COPYING \
 Copyright \
 autogen.sh \
 simactivityservice.spec.in

clean-local: cleantar
	-rm -f $(PACKAGE)-*.tar.gz
	-rm -rf .rpmbuild
	-rm -rf .debbuild
	-rm -rf $(PACKAGE).spec

edit = sed \
       -e 's|@VERSION[@]|$(VERSION)|g'

$(PACKAGE).spec: $(PACKAGE).spec.in
	if test -f $@; then chmod u+w $@; fi
	$(edit) $< > $@
	chmod g-w,u-w $@

if HAVE_RPMBUILD

rpm: $(PACKAGE).spec dist-gzip
	mkdir -p .rpmbuild/BUILD \
	.rpmbuild/SPECS \
	.rpmbuild/SOURCES \
	.rpmbuild/SRPMS \
	.rpmbuild/RPMS/noarch \
	.rpmbuild/tmp
	cp -f $(PACKAGE)-$(VERSION).tar.gz .rpmbuild/SOURCES
	rpmbuild --clean -ba $(top_srcdir)/$(PACKAGE).spec \
    --define "_topdir $$PWD/.rpmbuild" \
    --define "_tmppath $$PWD/.rpmbuild/tmp" \
    $(RPMBUILD_ARGS)
	@echo "============================================================"
	@echo "RPMs located in .rpmbuild/RPMS and .rpmbuild/SRPMS"

endif

if HAVE_DEB

deb: dist-gzip
	rm -rf .debbuild
	mkdir -p .debbuild
	cp -f $(PACKAGE)-$(VERSION).tar.gz .debbuild/$(PACKAGE)_$(VERSION).orig.tar.gz
	cd .debbuild && tar xzf $(PACKAGE)_$(VERSION).orig.tar.gz
	cp -r $(top_srcdir)/debian .debbuild/$(PACKAGE)-$(VERSION)/debian
	sed -e 's|@VERSION[@]|$(VERSION)|g' \
	    -e "s|@DATE[@]|$$(date -R)|g" \
	    .debbuild/$(PACKAGE)-$(VERSION)/debian/changelog.in > \
	    .debbuild/$(PACKAGE)-$(VERSION)/debian/changelog
	rm -f .debbuild/$(PACKAGE)-$(VERSION)/debian/changelog.in
	cd .debbuild/$(PACKAGE)-$(VERSION) && \
	dpkg-buildpackage -us -uc -b $(DEBBUILD_ARGS)
	@echo "============================================================"
	@echo "debs located in .debbuild"

endif
