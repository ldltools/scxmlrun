#

all::
clean::;	rm -f *~
veryclean::	clean
	rm -f out/*

RULES = $(shell echo accumulator*.rules)

%.scxml:	%.rules
	reject_invalid_events=1 rules2scxml $< -o $@
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
