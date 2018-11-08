#! /bin/bash

monitor=
scenario=
observer=monitors/observer.js
topic=voting

export ROOT=$(readlink -f $(dirname $0))/_build
## monitors/observer.js (and scenarios) refers to this variable

usage ()
{
    echo "usage: $(basename $0) <monitor>.scxml <scenario>.js"
}

test $# -eq 0 && { usage; exit 0; }

while test $# -gt 0
do
    case $1 in
	-h | --help)
	    usage; exit 0
	    ;;
	-*)
	    ;;
	*)
	    test -z "$monitor" && monitor=$1 || scenario=$1
    esac
    shift
done

test -f "$scenario" || exit 1
test -f "$monitor" || exit 1
test -f "$observer" || exit 1

pgrep -a node | grep -q ganache-cli || { echo "ganache not running" > /dev/stderr; exit 1; }

# observer: ganache -> mqtt
echo
echo "* starting $observer"
#rsync $observer _build/test
(cd $(dirname $observer); node $(basename $observer)) &
sleep 1

# monitor
echo
echo "* starting $monitor"
scxmlrun $monitor --sub $topic -v &
sleep 1

# scenario
echo
echo "* running $scenario"
(cd $(dirname $scenario); node $(basename $scenario))

# terminate observer
#pkill node
