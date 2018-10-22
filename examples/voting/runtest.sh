#! /bin/bash

monitor=monitors/monitor1.scxml
observer=scenarios/observer.js
scenario=scenarios/scenario1.js
topic=voting

# observer: ganache -> mqtt
(cd $(dirname $observer); node $(basename $observer)) &
sleep 1

# monitor
scxmlrun $monitor --sub $topic -v &
sleep 1

# scenario
(cd $(dirname $scenario); node $(basename $scenario))

# terminate observer
pkill node
