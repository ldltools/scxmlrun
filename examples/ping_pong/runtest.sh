#! /bin/bash

test $(pgrep -u mosquitto mosquitto | wc -l) -eq 1 || { echo "mosquitto is not running" > /dev/stderr; exit 1; }

# ----------
# PING
# ----------
echo "start ping"
rm -f .ping_done
./ping.sh && { echo "ping done"; touch .ping_done; } &

# ----------
# PONG
# ----------
echo "start pong"
rm -f .pong_done
./pong.sh && { echo "pong done"; touch .pong_done; } &

# ----------
# PING_PONG -- monitor
# ----------
echo "start ping_pong"
pkill scxmlrun
(scxmlrun ./ping_pong.scxml --sub ping --sub pong --sub ping_pong;\
 echo "ping_pong done") &
sleep 1
mosquitto_pub -t ping_pong -m "{\"event\":{\"name\":\"_init\"}}"

#echo sending a message to ping
mosquitto_pub -t ping -m "{\"event\":{\"name\":\"ping\", \"data\":0}}"

while true
do
    test -f .ping_done -a -f .pong_done && break
    sleep 1
done
mosquitto_pub -t ping_pong -m "{\"event\":{\"name\":\"_accept\"}}"

# ----------
# clean up
# ----------

## to be extra sure..
#pkill scxmlrun

rm -f .ping_done .pong_done
