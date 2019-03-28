# generated from "echo4.json" by scxmlrun-all at 2019-03-23T04:32:49
# (https://github.com/ldltools/scxmlrun/tree/master/tools/)
# preamble
set -eu -o pipefail
USER=${USER:-$(id -u -n)}
test $(pgrep -c -u $USER scxmlrun) -eq 0 || { echo "scxml running"; exit 1; }
# run: 2 processes
# (const(../../../examples/echo/echo.scxml))[id = 1, input = [mqtt_topic = ["echo"], protocol = mqtt], type = scxml, output = [protocol = mqtt], protocol = mqtt]
{ scxmlrun ../../../examples/echo/echo.scxml --sub echo --mqtt; } &
# (const(sleep 1s; mosquitto_pub -t echo -l))[id = 2, input = [protocol = none, events = [{"event":{"name":"echo","data":"hello"}}, {"event":{"name":"quit"}}]], type = script, output = [protocol = mqtt], protocol = mqtt]
{ echo -ne '{"event":{"name":"echo","data":"hello"}}\n{"event":{"name":"quit"}}\n' | (sleep 1s; mosquitto_pub -t echo -l); }
# postamble
sleep 1s
test $(pgrep -c -u $USER scxmlrun) -eq 0 || { echo "scxml running"; pkill -u $USER scxmlrun; exit 1; }
exit 0
