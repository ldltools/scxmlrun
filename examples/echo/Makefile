all::
	@echo "usage: make run | run-mqtt"

clean::
	rm -f *~

run:
#	scxmlrun echo.scxml echo.in
	shelltest echo.conf

# mqtt
run-mqtt:
#	shelltest passthru_echo.conf
	shelltest echo_mqtt.conf

echo_mqtt.sh:	echo_mqtt.json
	scxmlrun-all --no-exec $< -o $@

