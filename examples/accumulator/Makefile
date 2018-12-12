#

usage:
	@echo "usage: make <target>"
	@echo "where <target> can be either of the following"
	@echo -e "  run\t\t\tperform \"run-accumulator\" and \"run-monitors\""
	@echo -e "  run-accumulator\trun accumulator against each scenario in the \"scenarios\" directory"
	@echo -e "  run-monitors\t\trun monitors against the scenarios"

all::
clean::
	rm -f *~ .*done
	$(MAKE) -C monitors $@
	$(MAKE) -C scenarios $@

veryclean::	clean
	rm -f out/*

run:	run-accumulator run-monitors

# accumulator
run-accumulator:
	shelltest accumulator.conf
run-passthru-accumulator:
	shelltest passthru_accumulator.conf

# monitors
run-monitors:	check
	shelltest monitors.conf
run-passthru-monitors:	check
	shelltest passthru_monitors.conf

# accumulator + monitors
run-accumulator_monitors:	check
	shelltest accumulator_monitors.conf

#
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
