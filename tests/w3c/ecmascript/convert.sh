#! /bin/bash

infile=$1
test -f "$infile" || { echo "$infile not found" > /dev/stderr; exit 1; }

cat $infile | gawk '
/^ *"(initial|next)Configuration/ {
	state = gensub (/.*Configuration\" : \[(.*)\].*/, "\\1", "g", $0);
	printf ("{\"state\" : {\"id\" : %s}}\n", state);
}
/^ *"event"/ {
	event = gensub (/.*event\" : ({.*}).*/, "\\1", "g", $0);
	printf ("{\"event\" : %s}\n", event);
}
/^ *"after"/ {
	printf ("{\"delay\" : %d}\n", $3);
}'
