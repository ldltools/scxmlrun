all::
clean::
	find . -name '*~' | xargs rm -f

launch:
	pkill -u $$USER node || true
	port=7545; network=5777; gas_price=20000000000; gas_limit=8000000;\
	ganache-cli -p $$port --networkId $$network\
		--gasLimit $${gas_limit} &

build:	clean
	truffle compile && truffle deploy

clean::
	rm -f ./build/contracts/*json
