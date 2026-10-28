EXTRA_DIST= \
 setup.py.in \
 simactivityservice \
 scripts

BUILT_SOURCES = \
 simactivityservice/interface/simactivityservice_pb2.py \
 simactivityservice/interface/upstreamtime_pb2.py \
 setup.py

all-local:
	$(PYTHON) setup.py build

clean-local: setup.py
	$(PYTHON) setup.py clean
	-rm -f $(BUILT_SOURCES)
	-find . -name "*.pyc" -delete
	-rm -f .installedfiles
	-rm -rf build

sdist:
	$(PYTHON) setup.py sdist

edit = sed \
	-e 's|@VERSION[@]|$(VERSION)|g'

simactivityservice/interface/simactivityservice_pb2.py: ../../include/simactivityservice/proto/simactivityservice.proto
	protoc -I../../include/simactivityservice/proto \
	  --python_out=simactivityservice/interface  $<

simactivityservice/interface/upstreamtime_pb2.py: ../../include/simactivityservice/proto/upstreamtime.proto
	protoc -I../../include/simactivityservice/proto \
	  --python_out=simactivityservice/interface  $<

setup.py:	setup.py.in
	if test -f $@; then chmod u+w $@; fi
	$(edit) $< > $@
	chmod g-w,u-w $@

install-exec-hook: $(BUILT_SOURCES)
	$(PYTHON) setup.py install \
            --single-version-externally-managed \
	    -O1 \
            --record .installedfiles \
            --prefix=$(prefix) \
            --exec-prefix=$(exec_prefix) \
            $(if $(DESTDIR),--root=$(DESTDIR)) \
            $(if $(subst false,,$(HAVE_DEB)),--install-layout=deb)

uninstall-hook:
	if test -f .installedfiles; then xargs -a .installedfiles rm -f; fi
