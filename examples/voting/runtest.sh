#! /bin/bash

monitor=monitors/monitor1.scxml
observer=scenarios/observer.js
scenario=scenarios/scenario1.js
topic=voting
export ROOT=$(readlink -f $(dirname $0))/_build
echo "ROOT=$ROOT"

# observer: ganache -> mqtt
echo "* starting $observer"
rsync $observer _build/test
(cd $(dirname $observer); node $(basename $observer)) &
sleep 1

# monitor
echo "* starting $monitor"
scxmlrun $monitor --sub $topic -v &
sleep 1

# scenario
echo "* running $scenario"
(cd $(dirname $scenario); node $(basename $scenario))

# terminate observer
#pkill node
