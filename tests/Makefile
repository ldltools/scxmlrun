#

usage:
	@echo "usage: make <target>"
	@echo "where <target> can be either of the following"
	@echo -e "\ttest\trun tests"

EXAMPLES =	hello echo ping_pong accumulator
EXAMPLES +=	microwave calc traffic_report blackjack
EXAMPLES +=	voting vulnerable

define TESTRULE
test::	test_$(1)
test_$(1):
	@cd ../examples/$(1); test -f $(1).conf && shelltest $(1).conf
endef
$(foreach ex,$(EXAMPLES),$(eval $(call TESTRULE,$(ex))))

all::
clean::;	find . -name '*~' | xargs rm -f
install::
