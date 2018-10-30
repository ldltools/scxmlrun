/*
 * @source: http://blockchain.unica.it/projects/ethereum-survey/attacks.html#simpledao
 * @author: Atzei N., Bartoletti M., Cimoli T
 * Modified by Josselin Feist
 */
pragma solidity 0.4.24;

contract SimpleDAO {
  mapping (address => uint) public credit;
    
  event Donate (address indexed account, uint amount);
  event Withdraw (address indexed account, uint amount);

  function donate(address to) payable public{
    credit[to] += msg.value;

    emit Donate (to, msg.value);
  }
    
  function withdraw(uint amount) public{
    if (credit[msg.sender]>= amount) {
      emit Withdraw (msg.sender, amount);
      require (msg.sender.call.value (amount) ());
        // ** transfer <amount> from SimpleDAO to msg.sender
        //    the caller's code can call withdraw recursively
	//    this should come after the following state change
      credit[msg.sender]-=amount;
    }
  }  

  function queryCredit(address to) view public returns(uint){
    return credit[to];
  }
}
