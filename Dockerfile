FROM debian:bullseye-slim as builder
#FROM ubuntu:20.04 as builder
MAINTAINER LDL Tools development team <ldltools@outlook.com>

ENV DEBIAN_FRONTEND noninteractive
ENV DEBIAN_PRIORITY critical
ENV DEBCONF_NOWARNINGS yes

RUN echo "dash dash/sh boolean false" | debconf-set-selections;\
    dpkg-reconfigure -f noninteractive dash;\
    echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local-lib.conf
RUN apt-get update;\
    apt-get install -y build-essential g++ flex bison gawk jq shelltestrunner;\
    apt-get install -y file rsync wget net-tools nmap

# Qt5/QtScxml
RUN apt-get install -y qtbase5-dev qtbase5-private-dev;\
    apt-get install -y qtdeclarative5-dev qtdeclarative5-private-dev;\
    apt-get install -y libqt5scxml5-dev

# QtSCXML -- add extra header files
WORKDIR /root
RUN QTVERSION=$(gawk '/QTCORE_VERSION_STR/{print($3)}' /usr/include/x86_64-linux-gnu/qt5/QtCore/qtcoreversion.h | sed 's/"//g') && \
    wget https://github.com/qt/qtscxml/archive/refs/tags/v${QTVERSION}.tar.gz && \
    tar xzf v${QTVERSION}.tar.gz;\
    dir=/usr/include/x86_64-linux-gnu/qt5/QtScxml/private; mkdir -p $dir; cp -p qtscxml-${QTVERSION}/src/scxml/*.h $dir

# qtscxml archive (for copying to the final image)
RUN cd /usr/lib/x86_64-linux-gnu; tar cvf /tmp/libqtscxml.tar libQt5Scxml.so*

# JSON
RUN apt-get install -y nlohmann-json3-dev

# MQTT
RUN apt-get install -y mosquitto mosquitto-clients libmosquitto-dev

# SCXMLRUN
ADD . /root/scxmlrun
WORKDIR /root/scxmlrun
RUN make && PREFIX=/usr/local make install

# MQTT w. the WebSocket-bridge functionality activated
RUN dir=/etc/mosquitto/conf.d; conf=${dir}/websockets.conf;\
    test -d $dir -a ! -f $conf &&\
    echo -e "listener 1883\nlistener 9001\nprotocol websockets" > $conf
#    service mosquitto restart

# MQTT/Websocket ports
EXPOSE 1883
EXPOSE 9001

#
WORKDIR /root
CMD ["/bin/bash"]

# ====================
# final image
# ====================
FROM debian:bullseye-slim
#FROM ubuntu:20.04

RUN echo "dash dash/sh boolean false" | debconf-set-selections;\
    dpkg-reconfigure -f noninteractive dash;\
    echo "/usr/local/lib" > /etc/ld.so.conf.d/usr-local-lib.conf;\
    apt-get update

# Qt5
RUN apt-get install -y libqt5core5a libqt5network5 libqt5qml5

# QtSCXML
WORKDIR /usr/lib/x86_64-linux-gnu/
COPY --from=builder /tmp/libqtscxml.tar .
RUN tar xf libqtscxml.tar
#RUN QTVERSION=$(gawk '/QTCORE_VERSION_STR/{print($$3)}' /usr/include/x86_64-linux-gnu/qt5/QtCore/qtcoreversion.h | sed 's/"//g');\
#    ln -s libQt5Scxml.so.5.9.5 libQt5Scxml.so.5.9;\
#    ln -s libQt5Scxml.so.5.9.5 libQt5Scxml.so.5;\
#    ln -s libQt5Scxml.so.5.9.5 libQt5Scxml.so

# MQTT
RUN apt-get install -y mosquitto mosquitto-clients;\
    dir=/etc/mosquitto/conf.d; conf=${dir}/websockets.conf;\
    test -d $dir -a ! -f $conf &&\
    echo -e "listener 1883\nlistener 9001\nprotocol websockets" > $conf

# SCXMLRUN
COPY --from=builder /usr/local /usr/local
RUN ldconfig

# examples & tests
ADD examples /root/examples
ADD tests /root/tests
RUN apt-get install -y make gawk jq shelltestrunner

WORKDIR /root
CMD ["/bin/bash"]

# MQTT/Websocket ports
EXPOSE 1883 9001
