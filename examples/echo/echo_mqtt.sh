# generated from "echo_mqtt.json" by scxmlrun-all at 2019-03-23T15:32:04
# (https://github.com/ldltools/scxmlrun/tree/master/tools/)
# preamble
set -eu -o pipefail
USER=${USER:-$(id -u -n)}
test $(pgrep -c -u $USER scxmlrun) -eq 0 || { echo "scxml running"; exit 1; }
# run: 2 processes
{ scxmlrun ./echo.scxml --sub echo --mqtt; } &
{ cat /dev/stdin | { sleep 1s; mosquitto_pub -t echo -l; }; }
# postamble
sleep 1s; test $(pgrep -c -u $USER scxmlrun) -eq 0 || { pkill -u $USER scxmlrun; exit 1; }
exit 0
