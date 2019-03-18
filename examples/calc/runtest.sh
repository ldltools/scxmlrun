#! /bin/bash

scxml=calc.scxml
ui=calc_ui.html

random=$(date +%N)
topic_scxml=calc${random}
topic_ui=calcui${random}
host=127.0.0.1
port=9001

mqttws=./mqttws31.min.js

no_browser=1
# this is potentially dangerous
browser=${browser:-/usr/bin/firefox}

verbose=

usage ()
{
    echo "usage: $(basename $0) [-v] [--host <host>] [--port <port>]"
}

#test $# -eq 0 && { usage; exit 0; }

while test $# -gt 0
do
    case $1 in
	--host)
	    host=$2
	    shift
	    ;;
	--port)
	    port=$2
	    shift
	    ;;
	--open-url)
	    no_browser=0
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
	*)
	    echo "unknown argument: $1"
	    exit 1
    esac
    shift
done

# check
test $(pgrep -u mosquitto mosquitto | wc -l) -eq 0 && { echo "** mosquitto is not running" > /dev/stderr; exit 1; }

# start scxmlrun
rm -f .calc_done
(scxmlrun $scxml --mqtt ${host} --sub ${topic_ui} --pub ${topic_scxml} $verbose && { touch .calc_done; echo "$scxml terminated"; }) &
sleep 1

# gen url for ui
test -f $mqttws || wget https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.0.1/mqttws31.min.js -O $mqttws
url="file:$(readlink -f ${ui})?host=${host}&port=${port}&sub=${topic_scxml}&pub=${topic_ui}"
if test ${no_browser} -eq 1 -o ! -x "$browser"
then
    echo -e "\n${scxml} has been invoked successfully"
    echo -e "OPEN: \"$url\"\n"
    test -f "$mqttws" || echo "** note that you need a local copy of \"mqttws31\""
else
    $browser --new-tab "$url" &
fi

# wait for termination
n=0
while test ! -f .calc_done
do
    sleep 1m
    let n=n+1; test $n -gt 60 && break
done

# clean up
rm -f .calc_done
USER=${USER:-$(id -u -n)}
test $(pgrep -u $USER scxmlrun | wc -l) -eq 0 || { echo "** scxmlrun is running" > /dev/stderr; pkill -u $USER scxmlrun; exit 1; }
