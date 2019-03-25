all::
	@echo "usage: make run | run-mqtt"

clean::
	rm -f *~

run:
	shelltest ping_pong.conf

run-mqtt:
	shelltest ping_pong_mqtt.conf

ping_pong_mqtt.sh:	ping_pong_mqtt.json
	scxmlrun-all --no-exec $< -o $@
