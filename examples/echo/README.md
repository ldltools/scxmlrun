# echo

## statechart: `echo.scxml`

The [statechart](echo.scxml) receives one or more _echo_ events and prints out the event parameter onto the console.
It terminates when _\_accept_ event is received.  
See also [README.md](https://github.com/ldltools/dsl4sc/examples/echo/README.md)
in the dsl4sc repository.

![statechart](echo.svg)

## scenario: `echo.in`

```
{"event" : {"name":"echo", "data":"hello"}}  
{"event" : {"name":"echo", "data":"world"}}  
{"event" : {"name":"_accept"}}
```

## testing the statechart

```
$ scxmlrun echo.scxml echo.in  
hello  
world
```
