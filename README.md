# Summary

scxmlrun is a stand-alone [SCXML](https://www.w3.org/TR/scxml/) processor
written in C++
with the following features:

- it is built on top of [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)  
  alternatively, [uSCXML](https://github.com/tklab-tud/uscxml) can be used as the underlying SCXML engine.
- it can be used as a command-line program that reads from and writes to local files.
- input/output events can be transmitted over the network via the [MQTT](https://mqtt.org/) protocol.  

# Installation on Docker
(to be filled in)

# Installation on Debian/Ubuntu

## Prerequisites

- [Qt5](http://doc.qt.io/qt-5/) libraries  
  run: `apt-get install qt5-default qtbase5-dev qtbase5-private-dev`  
  run: `apt-get install qtdeclarative5-dev qtdeclarative5-private-dev`  

  Note: as of Sep 2018, the packages are based on Qt v5.9.5.

- [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)  
  download the corresponding version of the QtSCXML source archive  
  run: `wget https://github.com/qt/qtscxml/archive/v5.9.5.tar.gz`

  expand the archive, then build and install QtSCXML  
  run: `cd qtscxml-5.9.5; mkdir _build; cd _build; cmake .. && make && make install`

- [JSON for C++](https://github.com/nlohmann/json): JSON parser/serializer  
  run: `apt-get install nlohmann-json-dev`

- [mosquitto](https://mosquitto.org): MQTT broker and the library  
  run: `apt-get install mosquitto libmosquitto-dev`  

## Prerequisites when using uSCXML as the SCXML engine

- [uscxml](http://tklab-tud.github.io/uscxml): SCXML processor
  1. download from source  
     `git clone https://github.com/tklab-tud/uscxml`
  1. install the library packages listed below, if necessary
  1. build and install  
     `cd uscxml; mkdir build; cd build; cmake ..; make && make install`

  refer also to [the detailed instruction](https://github.com/tklab-tud/uscxml/blob/master/docs/BUILDING.md)

- Peripheral libraries for uscxml  
  run: `apt-get install libevent-dev libxerces-c-dev libssl-dev libcurl4-openssl-dev liburiparser-dev`

- [JavaScriptCore](https://developer.apple.com/documentation/javascriptcore): JavaScript processor for uscxml  
  run: `apt-get install libjavascriptcoregtk-4.0-dev`  
  (alt: `apt-get install libv8-3.14.5 libv8-dev` to use google v8)

## Build
- run `make && make install` in this directory  
