FROM ubuntu:latest
LABEL maintainer="jens@x-ion.de"
ADD . /opt/stress3
RUN apt update && apt upgrade -y && apt install -y build-essential git libcurl4-gnutls-dev libxml2-dev libssl-dev gdb cmake nvi && cd /opt && git clone https://github.com/jbeder/yaml-cpp && mkdir /opt/yaml-cpp/build && cd /opt/yaml-cpp/build && cmake .. && make && cd /opt && git clone https://github.com/bji/libs3 && cd /opt/libs3 && patch -p1 < /opt/stress3/libs3.patch && make deb && dpkg -i build/pkg/* && cd /opt/stress3 && make && apt purge -y build-essential git gdb cmake nvi && apt -y autoclean && apt autoremove -y
ENV PATH=/opt/stress3:/bin:/usr/bin:/sbin:/usr/sbin
CMD /bin/bash
