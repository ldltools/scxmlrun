# Summary

scxmlrun is a stand-alone [SCXML](https://www.w3.org/TR/scxml/) processor
written in C++
with the following features:

- it is built on top of [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)  
- it can be used as a command-line program that simply reads from and writes to local files.
- input/output events can also be received/transmitted over the network via the [MQTT](https://mqtt.org/) protocol.  
- several JavaScript functions, including `SCXML.raise` and `SCXML.send` for raising/sending events in the JSON format, are introduced for interfacing with the underlying SCXML engine.

# Example: [echo](examples/echo/README.md)

The following statechart, which we here call "echo.scxml", receives a single _echo_ event, prints out its _data_ parameter onto the console, and then terminates.

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
$ scxmlrun echo.scxml --sub echo &  
$ mosquitto_pub -t echo -m '{"event":{"name":"echo","data":"world"}}'  
world
```

To see how SCXML processes interact with each other via MQTT,
take a look at the [ping\_pong](examples/ping_pong/README.md) example.  

To browse examples,
check out [this list](examples/README.md).  
For the usage of _scxmlrun_,
see [the man page](https://ldltools.github.io/docs/man/scxmlrun.html)
which will be accessible through `man scxmlrun` after installation.

# Installation on Docker

- run `make docker-build` to build a new docker image for scxmlrun
- run `make docker-run` to spawn a container process and enter into it
- (in the container, try `make -C /root/tests test`)

# Installation on Debian/Ubuntu

## Prerequisites

- [Qt5](http://doc.qt.io/qt-5/) libraries including:
  libQt5Core, libQt5Qml, and libQt5Network  

  run: `apt-get install qt5-default qtbase5-dev qtbase5-private-dev`  
  run: `apt-get install qtdeclarative5-dev qtdeclarative5-private-dev`  

  Note: as of Jul 2020, the packages for Ubuntu 20.04 are based on Qt v5.12.8.

- [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html): SCXML processor  
  run: `apt-get install libqt5scxml5-dev`

- Extra header files for hacking the QtSCXML engine

  download/expand the corresponding version of the QtSCXML source package  
  run: `wget https://github.com/qt/qtscxml/archive/v5.12.8.tar.gz; tar xzf v5.12.8.tar.gz`

  copy those header files into the system QtSCXML direcotry  
  run: `mkdir -p ${QTDIR}/QtScxml/private; cp qtscxml-5.12.8/src/scxml/*.h ${QTDIR}/QtScxml/private`  
  where `QTDIR` refers to the system-wide directory for the Qt5 header files
  (`/usr/include/x86_64-linux-gnu/qt5` on ubuntu 20.04)

- [Mosquitto](https://mosquitto.org): MQTT broker and development library  
  run: `apt-get install mosquitto mosquitto-clients libmosquitto-dev`  

- [JSON for C++](https://github.com/nlohmann/json): JSON parser/serializer  
  run: `apt-get install nlohmann-json3-dev`

## Build
- run: `make && make install` in the top directory.

## Testing

Once installation is done, try the following.

- run: `make -C ./tests test`

- run: `make -C ./tests test-mqtt`

  Note: you may need to install [shelltest](https://github.com/simonmichael/shelltestrunner) (by running `apt-get install shelltestrunner`).

# Installation on macOS/Darwin

Refer to [this note](docs/macos.md).

# Remarks

## replacing the underlying SCXML engine

Although
[QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html)
is assumed as the default SCXML engine of scxmlrun,
it is not difficult to switch to another one.

For instance,
[uSCXML](https://github.com/tklab-tud/uscxml) can be used
instead of QtSCXML.  
To try this option, see [this note](docs/uScxml.md).

