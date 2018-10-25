#

usage:
	@echo "usage: make launch|build|test"

all::
clean::
install::

# truffle
CONTRACTS	= $(shell echo contracts/{SimpleDAO,SimpleDAO_exploit}.sol)
SCENARIOS	= $(shell echo scenarios/*.js)
TESTS		= $(shell echo scenarios/swc107*.js)
# ganache
PORT		= 7545
NETWORK		= 5777
GAS_LIMIT	= 8000000

build:	clean compile launch deploy

launch:	check-ganache
	file=.ganache_pid; test -s $$file && echo "ganache is running" ||\
	{ (ganache-cli -p $(PORT) --networkId $(NETWORK) --gasLimit $(GAS_LIMIT) &);\
	  pgrep -a node | grep ganache-cli | gawk '{print($$1)}' > $$file; }

check-ganache:
	@file=.ganache_pid; test -s $$file -a -d "/proc/$$(cat $$file)" ||\
	{ pgrep -a node | grep ganache-cli | gawk '{print($$1)}' > $$file; }

clean::
	rm -f .ganache_pid

init:
	test -d _build ||\
	(mkdir -p _build; cd _build; truffle init; mv -f truffle.js{,.bak})

compile:	init _build/truffle.js
	rsync contracts/*.sol _build/contracts
	rm -f _build/build/contracts/*
	(cd _build; truffle compile)

deploy:	compile _build/migrations/2_deploy_contracts.js check-ganache
	test -s .ganache_pid || { echo "ganache is not running"; exit 1; }
	(cd _build; truffle deploy)

test:	check-ganache
	rsync $(TESTS) _build/test
	test -s .ganache_pid || { echo "ganache is not running"; exit 1; }
	(cd _build; truffle test)

_build/truffle.js:
	rm -f $@; touch $@
	echo "module.exports = {"	>> $@
	echo " networks: {"		>> $@
	echo "  development: {"		>> $@
	echo "   host: \"127.0.0.1\", port: $(PORT), network_id: \"*\"" >> $@
	echo "}}};"			>> $@

_build/migrations/2_deploy_contracts.js:	$(CONTRACTS)
	mkdir -p _build/migrations
	rm -f $@; touch $@
	for f in $(CONTRACTS); do \
	  echo "var $$(basename $$f .sol) = artifacts.require (\"./$$(basename $$f)\");" >> $@;\
	done
	echo "module.exports = function (deployer) {" >> $@
	for f in $(CONTRACTS); do \
	  echo "deployer.deploy($$(basename $$f .sol));" >> $@;\
	done
	echo "}" >> $@

veryclean::	clean
	rm -rf _build
