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
