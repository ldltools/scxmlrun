#

usage:
	@echo "usage: make <target>"
	@echo "where <target> can be either of the following"
	@echo -e "\ttest\t\trun unit tests"
	@echo -e "\ttest-mqtt\trun tests that involve MQTT communication"

EXAMPLES =	hello echo ping_pong accumulator
EXAMPLES +=	microwave calc traffic_report blackjack
EXAMPLES +=	voting vulnerable

test-mqtt::
	@test $$(pgrep -c -u mosquitto mosquitto) -eq 1 || { echo "** mosquitto is not running"; exit 1; }

define TESTRULE
test::		test_$(1)
test-mqtt::	test-mqtt_$(1)
test_$(1):
	@cd ../examples/$(1); test -f $(1).conf && shelltest $(1).conf || true
test-mqtt_$(1):
	@cd ../examples/$(1); test -f $(1)_mqtt.conf && shelltest $(1)_mqtt.conf || true
endef
$(foreach ex,$(EXAMPLES),$(eval $(call TESTRULE,$(ex))))

all:
clean:;		find . -name '*~' | xargs rm -f
install:
