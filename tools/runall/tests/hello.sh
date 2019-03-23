# generated from "hello.json" by scxmlrun-all at 2019-03-23T05:38:56
# (https://github.com/ldltools/scxmlrun/tree/master/tools/)
# preamble
set -eu -o pipefail
USER=${USER:-$(id -u -n)}
test $(pgrep -c -u $USER scxmlrun) -eq 0 || { echo "scxml running"; exit 1; }
# run: 1 process
{ scxmlrun ../../../examples/hello/hello.scxml  -i /dev/null  -o /dev/null; }
# postamble
sleep 1s; test $(pgrep -c -u $USER scxmlrun) -eq 0 || { pkill -u $USER scxmlrun; exit 1; }
exit 0
