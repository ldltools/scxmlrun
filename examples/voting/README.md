# voting

## contract: [Ballot](contracts/Ballot.sol) in Solidity

[This contract](contracts/Ballot.sol) originally appears in this [Solidity tutorial material](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#voting).

In addition to the Solidity version,
we also provide another [DSL4SC version](contracts/Ballot.rules) of the contract,
which _simulates_ the logical behavior of the Solidity version without connection
to Ethereum network.  
It can be used as a monitor for strict run-time verification of the Solidity contract.

<details>
  <summary>Ballot (SCXML)</summary>
  <div><img src="contracts/Ballot.svg?sanitize=true"/></div>
</details>

## scenarios

- [scenario1](scenarios/scenario1.js)

  `GiveRightTo` (3 times), followed by `Vote` (3 times)

- [invalid1](scenarios/invalid1.js)

  `GiveRightTo` (3 times), followed by `Vote` (2 times)

## monitors for the contract

- [monitor1](monitors/monitor1.rules)

  ```
  protocol GiveRightTo; (GiveRightTo + Vote + Delegate)*; VotingClosed ;;
  ```

## Testing Ballot.sol

### Prerequisites

We need `truffle` and `ganache-cli` for testing.
In addition, for testing _with_ monitors, `mqtt.js` is also required.

Note that if you are not familiar with testing smart contracts on a local Ethereum network,
please take a look at [this memo](../../docs/ethereum.md).

### Testing _without_ monitors

```
$ make launch  
$ make build  
$ make test
```

- `make launch` launches `ganache-cli`, a local Ethereum network
- `make build` compiles and deploys `Ballot.sol`
- `make test` run test cases using `web3`.


### _with_ monitors

```
$ npm install mqtt  
$ ./runtest.sh scenarios/scenario1.js
```

the observer script for briding Ethereum and MQTT

## Testing Ballot.scxml
