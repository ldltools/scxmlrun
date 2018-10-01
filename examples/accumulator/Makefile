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

-include ../Makefile.common

RULES = $(shell echo accumulator*.rules)

dfa:	$(RULES:%.rules=out/%.dfa.png)
scxml:	$(RULES:%.rules=out/%.scxml)
	for f in $^; do test $$f -nt `basename $$f` && cp -i $$f .; done; true
uml:	$(RULES:%.rules=out/%.uml)
svg:	$(RULES:%.rules=out/%.svg)
	for f in $^; do test $$f -nt `basename $$f` && cp -i $$f .; done; true
.PRECIOUS:	%.scxml %.uml %.svg

check:	$(RULES:%.rules=%.scxml)
	@for f in $(RULES); do\
	  scxml=`basename $$f .rules`.scxml;\
	  test $$f -nt $$scxml && { echo "** $$scxml is obsolete"; exit 1; };\
	done; exit 0

run:	check
	make -C tests $@
