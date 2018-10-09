#! /bin/bash

topic="echo/$(date +%N)"

test $(pgrep -u mosquitto mosquitto | wc -l) -eq 1 || { echo "mosquitto is not running" > /dev/stderr; exit 1; }

# mqtt($topic) -> echo.scxml
echo "start echo.scxml ($topic)"
(scxmlrun echo.scxml --sub "$topic"; echo "echo done") &
sleep 1

# echo.in -> passthru.scxml -> mqtt($topic)
echo "start passthru ($topic)"
scxmlrun passthru.scxml echo.in --pub "$topic"
echo "passthru done"
