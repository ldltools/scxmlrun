#! /bin/bash

rm -f .ping_done .pong_done

# ----------
# PING
# ----------
echo "start ping"
./ping.sh && { echo "ping done"; touch .ping_done; } &

# ----------
# PONG
# ----------
echo "start pong"
./pong.sh && { echo "pong done"; touch .pong_done; } &

# ----------
# PING_PONG -- monitor
# ----------
echo "start ping_pong"
pkill scxmlrun
scxmlrun ./ping_pong.scxml --sub ping --sub pong --sub ping_pong &
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

rm -f .ping_done .pong_done
