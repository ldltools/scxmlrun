usage:
	@echo "usage: make run | run-mqtt"

all::
clean::;	rm -f *~ out/*
veryclean::	clean

run:
#	"scxml" should refer to a scxml file
	@echo "** run `basename $$scxml`"
	@test -n "$$scxml" -a -f "$$scxml" || { echo "\"$$scxml\" is not valid" > /dev/stderr; exit 1; }
	@test -d out || exit 1
	@for f in {s,i}*.txt; do\
	  echo "[$$f]";\
	  trace=out/`basename $$scxml .scxml`-`basename $$f .txt`.trace;\
	  rm -f $$trace;\
	  scxmlrun $$scxml $$f --trace $$trace;\
	  test -f $$trace || exit 1;\
	  tail -1 $$trace | grep -q rejected && echo FAIL || echo PASS;\
	done

run-mqtt-passthru:
	@echo "** run `basename $$scxml`"
	@test -n "$$scxml" -a -f "$$scxml" || { echo "\"$$scxml\" is not valid" > /dev/stderr; exit 1; }
	@test -d out || exit 1
	@for f in {s,i}*.txt; do\
	  echo "[$$f]";\
	  trace=out/passthru-`basename $$scxml .scxml`-`basename $$f .txt`.trace;\
	  rm -f $$trace;\
	  ../runtest.sh $$scxml $$f --trace $$trace --passthru;\
	  test -f $$trace || exit 1;\
	  tail -1 $$trace | grep -q rejected && echo FAIL || echo PASS;\
	done

# note: accumulator.scxml does not work with scenarios/invalid*.txt
#       because they just hang up when receiving a unrecognizable event
run-mqtt:
	@echo "** run `basename $$scxml`"
	@test -n "$$scxml" -a -f "$$scxml" || { echo "\"$$scxml\" is not valid" > /dev/stderr; exit 1; }
	@test -d out || exit 1
	@for f in s*.txt; do\
	  echo "[$$f]";\
	  trace=out/`basename $$scxml .scxml`-`basename $$f .txt`.trace;\
	  rm -f $$trace;\
	  ../runtest.sh $$scxml $$f --trace $$trace;\
	  test -f $$trace || exit 1;\
	  tail -1 $$trace | grep -q rejected && echo FAIL || echo PASS;\
	done
