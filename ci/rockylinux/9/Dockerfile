FROM rockylinux:9

RUN mkdir -p /opt/built

RUN dnf -y update
RUN dnf -y install epel-release dnf-plugins-core
RUN dnf config-manager --enable crb
RUN dnf clean all







# build and install openstatistic dependency
RUN dnf -y install git gcc-c++ make autoconf automake libtool rpm-build python3-setuptools python3-devel protobuf-devel

RUN cd /opt && git clone https://github.com/adjacentlink/openstatistic -b develop
WORKDIR /opt/openstatistic
RUN ./autogen.sh && ./configure && make rpm
RUN cp $(find .rpmbuild/RPMS -name "*\.rpm") /opt/built
RUN dnf -y install /opt/built/*

# build and install simactivityservice
RUN dnf -y install libxml2-devel libuuid-devel zeromq-devel

RUN cd /opt && git clone https://github.com/adjacentlink/simactivityservice -b develop
WORKDIR /opt/simactivityservice
RUN ./autogen.sh && ./configure && make rpm
RUN cp $(find .rpmbuild/RPMS -name "*\.rpm") /opt/built
RUN dnf -y install /opt/built/*

RUN dnf clean all

RUN echo 'complete'

