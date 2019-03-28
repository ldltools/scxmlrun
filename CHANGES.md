# v0.12.0rc

- **new** [scxmlrun-all](tools/runall):
  run several SCXML processes (along with generic MQTT processes) in parallel.

- add `*_mqtt.conf` for testing composite SCXML processes connected via MQTT.
- update/adjust examples

# v0.11.0 (2018-12-27)

- **new** introduce _"event repeater"_ (activated by: `scxmlrun -r`) which relays incoming events to elsewhere.

- add/update examples
- add `*.conf` files for unit-testing using [shelltest](https://github.com/simonmichael/shelltestrunner)
- add man page (for `man scxmlrun`)

# v0.10.0 (2018-11-08)

- **new** introduce JS functions: `SCXML.raise` and `SCXML.send`

- add examples: microwave, calc (with web ui), traffic-control, and blackjack
- add examples: voting and vulnerable

# v0.9.0 (2018-10-06)

initial public release
