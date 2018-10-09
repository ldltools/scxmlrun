all::
clean::;	rm -f *~ out/*
veryclean::	clean

run:
	@echo "** run $$scxml"
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

run-mqtt:
	@echo "** run $$scxml"
	@test -n "$$scxml" -a -f "$$scxml" || { echo "\"$$scxml\" is not valid" > /dev/stderr; exit 1; }
	@test -d out || exit 1
	@for f in {s,i}*.txt; do\
	  echo "[$$f]";\
	  trace=out/`basename $$scxml .scxml`-`basename $$f .txt`.trace;\
	  rm -f $$trace;\
	  ../runtest.sh $$scxml $$f --trace $$trace;\
	  test -f $$trace || exit 1;\
	  tail -1 $$trace | grep -q rejected && echo FAIL || echo PASS;\
	done
