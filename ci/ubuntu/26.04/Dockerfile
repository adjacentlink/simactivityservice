FROM ubuntu:26.04

RUN mkdir -p /opt/built

# prevent failures due to interactive apt transactions
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update
RUN apt-get -y upgrade --no-install-recommends







# Build and install openstatistic dependency
RUN apt-get -y install git gcc g++ autoconf automake libtool dh-python debhelper pkg-config python3-setuptools \
                       protobuf-compiler libprotobuf-dev python3-dev

RUN cd /opt && git clone https://github.com/adjacentlink/openstatistic -b develop
WORKDIR /opt/openstatistic
RUN ./autogen.sh && ./configure && make deb
RUN cp $(find .debbuild -name "*\.deb") /opt/built
RUN dpkg -i /opt/built/*; apt-get -y install -f

# build and install simactivityservice
RUN apt-get -y install libxml2-dev uuid-dev libzmq3-dev

RUN cd /opt && git clone https://github.com/adjacentlink/simactivityservice -b develop
WORKDIR /opt/simactivityservice
RUN ./autogen.sh && ./configure && make deb
RUN cp $(find .debbuild -name "*\.deb") /opt/built
RUN dpkg -i /opt/built/*\.deb; apt-get -y install -f

RUN echo 'complete'

