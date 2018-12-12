# financial accumulator

_Accumulator_ is a _derivative_ product for purchasing shares with leverage
in a continuous and accumulating manner (often on a daily basis)
until either its settlement date is reached or the share price goes up above
some pre-agreed _knock-out_ price.  
If you are not familiar with its concept, be advised to take a look at
[this wikipedia article](https://en.wikipedia.org/wiki/Accumulator\_\(structured_product\)).  

## accumulator in dsl4sc/scxml

A simplified version of _accumulator_ is defined as a statechart as follows.  

- [accumulator in dsl4sc](accumulator.rules)
- [accumulator in scxml](accumulator.scxml) generated from the above dsl4sc definition

  <details>
    <summary>operation flow</summary>
    <div>Upon each <i>day_close</i> event,
      <ul>
      <li>if the share price (carried by <i>day_close</i> as <i>_event.data.price</i>) is high above <i>ko_price</i>,<br>
          perform the knockout (sell-off) operation and terminate (<i>_knockout</i>)
      </li>
      <li>otherwise (<i>_not_knockout</i>),
        <ol>
        <li>purchase shares (<i>_over_strike</i> or <i>_under_strike</i>),
            the amount of which varies depending on the price</li>
	<li>terminate normally if the expiration date is reached, or
	    wait for a <i>day_open</i> event to repeat the entire process</li>
	</ol>
      </li>
      </ul>
    </div>
  </details>

![accumulator](accumulator.svg)

For the detail of the definition (and its monitors listed below),
refer to [this material](accumulator.pdf).

## monitors for accumulator

In addition, several monitors are defined, also as statecharts,
to ensure that the accumulator works exactly as intended.

- [monitor1](monitors/accumulator_mon1.scxml): simple event-pattern checking

  <details>
  <summary>monitor1 statechart</summary>
  <div>
    monitor1 examines whether incoming events match with the following <i>regular pattern</i>:
    <p><i>init; day_close; (day_open; day_close)*; (knockout + terminate)</i></p>
    <div><img src="monitors/accumulator_mon1.svg?sanitize=true"/></div>
  </div>
  </details>

- [monitor2](monitors/accumulator_mon2.scxml): monitor1 + event parameter validation

  <details>
  <summary>monitor2 statechart</summary>
  <div>
    in addition to what monitor1 does,
    monitor2 validates the share price upon <i>knockout</i>.<br>
    if <i>knockout</i> is triggered incorrectly
    (when the price is not higher than <i>ko_price</i>), a runtime error is raised.
    <div><img src="monitors/accumulator_mon2.svg?sanitize=true"/></div>
  </div>
  </details>

- [monitor3](monitors/accumulator_mon3.scxml): safety and liveness

  <details>
  <summary>monitor3 statechart</summary>
  <div>
    <div>safety: <i>it never occurs</i> that
      both knockout termination and normal termination take place in a single run.
    </div>
    <div>liveness: <i>it will always eventually occur</i> that
      either knockout termination or normal termination takes place.
    </div>
    <div><img src="monitors/accumulator_mon3.svg?sanitize=true"/></div>
  </div>
  </details>

- [monitor4](monitors/accumulator_mon4.scxml): anomaly detection, a sort of

  <details>
  <summary>monitor4 statechart</summary>
  <div>
    let us call it an <i>anomaly</i> that the following conditions both hold.
    <ul>
      <li><i>_over_strike</i> never occurs until termination</li>
      <li><i>_under_strike</i> never occurs until termination</li>
    </ul>
    monitor4 detects this anomaly and raises an error.
    <div><img src="monitors/accumulator_mon4.svg?sanitize=true"/></div>
  </div>
  </details>

## scenarios

- valid cases:
  [scenario1](scenarios/scenario1.txt),
  [scenario2](scenarios/scenario2.txt)

- invalid cases:
  [invalid1](scenarios/invalid1.txt),
  [invalid2](scenarios/invalid2.txt),
  [invalid3](scenarios/invalid3.txt),
  [invalid4](scenarios/invalid4.txt)

## running the accumulator statechart and its monitors against valid/invalid scenarios

- to run the accumulator against the valid scenarios

  ```
  $ shelltest accumulator.conf
  ```

  send input events to accumulator via MQTT

  ```
  $ shelltest passthru_accumulator.conf
  ```

- to run the monitors against the all valid/invalid scenarios

  ```
  $ shelltest monitors.conf
  ```

  send input events to monitors via MQTT

  ```
  $ shelltest passthru_monitors.conf
  ```

- to run the accumulator and each monitor connected via MQTT against the scenarios

  ```
  $ shelltest accumulator_monitors.conf
  ```
