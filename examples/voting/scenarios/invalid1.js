//

const Web3 = require ('web3');
const web3 = new Web3 ();
console.log ("web3:", web3.version.api);
const host = 'localhost';
const port = '7545';
const network = '5777';
web3.setProvider (new web3.providers.HttpProvider ('http://' + host + ':' + port));
//web3.eth.getAccounts (console.log);
// default sender
web3.eth.defaultAccount = web3.eth.accounts[0];

// contract
const fs = require ('fs');
const json = JSON.parse (fs.readFileSync ('../build/contracts/Ballot.json', 'utf8'));
const abi = json.abi;
//console.log (JSON.stringify (abi));
const address = json.networks[network].address;
//console.log (address);
const Contract = web3.eth.contract (abi);
const contract = Contract.at (address);

// --------------------------------------------------------------------------------
console.log ("proposals:", contract.proposals);
console.log ("chairperson:", contract.chairperson);
console.log ("voter[0]:", contract.voters (web3.eth.accounts[0]));

// GiveRightTo
for (var i = 1; i < 4; i++)
{
    console.log ("giveRightToVote:", i);
    addr = web3.eth.accounts[i];
    contract.giveRightToVote (web3.eth.accounts[i]);
}

// Vote
console.log ("acc1.vote (0)");
//web3.eth.sendTransaction ({data: 0, from: web3.eth.accounts[1], to: contract.address});
contract.vote.sendTransaction (0, {from: web3.eth.accounts[1], gas: 100000});
console.log ("acc2.vote (0)");
contract.vote.sendTransaction (0, {from: web3.eth.accounts[2], gas: 100000});
//console.log ("acc3.vote (1)");
//contract.vote.sendTransaction (1, {from: web3.eth.accounts[3], gas: 100000});

const winner = contract.winnerName.call ();
//console.log ("result:", winner);
console.log ("result:", web3.toUtf8 (winner));

// --------------------------------------------------------------------------------

// mqtt
const MQTT = require ('mqtt');
const mqtt_host = "127.0.0.1";
const mqtt_port = "1883";
const mqtt = MQTT.connect ("mqtt://" + mqtt_host + ":" + mqtt_port);
const mqtt_topic = "voting";
mqtt.on ('connect', function () {
    payload = "{\"event\":{\"name\":\"_accept\"}}";
    mqtt.publish (mqtt_topic, payload);
    mqtt.end ();
});

