// test case definitions (mocha)

const Ballot = artifacts.require ("Ballot");

contract ('Ballot_scenario1', async (accounts) => {

    const chairperson = accounts[0]; // web3.eth.accounts[0]
    const nproposal = 3;

    var ballot;

    // deploy
    beforeEach ('beforeEach', async () => {
	ballot = await Ballot.deployed ();
	// Ballot.deployed ().then (b => { ballot = b })
    });

    // proposals
    it ("_init", async () => {
	assert.ok (ballot.address);
	//assert.equal (ballot.chairperson (), accounts[0]);

	ballot._init (nproposal);

	console.log ("#proposal: ", nproposal);
    });

    // GiveRithtToVote: chairperson -> voter[1-3]
    it ("giveRightToVote", async () => {
	ballot.giveRightToVote (accounts[1]);
	ballot.giveRightToVote (accounts[2]);
	ballot.giveRightToVote (accounts[3]);

	console.log ("#voterl: ", 3);
    });
	
    // voter[1-3] vote
    it ("vote", async () => {
	ballot.vote (0, {from: accounts[1]});
	console.log ("voter1 vote to proposal 0");
	ballot.vote (1, {from: accounts[2]});
	console.log ("voter2 vote to proposal 1");
	ballot.vote (0, {from: accounts[3]});
	console.log ("voter3 vote to proposal 0");

	let p = await ballot.winningProposal ();
	console.log ("Winning proposal: ", p.toString (10));
    });


});

