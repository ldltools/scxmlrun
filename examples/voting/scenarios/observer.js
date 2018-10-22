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
const address = json.networks[network].address;
const Contract = web3.eth.contract (abi);
const contract = Contract.at (address);

// mqtt
const MQTT = require ('mqtt');
const mqtt_host = "127.0.0.1";
const mqtt_port = "1883";
//const clientid = "client" + new Date ().getTime ();
//const mqtt = new Paho.MQTT.Client (host, Number (port), clientid);
//const mqtt = new MQTT.Client (mqtt_host, Number (mqtt_port), clientid);
const mqtt = MQTT.connect ("mqtt://" + mqtt_host + ":" + mqtt_port);
//const mqtt = MQTT.connect ("ws:" + mqtt_host + ":" + "9001");
mqtt.on ('connect', function () { console.log ('mqtt connected'); });
console.log ('mqtt connecting: ' + mqtt_host + ':' + mqtt_port);

// event listeners
const opts = {fromBlock: 'latest', toBlock: 'latest'};
const mqtt_topic = "voting";

contract.GiveRightTo ({}, opts, function (err, log) {
    if (err) { console.log ("err:", err); return; }
    payload = "{\"event\":{";
    payload += "\"name\":\"" + log.event + "\"";
    payload += ", \"data\":\"" + log.args._voter + "\"";
    payload += "}}";
    mqtt.publish (mqtt_topic, payload);
    console.log ("event:", log.event, {"voter" : log.args._voter});
});

contract.Delegate ({}, opts, function (err, log) {
    if (err) { console.log ("err:", err); return; }
    w = log.args._weight.toNumber ();
    payload = "{\"event\":{";
    payload += "\"name\":\"" + log.event + "\"";
    payload += ", \"data\":" + w;
    payload += "}}";
    mqtt.publish (mqtt_topic, payload);
    console.log ("event:", log.event, {"weight" : w});
});

contract.Vote ({}, opts, function (err, log) {
    if (err) { console.log ("err:", err); return; }
    p = log.args._proposal.toNumber ();
    w = log.args._weight.toNumber ();
    payload = "{\"event\":{";
    payload += "\"name\":\"" + log.event + "\"";
    payload += ", \"data\":{\"proposal\":" + p + ", \"weight\":" + w + "}";
    payload += "}}";
    mqtt.publish (mqtt_topic, payload);
    console.log ("event:", log.event, {"proposal" : p, "weight" : w});
});

contract.VotingClosed ({}, opts, function (err, log) {
    if (err) { console.log ("err:", err); return; }
    payload = "{\"event\":{\"name\":\"" + log.event + "\"}}";
    mqtt.publish (mqtt_topic, payload);
    console.log ("event:", log.event);
});
