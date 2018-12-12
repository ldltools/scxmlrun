# voting

## Contract: [Ballot](contracts/Ballot.sol) in Solidity

[This contract](contracts/Ballot.sol) originally appears in [this Solidity tutorial material](https://solidity.readthedocs.io/en/v0.4.24/solidity-by-example.html#voting).

The contract is for selecting one out of several proposals (on a majority vote)
through a series of 3 different sorts of transactions, namely
`giveRightToVote`, `delegate`, and `vote`.

### DSL4SC/SCXML version of Ballot (optional)

In addition to the Solidity version,
we also provide another [DSL4SC version](contracts/Ballot.rules) of the contract:

- it can replace `Ballot.sol` and run as an Ethereum smart contract (when combined with [`proxy.sol`](contracts/proxy.sol)).
- alternatively, it can coexist with `Ballot.sol` and run as a _monitor_ for `Ballot.sol`.

![Ballot (SCXML)](contracts/Ballot.svg?sanitize=true)

## Scenarios

- [scenario1](scenarios/scenario1.js)

  `GiveRightTo` (voter1-3), followed by `Vote` (voter1-3)

  The winning proposal should be: `proposal0`

- [scenario1](scenarios/scenario2.js)

  `GiveRightTo` (voter1-3), followed by `Vote` (voter1-2) and `Delegate` (voter3)

  The winning proposal should be: `proposal1`

- [invalid1](scenarios/invalid1.js)

  `GiveRightTo` (voter1-3), followed by `Vote` (voter1-2)

  This should fail since the voting terminates without a vote of voter3.

## Monitors

- [monitor1](monitors/monitor1.rules)

  This monitor ensures that each voting process terminates with a `VotingClosed` event.

  ```
  protocol GiveRightTo; (GiveRightTo + Vote + Delegate)*; VotingClosed ;;
  ```

  <details>
    <summary>monitor1 (SCXML)</summary>
    <div><img src="monitors/monitor1.svg?sanitize=true"/></div>
  </details>

- [monitor2](monitors/monitor2.rules)

  (to be filled in)

## Testing Ballot.sol

### Prerequisites

We need `truffle` and `ganache-cli` for testing.
In addition, for testing _with_ monitors, `mqtt.js` is also required.

Note that if you are not familiar with testing smart contracts on a local Ethereum network,
please take a look at [this memo](../../docs/ethereum.md).

### Testing Ballot.sol _without_ monitors

(This is pure Solidity contract testing and has nothing to do with `scxmlrun`.)

```
$ make launch  
$ make build  
$ make test
```

<details>
  <summary>Remarks</summary>
  <div>
    <ul>
      <li>`make launch` launches `ganache`, a local Ethereum network</li>
      <li>`make build` compiles and deploys `Ballot.sol`</li>
      <li>`make test` run test cases using `web3.js`</li>
    </ul?
  </div>
</details>

### Testing Ballot.sol _with_ monitors in SCXML

`Ballot.sol` can be _monitored_ in the following manner.

```
$ make launch  
$ make build  
$ ./runtest.sh monitors/monitor1.scxml scenarios/scenario1.js
```

In the above exmple,
events emitted by `Ballot.sol` are
sent to `monitor1.scxml` through
an [_observer_ script](monitors/observer.js)
which bridges Ethereum and MQTT.

<details>
  <summary>contract and monitor</summary>
  <div><img src="monitors/observer.png"/></div>
</details>

## Testing Ballot.scxml

(to be filled in)
