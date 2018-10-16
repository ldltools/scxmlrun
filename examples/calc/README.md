# calc

The [SCXML definition](calc.scxml) originates from [this](https://www.w3.org/TR/scxml/#N11630).

<details>
<summary>calc statechart</summary>
<div><img src="calc.svg?sanitize=true"/></div>
</details>

## scenario: `calc.in` for computing 12 + 34 = 46

The following input events are included in [calc.in](calc.in).

```
{"event" : {"name" : "DIGIT.1"}}  
{"event" : {"name" : "DIGIT.2"}}  
{"event" : {"name" : "OPER.PLUS"}}  
{"event" : {"name" : "DIGIT.3"}}  
{"event" : {"name" : "DIGIT.4"}}  
{"event" : {"name" : "EQUALS"}}  
{"event" : {"name" : "terminate"}}
```

## running the statechart against the scenario

```
$ scxmlrun calc.scxml calc.in -o /dev/null  
result: 0  
result: 0  
result: 1  
result: 12  
result: 12  
result: 3  
result: 34  
result: 46
```

## web ui for calc

You can also try a [web ui](calc_ui.html) for the calc statemachine
that runs on your browser.

<div><img src="calc_ui.jpg" width="320"/></div>

First, you need to configure mosquitto to work as a bridge between MQTT and Websocket.  
Follow [this instruction](../../docs/websocket.md).

Once it is done, invoke the ui by running `runtest.sh`.  
The fancy calculator shown above should pop up.

```
$ ./runtest.sh  

calc.scxml has been invoked successfully  
OPEN: "file:/.../calc_ui.html?host=127.0.0.1&port=9001&sub=calc477855293&pub=calcui477855293"  

$ firefox --new-tab "file:/.../calc_ui.html?host=127.0.0.1&port=9001&sub=calc477855293&pub=calcui477855293"
```
