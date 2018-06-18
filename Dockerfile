FROM ubuntu:bionic
LABEL maintainer="j.harbott@x-ion.de"
ADD . /opt/stress3
RUN apt update && apt upgrade -y && apt install -y build-essential git libcurl4-gnutls-dev libxml2-dev libssl-dev cmake && cd /opt && git clone https://github.com/bji/libs3 && cd /opt/libs3 && patch -p1 < /opt/stress3/libs3.patch && make deb && dpkg -i build/pkg/* && cd /opt/stress3 && make && apt purge -y build-essential git cmake && apt -y autoclean && apt autoremove -y
ENV PATH=/opt/stress3/bin:/bin:/usr/bin:/sbin:/usr/sbin
CMD /bin/bash
