#

usage::
	@echo "usage: make [test | test-solidity | test-combined]"

# scxml
test:
	shelltest voting.conf

# solidity
T_CONTRACTS	= contracts/Ballot.sol
T_TESTS		= $(shell echo scenarios/Ballot_scenario{1,2}.js)
T_GAS_LIMIT	= "8000000"

-include ../Makefile.truffle

test-solidity:	truffle-deploy truffle-test

# solidity + scxml
test-combined:	truffle-deploy run-combined
run-combined:
	./runtest.sh monitors/monitor1.scxml scenarios/scenario1.js 
