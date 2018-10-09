#

usage:
	@echo "usage: make run"

all::
clean::;	rm -f *~

run:
	@echo "compute 12 + 34 = 46"
#	scxmlrun calc.scxml calc.in -v
	scxmlrun calc.scxml calc.in -v |& fgrep 'log'
