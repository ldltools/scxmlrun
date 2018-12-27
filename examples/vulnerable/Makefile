#

usage:
	@echo "usage: make [test | test-solidity | test-combined]"

# scxml
test:
	shelltest vulnerable.conf

# solidity

T_BUILD_DIR	= ./_build
T_CONTRACTS	= $(shell echo contracts/SimpleDAO*.sol)
T_TESTS		= scenarios/SimpleDAO_test.js
T_GAS_LIMIT	= "40000000"

-include ../Makefile.truffle

test-solidity:	truffle-deploy truffle-test

# solidity + scxml
test-combined:	truffle-deploy
#	./runtest.sh monitors/monitor1.scxml scenarios/scenario1.js
