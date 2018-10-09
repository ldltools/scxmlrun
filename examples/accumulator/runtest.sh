#! /bin/bash

root=$(dirname $(readlink -f $0))
scxml=
passthru=$root/passthru.scxml
scenario=
trace=/dev/null
topic="accumulator$(date +%N)"
verbose=

while test $# -gt 0
do
    case $1 in
	--trace)
	    trace=$2
	    shift
	    ;;
	-v)
	    verbose="-v"
	    ;;
	-)
	    echo "unknown option: $1"
	    exit 1
	    ;;
	*) 
	    test -z "$scxml" && scxml=$1 || scenario=$1
    esac
    shift
done

test -f "$scxml" || { echo "\"$scxml\" not fund"; exit 1; }
test -f "$passthru" || { echo "\"$passthru\" not fund"; exit 1; }
test -f "$scenario" || { echo "\"$scenario\" not fund"; exit 1; }
test -f "$trace" && rm -f $trace

# mqtt($topic) -> $scxml
echo "start $scxml ($topic)"
(scxmlrun $scxml --sub "$topic" --trace $trace $verbose; echo "$scxml done") &
sleep 1

# $scenario -> $passthru -> mqtt($topic)
echo "start passthru ($topic)"
scxmlrun $passthru $scenario --pub "$topic"
echo "passthru done"
