#! /bin/bash
set -Ceuo pipefail

passthru=passthru.scxml
echo_=echo.scxml
scenario=
topic="echo/$(date +%N)"
verbose=

usage ()
{
    echo "usage: $(basename $0) [--restart] <scenario>"
    echo "description"
    echo "  events defined in <scenario> are first processed by passthru.scxml."
    echo "  then passthru forwards the events via MQTT,"
    echo "  which are received by echo.scxml"
    echo "dataflow"
    echo "  <scenario> -(events)-> passthru.scxml -(events via MQTT)-> echo.scxml"
}

test $# -eq 0 && { usage; exit 0; }
while test $# -gt 0
do
    case $1 in
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
	-h | --help)
	    usage; exit 0
	    ;;
	-*)
	    echo "unknown option: $1"
	    exit 1
	    ;;
	*) 
	    scenario=$1
    esac
    shift
done

test .$scenario = . && { echo "scenario:\"$scenario\" not specified"; exit 1; }
test -e "$scenario" || { echo "scenario:\"$scenario\" not found"; exit 1; }

#test $(pgrep -u mosquitto mosquitto | wc -l) -eq 1 || { echo "mosquitto is not running" > /dev/stderr; exit 1; }

# mqtt($topic) -> echo.scxml
#echo "start ${echo_} ($topic)"
rm -f .echo_done
(scxmlrun ${echo_} --sub "$topic" $verbose; touch .echo_done) &
sleep 1

# echo.in -> passthru.scxml -> mqtt($topic)
#echo "start $passthru ($topic)"
scxmlrun $passthru $scenario --pub "$topic" $verbose
#echo "passthru done"

# wait for termination
n=0
while test ! -f .echo_done
do
    sleep 1s
    let n=n+1; test $n -gt 10 && break
done

# clean up
rm -f .echo_done
USER=${USER:-$(id -u -n)}
test $(pgrep -u $USER scxmlrun | wc -l) -eq 0 || { echo "** scxmlrun is running" > /dev/stderr; pkill -u $USER scxmlrun; exit 1; }
