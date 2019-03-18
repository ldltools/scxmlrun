#! /bin/bash
set -Ceuo pipefail

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
    echo "usage: $(basename $0) [--restart] [-f <accumulator>] <monitor> <scenario>"
    echo "description"
    echo "  events defined in <scenario> are first processed by <accumulator>."
    echo "  then, <accumulator> responds by emitting new events via MQTT,"
    echo "  which are received by <monitor>"
    echo "dataflow"
    echo "  <scenario> -(events)-> <accumulator> -(events via MQTT)-> <monitor>"
}

test $# -eq 0 && { usage; exit 0; }

while test $# -gt 0
do
    case $1 in
	-f | --accumulator)
	    accumulator=$2
	    shift
	    ;;
	-p | --passthru)
	    accumulator=$passthru
	    ;;
	--trace)
	    trace=$2
	    shift
	    ;;
	-r | --restart)
	    sudo /etc/init.d/mosquitto restart
	    test .$verbose = . || echo -n ";; [runtest] restarting mosquitto ... " > /dev/stderr
	    n=0; while test $n -lt 5; do sleep 1s; let n=n+1; done
	    pgrep mosquitto > /dev/null || { echo "failed" > /dev/null; exit 1; }
	    test .$verbose = . || echo "done" > /dev/stderr
	    ;;
	-v | --verbose)
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

test -f "$accumulator" || { echo "accumulator:\"$accumulator\" not found"; exit 1; }
test -f "$monitor" || { echo "monitor:\"$monitor\" not found"; exit 1; }
test -e "$scenario" || { echo "scenario:\"$scenario\" not found"; exit 1; }
test -f "$trace" && rm -f $trace

#
test .$verbose = . || echo ";; [runtest] $scenario -> $accumulator -> $monitor" > /dev/stderr

# mqtt($topic) -> $monitor
rm -f .monitor_done
test .$verbose = . || echo ";; [runtest] start $(basename $monitor) ($topic)"
(scxmlrun $monitor --sub "$topic" --trace $trace ${verbose}; touch .monitor_done) &
test .$verbose = . || echo ";; $(basename $monitor) launched" > /dev/stderr
#sleep 3s

# $scenario -> $accumulator -> mqtt($topic)
rm -f .accumulator_done
test .$verbose = . || echo ";; [runtest] start $(basename $accumulator) ($topic)"
(scxmlrun $accumulator $scenario --pub "$topic" -q; touch .accumulator_done) &
#while test ! -e .accumulator_done; do sleep 1; done
test .$verbose = . || echo ";; [runtest] $(basename $accumulator) launched" > /dev/stderr
#sleep 3s

# wait until accumulator is done
n=0
while test ! \( -f .accumulator_done \)
do
    sleep 1s
    let n=n+1; test $n -gt 10 && break
done
test -f .accumulator_done || exit 1
test .$verbose = . || echo ";; [runtest] $(basename $accumulator) done" > /dev/stderr
rm -f .accumulator_done .monitor_done

# wait until monitor is done
mosquitto_pub -t $topic -m '{"event" : {"name" : "_accept"}}'
n=0
while test ! \( -f .monitor_done \)
do
    sleep 1s
    let n=n+1; test $n -gt 10 && break
done
test -f .monitor_done || exit 1
test .$verbose = . || echo ";; [runtest] $(basename $monitor) done" > /dev/stderr

rm -f .monitor_done
USER=${USER:-$(id -u -n)}
test $(pgrep -u $USER scxmlrun | wc -l) -eq 0 || { echo "** scxmlrun is running" > /dev/stderr; pkill -u $USER scxmlrun; exit 1; }
