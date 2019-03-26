FROM ubuntu:18.04 as builder
MAINTAINER LDL Tools development team <ldltools@outlook.com>

ENV DEBIAN_FRONTEND noninteractive
ENV DEBIAN_PRIORITY critical
ENV DEBCONF_NOWARNINGS yes

RUN echo "dash dash/sh boolean false" | debconf-set-selections;\
    dpkg-reconfigure -f noninteractive dash;\
    echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local-lib.conf
RUN apt-get update;\
    apt-get install -y build-essential g++ flex bison gawk jq;\
    apt-get install -y file rsync wget net-tools nmap

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
    mkdir -p ${incdir}/private; cp -p *.h $incdir; cp ${incdir}/*.h ${incdir}/private;\
    ln -s . private;\
    make && make install;\
    echo "#include \"qscxmlstatemachine.h\"" > ${incdir}/QScxmlStateMachine;\
    echo "#include \"qscxmlevent.h\"" > ${incdir}/QScxmlEvent;\
    echo "#include \"qscxmlecmascriptdatamodel.h\"" > ${incdir}/QScxmlEcmaScriptDataModel

# JSON
RUN apt-get install -y nlohmann-json-dev

# MQTT w. the WebSocket-bridge functionality activated
RUN apt-get install -y mosquitto mosquitto-clients libmosquitto-dev;\
    dir=/etc/mosquitto/conf.d; conf=${dir}/websockets.conf;\
    test -d $dir -a ! -f $conf &&\
    echo -e "listener 1883\nlistener 9001\nprotocol websockets" > $conf &&\
    service mosquitto restart

# SCXMLRUN
ADD . /root/scxmlrun
WORKDIR /root/scxmlrun
RUN make && PREFIX=/usr/local make install

#
WORKDIR /root
CMD ["/bin/bash"]

# MQTT/Websocket ports
EXPOSE 1883
EXPOSE 9001

# ====================
# final image
# ====================
FROM ubuntu:18.04

RUN echo "dash dash/sh boolean false" | debconf-set-selections;\
    dpkg-reconfigure -f noninteractive dash;\
    echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local-lib.conf;\
    apt-get update

# Qt5
RUN apt-get install -y libqt5core5a libqt5network5 libqt5qml5

# QtSCXML
WORKDIR /usr/lib/x86_64-linux-gnu/
COPY --from=builder /usr/lib/x86_64-linux-gnu/libQt5Scxml.so.5.9.5 .
RUN ln -s libQt5Scxml.so.5.9.5 libQt5Scxml.so.5.9;\
    ln -s libQt5Scxml.so.5.9.5 libQt5Scxml.so.5;\
    ln -s libQt5Scxml.so.5.9.5 libQt5Scxml.so

# MQTT
RUN apt-get install -y mosquitto mosquitto-clients;\
    dir=/etc/mosquitto/conf.d; conf=${dir}/websockets.conf;\
    test -d $dir -a ! -f $conf &&\
    echo -e "listener 1883\nlistener 9001\nprotocol websockets" > $conf &&\
    service mosquitto start

# SCXMLRUN
WORKDIR /usr/local
COPY --from=builder /usr/local .
RUN ldconfig

WORKDIR /root
CMD ["/bin/bash"]

# MQTT/Websocket ports
EXPOSE 1883
EXPOSE 9001
