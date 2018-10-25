This memo explains how to compile, deploy, and run smart contracts on a local ethereum network.  
It also discusses event transmission from Ethereum to SCXML statecharts running with `scxmlrun`.

## [Ganache](https://truffleframework.com/ganache): local ethereum network for testing

```
$ npm install -g ganache-cli  
$ ganache-cli &
```

This launches a new local ethereum network (at port 7545 by default).

## Contract compilation and deployment

### [Truffle](https://truffleframework.com/) (Solidity)

```
$ truffle compile && truffle deploy
```

### [web3](https://github.com/ethereum/web3.js/)

## Connection between Ethereum and MQTT

At this moment, there is no 'off-the-shelf' tool but we are on our own.  

However, in order to send events from Solidity contracts to a MQTT broker,
we just need a _bridge_ script that performs
the following operations using
[web3.js](https://github.com/ethereum/web3.js/) and
[mqtt.js](https://github.com/mqttjs).

- listen to events emitted by contracts using `web3.eth`.
- publish them to a MQTT broker using the `MQTT.Client#publish` method.

Specifically, for each Solidity event _Event_ emitted by _contract_,
you can define a lister with a callback that looks like:

```
contract.Event ({...}, {fromBlock:'latest', toBlock:'latest'}, function (err, log) {  
  payload = "{\"event\":{\"name\":\"" + log.event + "\",...}}";  
  mqtt.publish (topic, payload);  
})
```

## References
