FROM ubuntu:latest
LABEL maintainer="jens@x-ion.de"
ADD . /opt/stress3
RUN apt update && apt upgrade -y && apt install -y build-essential git libcurl4-gnutls-dev libxml2-dev libssl-dev gdb cmake nvi
RUN cd /opt && git clone https://github.com/jbeder/yaml-cpp
RUN mkdir /opt/yaml-cpp/build
RUN cd /opt/yaml-cpp/build && cmake .. && make
RUN cd /opt && git clone https://github.com/bji/libs3
RUN cd /opt/libs3 && patch -p1 < /opt/stress3/libs3.patch && make deb && dpkg -i build/pkg/*
RUN cd /opt/stress3 && make
ENV PATH=/opt/stress3:/bin:/usr/bin:/sbin:/usr/sbin
CMD /bin/bash
