Contracts with several well-known vulnerabilities and
their monitors to detect their exploitation.

## Reentrancy ([SWC-107](https://smartcontractsecurity.github.io/SWC-registry/docs/SWC-107))

- [contract](contracts/SimpleDAO.sol)
- [exploit](contracts/SimpleDAO_exploit.sol)
- [monitor]()

## Transaction Order Dependence ([SWC-114](https://smartcontractsecurity.github.io/SWC-registry/docs/SWC-114))

- [contract]()
- [exploit]()
- [monitor]()

## Running scenarios

### Prerequisites

We need `truffle` and `ganache-cli` for testing.

Note that if you have never tested smart contracts on a local Ethereum network,
please take a look at [this memo](../../docs/ethereum.md).

### _without_ monitors

```
$ make launch  
$ make build  
$ make test
```

### _with_ monitors

```
$ make launch  
$ make build  
```

## References
- SmartContractSecurity,
  "Smart Contract Weakness Classification and Test Cases".
  ([link](https://smartcontractsecurity.github.io/SWC-registry/))
- ConsenSys, "Smart contract best practices".
  ([link](https://github.com/ConsenSys/smart-contract-best-practices/blob/master/docs/known_attacks.md))
- hackingdistributed.com, "Analysis of the DAO exploit", 2016.
  ([link](http://hackingdistributed.com/2016/06/18/analysis-of-the-dao-exploit/))
- trufflesuite.com, "Tutorial: exploiting the DAO", 2016.
  ([link](https://github.com/trufflesuite/trufflesuite.com/blob/master/src/tutorials/chain-forking-exploiting-the-dao.md))
- Tim Coulter, "DarkDAO", 2016.
  ([link](https://github.com/tcoulter/dao-truffle))
