#! /bin/sh

scxmlrun=scxmlrun
publish=$(dirname $(readlink -f $0))/publish.sh
test -x "$publish" || { echo "$publish not found" > /dev/stderr; exit 1; }

scxmlfile=
infile=
outfile=/dev/null
tracefile=/dev/stdout

host=localhost
topic=
verbose_opt="-q"

usage ()
{
    echo "usage: `basename $0` [<option>*] <scxmlfile> <infile>"
}

test $# -eq 0 && { usage; exit 0; }

while test $# -gt 0
do
    case $1 in
	--mqtt)
	    host=$2
	    shift;;
	--topic)
	    topic=$2
	    shift;;
	-o)
	    outfile=$2
	    shift;;
	--trace)
	    tracefile=$2
	    shift;;
	-v* | --verbose)
	    verbose_opt="$1"
	    ;;
	-h | --help)
	    usage
	    exit 0
	    ;;
	*)
	    test ."$scxmlfile" = . && scxmlfile=$1 || infile=$1
    esac
    shift
done

test ."$scxmlfile" = . && { echo "no scxml file specified" > /dev/stderr; exit 1; }
test ."$infile" = . && { echo "no input file specified" > /dev/stderr; exit 1; }
test -f "$outfile" && rm -f $outfile
test -f "$tracefile" && rm -f $tracefile

run ()
{
cat $infile |\
while read -r line || exit 0
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
	echo $line
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
    line=""
done |\
$scxmlrun $scxmlfile /dev/stdin -o $outfile --trace $tracefile ${verbose_opt} || exit 1
}

run_mqtt ()
{
    test ."$topic" = . && topic=$(basename "$infile" | sed 's/\..*//')
    $scxmlrun $scxmlfile --mqtt $host --sub "$topic" --trace $tracefile -q &
    test ".$infile" = . && exit 0
    $publish --host $host --topic "$topic" "$infile"
}

run
