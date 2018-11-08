#

usage:
	@echo "usage: make launch|build|test"

PROJECT_DIR	= .
#CONTRACTS	= $(shell echo contracts/*.sol)
CONTRACTS	= $(shell echo contracts/SimpleDAO*.sol)
TESTS		= scenarios/swc107_test.js
GAS_LIMIT	= "40000000"

include ../Makefile.truffle
