# ping\_pong

2 SCXML processes, [ping](ping.scxml) and [pong](pong.scxml), 
interact with each other by exchanging _ping_ and _pong_ events via MQTT.  
Specifically,
starting from a _ping ({"count" : n})_ event arriving at [ping](ping.scxml),
the 2 processes exchange the following events.

- _ping ({"count" : n})_
- _pong ({"count" : n - 1})_
- ...
- _ping ({"count" : 0})_ or _pong ({"count" : 0})_ depending on the parity of _n_
- _quit_

## statechart: [ping](ping.scxml)

Upon receiving a _ping ({"count" : n})_ event,
[ping](ping.scxml) examines whether _n_ is equal to 0 or greater.

- case: _n_ = 0

  [ping](ping.scxml) terminates, after transmitting a _quit_ event to [pong](pong.scxml)
  as follows

  ```
  SCXML.send ({event: {name: "quit", data: ...}, topic: "pong"})
  ```

- otherwise (_n_ > 0)

  [ping](ping.scxml) transmits a _pong ({"count": n - 1})_ event to [pong](pong.scxml)
  as follows

  ```
  SCXML.send ({event: {name: "pong", data: {count: (n - 1)}, topic: "pong"})
  ```

<details>
  <summary>ping</summary>
  <div><img alt="statechart" src="ping.svg?sanitize=true"/></div>
</details>

## statechart: [pong](pong.scxml)

the behavior of [pong](pong.scxml) is almost identical with its companion,
except for the following.

- it receives _pong_ and emits _ping_.
- it does not expect that the first event it receives is always _pong_.
  (instead, it could be _quit_)


<details>
  <summary>pong</summary>
  <div><img alt="statechart" src="pong.svg?sanitize=true"/></div>
</details>


## composition of ping\_pong and its execution

a small bash-script, [ping\_pong\_mqtt.sh](ping_pong_mqtt.sh), does invoke the following 3 processes in parallel.

- scxmlrun process for [ping](ping.scxml)
- scxmlrun process for [pong](pong.scxml)
- [mosquitto\_pub](https://github.com/eclipse/mosquitto) process for transmitting an initial _ping({"count": n})_ event

to run this script, try:

```
$ echo '{"event": {"name": "ping", "data": {"count": 2}}}' | bash ping_pong_mqtt.sh  
[ping] 2  
[pong] 1  
[ping] 0  
```

or more simply, 

```
$ shelltest ping_pong_mqtt.conf
```

as a remark, for monitoring progress,
it is convenient to run another [mosquitto\_sub](https://github.com/eclipse/mosquitto) process on a separate terminal as follows.

```
$ mosquitto_sub -t ping -t pong | jq -c '.event.name, .event.data?'  
"ping"  
{"count":2}  
"pong"  
{"count":1}  
"ping"  
{"count":0}  
"quit"  
{"die_alone":1}
```


## statechart: [monitor](monitor.scxml)

this statechart is for ensuring that each ping\_pong event sequence between the 2 processes
always starts with _ping_ and ends with _pong_.

- as a direct consequence,
  _n_ that appears in the initial _ping ({"count": n})_ needs to be an odd number.
- the monitor raises an error when execution violates the requirement.

<details>
  <summary>monitor</summary>
  <div><img alt="statechart" src="monitor.svg?sanitize=true"/></div>
</details>

## combining [monitor](monitor.scxml) with ping\_pong

(_to be filled in_)
