#! /bin/bash

root=$(dirname $(readlink -f $0))

accumulator=$root/accumulator.scxml
passthru=$root/passthru.scxml
monitor=
scenario=

trace=/dev/null
topic="accumulator/$(date +%N)"
verbose=

usage ()
{
    echo "usage: $(basename $0) <monitor> <scenario> [--passthru]"
    echo "description"
    echo "  events in <scenario> are processed by accumulator which via MQTT"
    echo "    <scenario> -(events)-> accumulator -(events via MQTT)-> <monitor>"
    echo "  when \"--passthru\" is specified, accumulator is replaced with passthru"
}

test $# -eq 0 && { usage; exit 0; }

while test $# -gt 0
do
    case $1 in
	--passthru)
	    accumulator=$passthru
	    ;;
	--trace)
	    trace=$2
	    shift
	    ;;
	-v)
	    verbose="-v"
	    ;;
	-vv)
	    verbose="-vv"
	    ;;
	-h | --help)
	    usage; exit 0
	    ;;
	-*)
	    echo "unknown option: $1"
	    exit 1
	    ;;
	*) 
	    test -z "$monitor" && monitor=$1 || scenario=$1
    esac
    shift
done

test -f "$accumulator" || { echo "accumulator:\"$accumulator\" not fund"; exit 1; }
test -f "$monitor" || { echo "monitor:\"$monitor\" not fund"; exit 1; }
test -f "$scenario" || { echo "scenario:\"$scenario\" not fund"; exit 1; }
test -f "$trace" && rm -f $trace

# mqtt($topic) -> $monitor
rm -f .monitor_done
echo "start $(basename $monitor) ($topic)"
(scxmlrun $monitor --sub "$topic" --trace $trace ${verbose};\
 touch .monitor_done;\
 echo "$(basename $monitor) done") &
sleep 1

# $scenario -> $accumulator -> mqtt($topic)
rm -f .accumulator_done
echo "start $(basename $accumulator) ($topic)"
(scxmlrun $accumulator $scenario --pub "$topic" -q;\
 touch .accumulator_done;\
 echo "$(basename $accumulator) done";\
 mosquitto_pub -t $topic -m '{"event" : {"name" : "_accept"}}') &

n=0
while test ! \( -f .accumulator_done -a -f .monitor_done \)
do
    sleep 1
    let n=n+1; test $n -gt 10 && break
done
rm -f .accumulator_done .monitor_done

test $(pgrep -u $USER scxmlrun | wc -l) -eq 0 || { echo "** scxmlrun is running" > /dev/stderr; pkill -u $USER scxmlrun; exit 1; }
