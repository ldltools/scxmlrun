#

usage:
	@echo "usage: make <target>"
	@echo "where <target> can be either of the following"
	@echo -e "\trun\t\trun each scenario in the \"scenarios\" directory"
	@echo -e "\trun-mqtt\tmqtt version"

all::
clean::
	rm -f *~
	make -C monitors $@
	make -C scenarios $@
veryclean::	clean
	rm -f out/*

run:	check
	for scxml in monitors/*.scxml; do\
	  scxml=`readlink -f $$scxml` make -C scenarios $@;\
	done

run-mqtt:	check
	for scxml in monitors/*.scxml; do\
	  scxml=`readlink -f $$scxml` make -C scenarios $@;\
	done

%.scxml:	%.rules
	rules2scxml $< -o $@
%.uml:	%.scxml
	scxml2uml.sh $< -o $@
%.svg:	%.uml
	plantuml.sh $< -tsvg

.PRECIOUS:	%.scxml %.uml %.svg
.PHONY:		check run

check:
	@for f in monitors/*.rules; do\
	  scxml=monitors/`basename $$f .rules`.scxml;\
	  test $$f -nt $$scxml && { echo "** $$scxml is older than $$f"; exit 1; };\
	done; exit 0
