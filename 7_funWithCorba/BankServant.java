import org.omg.CORBA.ORB;
import BankingApp.*;
import java.util.*;
import java.io.*;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.*;

public class BankServant extends BankingApp.BankPOA
{
	private POA rootPOA;

	private final Interbank interbank;
	private final int bankNo;

	private List<AccountServant> accounts;
	private List<Integer> secrets;

	public BankServant(ORB orb, int bankNo, Interbank interbank)
	{
		this.bankNo = bankNo;
		this.interbank = interbank;
		this.accounts = new ArrayList<>();
		this.secrets = new ArrayList<>();

		// Register a shutdown hook to save the state of the accounts on disk when the server is shutdown
        Runtime.getRuntime().addShutdownHook(new Thread() {
         	@Override
            public void run() {
				saveToDisk();
            } 
        });

		restoreFromDisk();

		// Registers to the interbank to be able to receive transactions from
		// other banks
		this.rootPOA = null;
		try
		{
			this.rootPOA = POAHelper.narrow(orb.resolve_initial_references("RootPOA"));
			this.rootPOA.the_POAManager().activate();
			Bank bankRef = BankHelper.narrow(this.rootPOA.servant_to_reference(this));
			interbank.connect(bankNo, bankRef);
		} 
		catch (Exception e) 
		{
			System.err.println("Exception in BankServant.java..." + e);
			e.printStackTrace();
		}
	}

	// Customer facing 
	 
	public int createAccount(int secret)
	{
		int accountNo = accounts.size();
		accounts.add(new AccountServant(this.bankNo, accountNo, secret, this, 0));
		secrets.add(secret);

		return accountNo;
	}

	public Account connect(int accountNo, int secret)
	{
		if (accountNo >= secrets.size() || secrets.get(accountNo) != secret)
			return null;

		AccountServant accountServant = accounts.get(accountNo);
		Account account = null;
		try
		{
			account = AccountHelper.narrow(rootPOA.servant_to_reference(accountServant));
		}
		catch (Exception e) 
    	{
            System.err.println("Exception in BankServant.java..." + e);
            e.printStackTrace();
        }

		return account;
	}

	// Interbank facing

	public boolean initTransaction(Transaction t)
	{
		if (this.bankNo != t.dstBankNo)
			return false;

		if (t.dstAccountNo >= accounts.size())
			return false;

		accounts.get(t.dstAccountNo).deposit(t.amount);

		return true;
	}

  	public void completeTransaction(Transaction t)
  	{
  		assert(t.srcAccountNo < accounts.size());
		accounts.get(t.srcAccountNo).withdraw(t.amount);
  	}

  	// Private

  	public void requestTransfer(int srcAccountNo, int secret, int dstBankNo, int dstAccountNo, int amount)
	{
		if (srcAccountNo >= secrets.size() || secrets.get(srcAccountNo) != secret)
			return;

		if (dstBankNo == this.bankNo)
		{
			if (dstAccountNo < secrets.size())
			{
				accounts.get(dstAccountNo).deposit(amount);
				accounts.get(srcAccountNo).withdraw(amount);				
			}
		}
		else
		{
			Transaction transaction = new Transaction(dstBankNo, dstAccountNo, this.bankNo, srcAccountNo, amount);
			interbank.requestTransaction(this.bankNo, transaction);
		}
	}

	public Transaction[] getHistory(int accountNo, int secret)
	{
		if (accountNo >= secrets.size() || secrets.get(accountNo) != secret)
			return new Transaction[0];

		Transaction[] bankHistory = interbank.getHistory(bankNo);

		List<Transaction> clientHistory = new ArrayList<>();
		for (Transaction t : bankHistory)
		{
			if ((t.srcBankNo == bankNo && t.srcAccountNo == accountNo) || (t.dstBankNo == bankNo && t.dstAccountNo == accountNo))
				clientHistory.add(t);
		}

		return clientHistory.toArray(new Transaction[clientHistory.size()]);
	}

	private void saveToDisk()
	{
		try {
    		PrintWriter writer = new PrintWriter(String.format("bank%d.dat", bankNo), "UTF-8");
			for (int i = 0; i < accounts.size(); i++)
			{
    			writer.println(String.format("%d %d %d", i, secrets.get(i), accounts.get(i).getBalance()));
			}
    		writer.close();
		} 
		catch (IOException e) {
			System.err.println("Exception in BankServant.java..." + e);
            e.printStackTrace();
		}
	}

	private void restoreFromDisk()
	{
		try {
			// Restore previous state
			File dbFile = new File(String.format("bank%d.dat", bankNo));
			
			if (dbFile.exists()) 
			{ 
				Scanner s = new Scanner(new FileInputStream(dbFile));
				int accountNo, secret, balance;
				while (s.hasNextInt()) 
				{
					accountNo = s.nextInt();
					secret = s.nextInt();
					balance = s.nextInt();
					accounts.add(new AccountServant(this.bankNo, accountNo, secret, this, balance));
					secrets.add(secret);
				}
				s.close();
			}
		}
		catch (IOException e) {
			System.err.println("Exception in BankServant.java..." + e);
            e.printStackTrace();
		}
	}

}