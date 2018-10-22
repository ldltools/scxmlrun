# voting

## contract: [Ballot](contracts/Ballot.sol)

This example appears in the [official Solidity manual](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#voting).

## monitors

- [monitor1](monitors/monitor1.rules)

  ```
  protocol GiveRightTo; (GiveRightTo + Vote + Delegate)*; VotingClosed ;;
  ```

## scenarios

- [scenario1](scenarios/scenario1.js)

  `GiveRightTo` x3, followed by `Vote` x3

- [invalid1](scenarios/invalid1.js)

  `GiveRightTo` x3, followed by `Vote` x2


## running each scenario

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
  $ ./runtest.sh scenarios/scenario1.js
  ```
