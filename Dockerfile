FROM ubuntu:18.04 as builder
MAINTAINER LDL Tools development team <ldltools@outlook.com>

ENV DEBIAN_FRONTEND noninteractive
ENV DEBIAN_PRIORITY critical
ENV DEBCONF_NOWARNINGS yes

RUN echo "dash dash/sh boolean false" | debconf-set-selections;\
    dpkg-reconfigure -f noninteractive dash;\
    echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local-lib.conf
RUN apt-get update;\
    apt-get install -y build-essential g++ flex bison gawk file rsync wget cmake

# Qt5
RUN apt-get install -y qt5-default qtbase5-dev qtbase5-private-dev;\
    apt-get install -y qtdeclarative5-dev qtdeclarative5-private-dev

# QtSCXML
WORKDIR /root
RUN version=5.9.5;\
    wget https://github.com/qt/qtscxml/archive/v${version}.tar.gz;\
    tar xzf v${version}.tar.gz;\
    cd qtscxml-${version}/src/scxml; qmake;\
    incdir=/usr/include/x86_64-linux-gnu/qt5/QtScxml;\
    mkdir -p ${incdir}/private; cp -p *.h $incdir; cp -p *.h ${incdir}/private;\
    mkdir -p private; cp -p *.h private;\
    make && make install;\
    echo "#include \"qscxmlstatemachine.h\"" > ${incdir}/QScxmlStateMachine;\
    echo "#include \"qscxmlevent.h\"" > ${incdir}/QScxmlEvent;\
    echo "#include \"qscxmlecmascriptdatamodel.h\"" > ${incdir}/QScxmlEcmaScriptDataModel

# JSON
RUN apt-get install -y nlohmann-json-dev

# MQTT
RUN apt-get install -y mosquitto mosquitto-clients libmosquitto-dev

# SCXMLRUN
ADD . /root/scxmlrun
WORKDIR /root/scxmlrun
RUN make && make install

#
WORKDIR /root
CMD ["/bin/bash"]
