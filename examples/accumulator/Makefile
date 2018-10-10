#

usage:
	@echo "usage: make <target>"
	@echo "where <target> can be either of the following"
	@echo -e "  run\t\t\tperform \"run-accumulator\" and \"run-monitors\""
	@echo -e "  run-accumulator\trun accumulator against each scenario in the \"scenarios\" directory"
	@echo -e "  run-monitors\t\trun monitors against the scenarios"
	@echo -e "  run-mqtt\t\trun accumulator + monitors (via MQTT)"
	@echo -e "  run-mqtt-passthru\trun passthru + monitors (via MQTT)"

all::
clean::
	rm -f *~
	make -C monitors $@
	make -C scenarios $@
veryclean::	clean
	rm -f out/*

run:	run-accumulator run-monitors

run-accumulator:
	@echo "** run accumulator.scxml"
	@for f in scenarios/s*.txt; do\
	  echo "[$$f]";\
	  scxmlrun accumulator.scxml $$f -o /dev/null;\
	  echo;\
	done

run-monitors:	check
	@for scxml in monitors/*.scxml; do\
	  scxml=`readlink -f $$scxml` make -C scenarios run;\
	done

run-mqtt:	check
	@for scxml in monitors/*.scxml; do\
	  scxml=`readlink -f $$scxml` make -C scenarios run-mqtt;\
	done

run-mqtt-passthru:	check
	@for scxml in monitors/*.scxml; do\
	  scxml=`readlink -f $$scxml` make -C scenarios run-mqtt-passthru;\
	done

%.scxml:	%.rules
	rules2scxml $< -o $@
%.uml:	%.scxml
	scxml2uml.sh $< -o $@ -landscape
%.svg:	%.uml
	plantuml.sh $< -tsvg

.PRECIOUS:	%.scxml %.uml %.svg
.PHONY:		check run

check:
	@for f in monitors/*.rules; do\
	  scxml=monitors/`basename $$f .rules`.scxml;\
	  test $$f -nt $$scxml && { echo "** $$scxml is older than $$f"; exit 1; };\
	done; exit 0
