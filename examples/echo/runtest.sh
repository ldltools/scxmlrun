#! /bin/bash

topic="echo/$(date +%N)"

test $(pgrep -u mosquitto mosquitto | wc -l) -eq 1 || { echo "mosquitto is not running" > /dev/stderr; exit 1; }

# mqtt($topic) -> echo.scxml
echo "start echo.scxml ($topic)"
rm -f .echo_done
(scxmlrun echo.scxml --sub "$topic" -v; touch .echo_done) &
sleep 1

# echo.in -> passthru.scxml -> mqtt($topic)
echo "start passthru ($topic)"
scxmlrun passthru.scxml echo.in --pub "$topic"
echo "passthru done"

# wait for termination
n=0
while test ! -f .echo_done
do
    sleep 1s
    let n=n+1; test $n -gt 10 && break
done

# clean up
rm -f .echo_done
test $(pgrep -u $USER scxmlrun | wc -l) -eq 0 || { echo "** scxmlrun is running" > /dev/stderr; pkill -u $USER scxmlrun; exit 1; }
