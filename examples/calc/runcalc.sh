#! /bin/bash

scxml=./calc.scxml
ui=./calc_ui.html

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
    echo "usage: $(basename $0) [-v] [--host <host>] [--port <port>] [--no-ui|--with-ui]"
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

	--with-ui)
	    no_browser=0
	    ;;
	--no-ui)
	    no_browser=1
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

#
start_calc ()
{
# check
test $(pgrep -c -u mosquitto mosquitto) -eq 0\
    && { echo "** mosquitto is not running" > /dev/stderr; exit 1; }

# start scxmlrun
rm -f .calc_done

scxmlrun $scxml --broker ${host} --sub ${topic_ui} --pub ${topic_scxml} $verbose 2> /dev/null\
    && { touch .calc_done; echo "$scxml terminated"; }

echo -e "\n${scxml} has terminated"

}

#
run_ui ()
{
local url=$1

if test $(echo -n $browser | tail -c 7) = firefox
then
    $browser --new-instance "$url"
else
    echo "** \"$browser\" not supported" > /dev/stderr
    exit 1
fi

# wait for termination of scxmlrun
#local n=0
#while test ! -f .calc_done
#do
#    sleep 1m
#    let n=n+1; test $n -gt 60 && break
#done
#rm -f .calc_done

# clean up
USER=${USER:-$(id -u -n)}
test $(pgrep -c -u $USER scxmlrun) -gt 0 && pkill -u $USER -n scxmlrun

}

# helpers

check_port ()
{
    local host=$1
    local port=$2

    echo -n "* checking $host:$port: "
    local status=$(nmap -p $port $host | gawk '/^PORT/{getline; print($2); exit 0}')
    test "$status" = open && { echo "open"; true; } || { echo "closed"; false; }
}

print_urls ()
{
    params="&port=${port}&sub=${topic_scxml}&pub=${topic_ui}"
    interfaces=$(ip addr | gawk '/state UP/ {print $2}' | sed 's/://')
    for if in $interfaces
    do
	addr=$(ifconfig $if | gawk '/ inet /{print($2);exit}')
	check_port $addr 9001 || continue
	url="file:///$(basename $ui)?host=${addr}${params}"
	echo "URL: $url"
    done
}

# ----
# main
# ----

echo -n "invoking \"$scxml\" ... "
start_calc &
echo "done"

# mqtt over ws
test -f $mqttws\
    || wget https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.0.1/mqttws31.min.js -O $mqttws

# gen url for ui
url="file://$(readlink -f ${ui})?host=${host}&port=${port}&sub=${topic_scxml}&pub=${topic_ui}"

# case no-ui
if test ${no_browser} -eq 1 -o ! -x "$browser"
then
    echo -e "\nopen the following url on your browser"
    echo "URL: $url"

    echo -e "\nin case you need a URL for a non-loopback interface, try either of these:"
    print_urls
    echo
    exit 0
fi

# run
echo "launching \"$browser\""
run_ui $url
