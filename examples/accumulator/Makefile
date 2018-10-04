#

usage:
	@echo "usage: make <target>"
	@echo "where <target> can be either of the following"
	@echo -e "\tscxml\tgenerate SCXML files (in ./out)"
	@echo -e "\trun\trun tests using scxmlrun"
	@echo -e "\t\t(for testing on hyperledger, see hyperledger/README.md)"

all::
clean::;	rm -f *~
veryclean::	clean
	rm -f out/*

RULES = $(shell echo accumulator*.rules)

%.scxml:	%.rules
	rules2scxml $< -o $@
%.uml:	%.scxml
	scxml2uml.sh $< -o $@
%.svg:	%.uml
	plantuml.sh $< -tsvg

veryclean::
	rm -f $(RULES:%.rules=%.uml) $(RULES:%.rules=%.svg)

.PRECIOUS:	%.scxml %.uml %.svg

scxml:	$(RULES:%.rules=%.scxml)
uml:	$(RULES:%.rules=%.uml)
svg:	$(RULES:%.rules=%.svg)

check:	$(RULES:%.rules=%.scxml)
	@for f in $(RULES); do\
	  scxml=`basename $$f .rules`.scxml;\
	  test $$f -nt $$scxml && { echo "** $$scxml is obsolete"; exit 1; };\
	done; exit 0

run:	check
	make -C tests $@
