FROM fedora:45

RUN mkdir -p /opt/built

RUN dnf -y update







# build and install openstatistic dependency
RUN dnf -y install git gcc-c++ make autoconf automake libtool rpm-build python3-setuptools python3-devel protobuf-devel

RUN git clone https://github.com/adjacentlink/openstatistic -b develop
WORKDIR openstatistic
RUN ./autogen.sh && ./configure && make rpm
RUN cp $(find .rpmbuild/RPMS -name "*\.rpm") /opt/built
RUN dnf -y install /opt/built/*

# build and install simactivityservice
RUN dnf -y install libxml2-devel libuuid-devel zeromq-devel

RUN git clone https://github.com/adjacentlink/simactivityservice -b develop
WORKDIR simactivityservice
RUN ./autogen.sh && ./configure && make rpm
RUN cp $(find .rpmbuild/RPMS -name "*\.rpm") /opt/built
RUN dnf -y install /opt/built/*

RUN dnf clean all

RUN echo 'complete'

