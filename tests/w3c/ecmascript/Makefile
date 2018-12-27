#

all::
	@echo "usage: make test"

clean::;	rm -f *~

SCXMLFILES	= $(shell echo test*.txml.scxml)
TESTFILES 	= $(shell echo test*.txml.json)
SCENARIOFILES	= $(TESTFILES:%.txml.json=scenarios/%.txt)
OUTFILES	= $(TESTFILES:%.txml.json=out/%.out)
TRACEFILES	= $(TESTFILES:%.txml.json=out/%.trace)

RUNTEST		= ../runtest.sh
CONVERT		= ./convert.sh

scenarios/%.txt:	%.txml.json
	$(CONVERT) $< > $@
out/%.trace:	%.txml.scxml scenarios/%.txt
	@mkdir -p out
	@echo "** $*" > /dev/stderr
	@topic=`basename $@ .trace`;\
	$(RUNTEST) $^ --topic "$$topic" -o out/$*.out --trace $@ || echo "** ERROR" >> $@

all::	$(SCENARIOFILES)

test:	$(TRACEFILES) check
run:	$(TRACEFILES)
check:
	@n=0; nsucc=0; nfail=0;\
	for f in $(TRACEFILES); do\
	  let n=n+1;\
	  test -f $$f || { echo "$$f : not found"; continue; };\
	  tail -n 1 $$f | fgrep -q \"pass\" && { let nsucc=nsucc+1; continue; };\
	  tail -n 1 $$f | fgrep -q \"fail\" && { echo "$$f : fail"; let nfail=nfail+1; continue; };\
	  echo "$$f : error";\
	done;\
	echo "pass : $$nsucc";\
	echo "fail : $$nfail";\
	echo "error : `expr $$n - $$nsucc - $$nfail`";\
	echo "total : $$n"

clean::
	pkill scxmlrun || true
	rm -f out/*

veryclean::	clean
realclean::	veryclean
	rm -f $(SCENARIOFILES)
