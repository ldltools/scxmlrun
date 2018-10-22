#

all::
clean::;	rm -f *~
veryclean::	clean
	rm -f out/*

RULES = $(shell echo monitor*.rules)

%.scxml:	%.rules
	reject_invalid_events=1 rules2scxml $< -o $@
%.puml:	%.scxml
	scxml2uml.sh $< -o $@
%.svg:	%.puml
	plantuml.sh $< -tsvg

veryclean::
	rm -f $(RULES:%.rules=%.puml) $(RULES:%.rules=%.svg)

.PRECIOUS:	%.scxml %.puml %.svg

scxml:	$(RULES:%.rules=%.scxml)
puml:	$(RULES:%.rules=%.puml)
svg:	$(RULES:%.rules=%.svg)

check:	$(RULES:%.rules=%.scxml)
	@for f in $(RULES); do\
	  scxml=`basename $$f .rules`.scxml;\
	  test $$f -nt $$scxml && { echo "** $$scxml is obsolete"; exit 1; };\
	done; exit 0
