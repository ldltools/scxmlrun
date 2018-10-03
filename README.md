# Summary

scxmlrun is a stand-alone [SCXML](https://www.w3.org/TR/scxml/) processor
written in C++
with the following features:

- it is built on top of [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)  
  alternatively, [uSCXML](https://github.com/tklab-tud/uscxml) can be used as the underlying SCXML engine.
- it can be used as a command-line program that reads from and writes to local files.
- input/output events can be transmitted over the network via the [MQTT](https://mqtt.org/) protocol.  
- the underlying [JavaScript engine](http://doc.qt.io/qt-5/qjsengine.html) is extended with several new built-in primitives including:
  \_\__raiseEvent_ (_event_, _data_) and \_\__sendEvent_ (_event_, _data_)

# Example: [ping\_pong](examples/ping_pong/README.md)

(1) [_ping.sh_](examples/ping_pong/ping.sh) and [_pong.sh_](examples/ping_pong/pong.sh) interact with each other by emitting _ping_ and _pong_ events via MQTT.  
They start communication when an initial _ping (0)_ event is received by _ping.sh_,
upon which _ping.sh_ emits _pong (1)_.  
Then, it is is delivered to _pong.sh_ and responded with _ping (1)_,
and interaction goes on that way.

(2) [_ping\_pong.scxml_](examples/ping_pong/ping_pong.scxml) is generated
from [_ping\_pong.rules_](examples/ping_pong/ping_pong.rules)
that is defined in *dsl4sc*.

(3) When running these together,
[*ping\_pong.scxml*](examples/ping_pong/ping_pong.scxml) works as a _monitor_
for _ping.sh_ and _pong.sh_.
It monitors all events exchanged between _ping.sh_ and _pong.sh_,
and reports them on the console.

```
$ ./runtest.sh
ping 0  
pong 1  
ping 1  
pong 2  
ping 2  
pong 3  
...
```

![ping\_pong](examples/ping_pong/ping_pong.jpg)

See [more examples](examples/README.md) if you are interested.

# Installation on Docker
(to be filled in)

# Installation on Debian/Ubuntu

## Prerequisites

- [Qt5](http://doc.qt.io/qt-5/) libraries including:
  libQt5Core, libQt5Qml, and libQt5Network  

  run: `apt-get install qt5-default qtbase5-dev qtbase5-private-dev`  
  run: `apt-get install qtdeclarative5-dev qtdeclarative5-private-dev`  

  Note: as of Sep 2018, the packages are based on Qt v5.9.5.

- [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)  
  download the corresponding version of the QtSCXML source package  
  run: `wget https://github.com/qt/qtscxml/archive/v5.9.5.tar.gz`

  expand the archive, then build and install QtSCXML  
  run: `cd qtscxml-5.9.5; mkdir _build; cd _build; cmake .. && make && make install`

- [JSON for C++](https://github.com/nlohmann/json): JSON parser/serializer  
  run: `apt-get install nlohmann-json-dev`

- [mosquitto](https://mosquitto.org): MQTT broker and development library  
  run: `apt-get install mosquitto libmosquitto-dev`  

## Prerequisites for using uSCXML as the SCXML engine

- [uscxml](http://tklab-tud.github.io/uscxml): another SCXML processor developed at
[Technische Universität Darmstadt](https://www.informatik.tu-darmstadt.de/telekooperation/telecooperation_group/)

  1. download from github  
     run: `git clone https://github.com/tklab-tud/uscxml`
  1. install the peripheral library packages listed below, if necessary
  1. build and install  
     run: `cd uscxml; mkdir _build; cd _build; cmake .. && make && make install`

  refer also to [the detailed instruction](https://github.com/tklab-tud/uscxml/blob/master/docs/BUILDING.md)

- Peripheral libraries for uscxml  
  run: `apt-get install libevent-dev libxerces-c-dev libssl-dev libcurl4-openssl-dev liburiparser-dev`

- [JavaScriptCore](https://developer.apple.com/documentation/javascriptcore): JavaScript processor developed by Apple  
  run: `apt-get install libjavascriptcoregtk-4.0-dev`  
  (alt: `apt-get install libv8-3.14.5 libv8-dev` to use google v8)

## Build
- run: `make && make install` in this directory  

In case of using uSCXML

- run: `PROCESSOR=uscxml make && make install` in this directory  
