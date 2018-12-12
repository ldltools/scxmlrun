#

usage:
	@echo "usage: make <target>"
	@echo "where <target> can be either of the following"
	@echo -e "\ttest\trun tests"

all::
clean::;	find . -name '*~' | xargs rm -f
install::

test::	test_examples

EXAMPLES =	hello echo ping_pong accumulator
EXAMPLES +=	microwave calc traffic_report blackjack
EXAMPLES +=	voting

test_examples:
	for d in $(EXAMPLES:%=../examples/%); do \
	  conf=$$(basename $$d).conf; (cd $$d; test -f $$conf && shelltest $$conf); \
	done
