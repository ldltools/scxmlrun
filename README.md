# Summary

scxmlrun is a stand-alone [SCXML](https://www.w3.org/TR/scxml/) processor
written in C++
with the following features:

- it is built on top of [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)  
- it can be used as a command-line program that simply reads from and writes to local files.
- input/output events can also be received/transmitted over the network via the [MQTT](https://mqtt.org/) protocol.  
- several JavaScript functions, including `SCXML.raise` and `SCXML.send` for raising/sending events in the JSON format, are introduced for interfacing with the underlying SCXML engine.

# Example: [echo](examples/echo/README.md)

The following statechart receives a single _echo_ event, prints out its _\_data_ parameter onto the console, and then terminates.

```
<scxml xmlns="http://www.w3.org/2005/07/scxml" version="1.0" datamodel="ecmascript" initial="q1">  
  <state id="q1">  
    <transition target="q2" event="echo"><script>console.log (_event.data)</script></transition>  
  </state>  
  <final id="q2"/>  
</scxml>
```

## Reading event(s) from a local file

```
$ echo '{"event":{"name":"echo","data":"hello"}}' | scxmlrun echo.scxml  
hello
```

## Reading event(s) via MQTT

```
$ scxmlrun echo.scxml --sub echo  
$ mosquitto_pub -t echo -m '{"event":{"name":"echo","data":"world"}}'  
world
```

Check out [more examples](examples/README.md) if you are interested.  
For the usage of _scxmlrun_, see [its man page](docs/man/scxmlrun.man)
which will be accessible through `man scxmlrun` after installation.

# Installation on Docker

- run `make docker-build` to build a new docker image for scxmlrun
- run `make docker-run` to enter into the image

# Installation on Debian/Ubuntu

## Prerequisites

- [Qt5](http://doc.qt.io/qt-5/) libraries including:
  libQt5Core, libQt5Qml, and libQt5Network  

  run: `apt-get install qt5-default qtbase5-dev qtbase5-private-dev`  
  run: `apt-get install qtdeclarative5-dev qtdeclarative5-private-dev`  

  Note: as of Sep 2018, the packages for Ubuntu are based on Qt v5.9.5.

- [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)  
  download the corresponding version of the QtSCXML source package  
  run: `wget https://github.com/qt/qtscxml/archive/v5.9.5.tar.gz`

  expand the archive, then build and install QtSCXML  
  run: `cd qtscxml-5.9.5/src/scxml; qmake && make && make install`

- [Mosquitto](https://mosquitto.org): MQTT broker and development library  
  run: `apt-get install mosquitto mosquitto-clients libmosquitto-dev`  

- [JSON for C++](https://github.com/nlohmann/json): JSON parser/serializer  
  run: `apt-get install nlohmann-json-dev`

## Build
- run: `make && make install` in this directory  

## Remark

Instead of [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html),
[uSCXML](https://github.com/tklab-tud/uscxml) can be used
as the SCXML engine.  
To try this option, see [this note](docs/uScxml.md).

# Installation on macOS/Darwin

Refer to [this note](docs/macos.md).
