# voting

## contract: [Ballot](contracts/Ballot.sol)

This example originally appears in some [Solidity tutorial material](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#voting).

## monitors for the contract

- [monitor1](monitors/monitor1.rules)

  ```
  protocol GiveRightTo; (GiveRightTo + Vote + Delegate)*; VotingClosed ;;
  ```

## scenarios

- [scenario1](scenarios/scenario1.js)

  `GiveRightTo` (3 times), followed by `Vote` (3 times)

- [invalid1](scenarios/invalid1.js)

  `GiveRightTo` (3 times), followed by `Vote` (2 times)


## running each scenario

Note that if you have never tested smart contracts on a local Ethereum network,
please take a look at [this memo](../../docs/ethereum.md).

- launch `ganache-cli`, a local Ethereum network

  ```
  $ make launch
  ```

- compile & deploy `Ballot.sol`

  ```
  $ make build
  ```

- run the observer script for briding Ethereum and MQTT

  ```
  $ npm install mqtt  
  $ ./runtest.sh scenarios/scenario1.js
  ```
