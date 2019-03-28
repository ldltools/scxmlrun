# generated from "calc_mqtt.json" by scxmlrun-all at 2019-03-28T01:05:24
# (https://github.com/ldltools/scxmlrun/tree/master/tools/)
# preamble
set -eu -o pipefail
USER=${USER:-$(id -u -n)}
test $(pgrep -c -u $USER scxmlrun) -eq 0 || { echo "scxml running"; exit 1; }
# run: 3 processes
{ { mosquitto_sub -t calc_resp | jq -c '.event.name, .event.data?'; } > /dev/stdout; } &
{ scxmlrun ./calc.scxml --sub calc --pub calc_resp; } &
{ cat /dev/stdin | { sleep 1s; mosquitto_pub -t calc -l; sleep 1s; pkill -n mosquitto_sub; }; }
# postamble
while test $(pgrep -c -u $USER scxmlrun) -gt 0; do sleep 1s; done
exit 0
