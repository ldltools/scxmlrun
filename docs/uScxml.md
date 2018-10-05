
## Prerequisites for using uSCXML as the SCXML engine

Instead of [QtSCXML](https://doc.qt.io/qt-5/qtscxml-overview.html),
[uSCXML](https://github.com/tklab-tud/uscxml) can be used
as the SCXML engine.
In that case, the following packages need to be installed.

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

- run: `PROCESSOR=uscxml make && make install` in this directory  
