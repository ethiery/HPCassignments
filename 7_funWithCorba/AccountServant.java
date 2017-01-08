import org.omg.CORBA.ORB;
import BankingApp.*;

public class AccountServant extends BankingApp.AccountPOA
{
	private final int bankNo;
	private final int accountNo;
	private int balance;

	private final BankServant bank;
	private final int secret;

	public AccountServant(int bankNo, int accountNo, int secret, BankServant bank, int balance)
	{
		this.bankNo = bankNo;
		this.accountNo = accountNo;
		this.balance = balance; 

		this.bank = bank;
		this.secret = secret;
	}

	public int getBalance()
	{
		return this.balance;
	}

  	public int getAccountNo()
  	{
  		return this.accountNo;
  	}

  	public int getBankNo()
  	{
  		return this.bankNo;
  	}

  	public Transaction[] getHistory()
  	{
  		return bank.getHistory(accountNo, secret);
  	}

	public void deposit(int amount)
	{
		balance += amount;
	}

	public void withdraw(int amount)
	{
		balance -= amount;
	}

	public void transfer(int dstBankNo, int dstAccountNo, int amount)
	{
		this.bank.requestTransfer(this.accountNo, this.secret, dstBankNo, dstAccountNo, amount);
	}

}