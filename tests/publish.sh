#! /bin/bash

infile=/dev/stdin
host=localhost
topic=

while test $# -gt 0
do
    case $1 in
	--host | --mqtt)
	    host=$2
	    shift;;
	--topic)
	    topic=$2
	    shift;;
	-h | --help)
	    echo "usage: $(basename $0) [--mqtt <host>] --topic <topic> <infile>"
	    exit 0;;
	*)
	    infile=$1
    esac
    shift
done

test ."$topic" = . && topic=$(basename $infile | sed 's/\..*//')

# each input line should look like:
# - {"state" : {"id" : "...", ..}}
# - {"event" : {"name" : "...", ...}}
# - {"delay" : ms}

cat $infile |\
while read -r line || [[ -n "$line" ]]
do
    #echo "* $line"
    #echo \"`echo $line | jq .state`\"
    #echo \"`echo $line | jq .event`\"
    #echo \"`echo $line | jq .delay`\"
    #continue

    if test "`echo $line | jq .state`" != "null"
    then
	continue
    elif test "`echo $line | jq .event`" != "null"
    then
	#echo "* event: $line" > /dev/stderr
	mosquitto_pub -h $host -t $topic -m "$line"
    elif test "`echo $line | jq .delay`" != "null"
    then
	ms=$(echo $line | jq ".delay")
	let s=ms/1000+1
	#echo "* sleep ${s}s" > /dev/stderr
	sleep $s
    else
	echo "** error in reading: " $line > /dev/stderr
	exit 1
    fi
done
