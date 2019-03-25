# generated from "ping_pong_mqtt.json" by scxmlrun-all at 2019-03-25T01:59:13
# (https://github.com/ldltools/scxmlrun/tree/master/tools/)
# preamble
set -eu -o pipefail
USER=${USER:-$(id -u -n)}
test $(pgrep -c -u $USER scxmlrun) -eq 0 || { echo "scxml running"; exit 1; }
# run: 3 processes
{ scxmlrun ./ping.scxml --sub ping --mqtt; } &
{ scxmlrun ./pong.scxml --sub pong --mqtt; } &
{ cat /dev/stdin | { sleep 1s; mosquitto_pub -t ping -s; }; }
# postamble
sleep 3s
sleep 1s; test $(pgrep -c -u $USER scxmlrun) -eq 0 || { pkill -u $USER scxmlrun; exit 1; }
exit 0
