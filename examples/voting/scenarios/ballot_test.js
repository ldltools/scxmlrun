//

const Ballot = artifacts.require("Ballot");

contract ('Ballot_test', async (accounts) => {

  // testcase: DEPLOY
  it ("deploy", async () => {
    let ballot = await Ballot.deployed();
    assert.ok (ballot.address);
  });

  // testcase: VOTE
  it ("vote", async () => {
    let ballot = await Ballot.deployed();

    // GiveRightTo
    // Vote


  });

});

