import org.omg.CORBA.*;
import org.omg.CosNaming.*;
import org.omg.PortableServer.*;
import BankingApp.*;
import java.util.*;
import java.io.*;


public class InterbankServant extends BankingApp.InterbankPOA
{
	List<Transaction> history;
	Map<Integer, Bank_InterbankFacing> connectedBanks;
	Map<Integer, Queue<Transaction>> queues;

	public InterbankServant(ORB orb)
	{
		this.history = new ArrayList<>();
		this.connectedBanks = new HashMap<>();
		this.queues = new HashMap<>();

		// Register a shutdown hook to save the state of the accounts on disk when the server is shutdown
        Runtime.getRuntime().addShutdownHook(new Thread() {
         	@Override
            public void run() {
				saveToDisk();
            } 
        });

		restoreFromDisk(orb);
	}

	public void connect(int bankNo, Bank_InterbankFacing bank)
	{		
		connectedBanks.put(bankNo, bank);

		// Performs logged transaction
		Queue<Transaction> queue = queues.get(bankNo);
		if (queue == null)
			return;

		while (queue.size() > 0)
		{
			Transaction t = queue.remove();
			
			if (bankNo == t.dstBankNo)
				requestTransaction(bankNo, t);

			else if (bankNo == t.srcBankNo)
				tryCompleteTransaction(t);
		}
	}

	private void queueTransaction(Transaction t)
	{
		if (!queues.containsKey(t.dstBankNo))
			queues.put(t.dstBankNo, new LinkedList<>());
		
		queues.get(t.dstBankNo).add(t);
	}

	/**
	 * Tries to init the specified transaction with the destination bank
	 * Returns true if this was done sucessfuly.
	 * Returns false else (dest bank invalidated the transaction, or 
	 * it wasn't online and the transaction was queued).
	 */
	private boolean tryInitTransaction(Transaction t)
	{
		Bank_InterbankFacing dstBank = connectedBanks.get(t.dstBankNo);
		if (dstBank == null)
		{
			queueTransaction(t);
			return false;
		}

		boolean result = false;
		try
		{
			result = dstBank.initTransaction(t);
		}
		catch (Exception e)
		{
			// If the destination bank server is down, we should remove it 
			// from connected banks and queue the transaction
			connectedBanks.remove(t.dstBankNo);
			queueTransaction(t);
		}

		return result;
	}

	/**
	 * Tries to complete the specified transaction with the destination bank
	 * In case of success, logs it in the history.
	 * In case of failure (src bank isn't online), queues it for later completion
	 */
	private void tryCompleteTransaction(Transaction t)
	{
		Bank_InterbankFacing srcBank = connectedBanks.get(t.srcBankNo);
		if (srcBank == null)
		{
			queueTransaction(t);
			return;
		}

		try
		{
			srcBank.completeTransaction(t);
			history.add(t);
		}
		catch (Exception e)
		{
			// If the src bank server is down, we should remove it 
			// from connected banks and queue the transaction
			connectedBanks.remove(t.srcBankNo);
			queueTransaction(t);
		}
	}

	public void requestTransaction(int bankNo, Transaction t)
	{
		// Step 1, verifiy the src bank
		// This is of course non secure, but this is out of the scope of this project
		// Here we suppose that bankNo is enough to authenticate the bank
		if (bankNo != t.srcBankNo)
			return;

		// Step 2, init the transaction with the destination bank
		boolean step2OK = tryInitTransaction(t);

		// Step 3, complete the transaction with the source bank and log to history
		if (step2OK)
			tryCompleteTransaction(t);
	}
	
	public Transaction[] getHistory(int bankNo)
	{
		List<Transaction> bankHistory = new ArrayList<>();
		for (Transaction t : history)
		{
			if (t.srcBankNo == bankNo || t.dstBankNo == bankNo)
				bankHistory.add(t);
		}

		return bankHistory.toArray(new Transaction[bankHistory.size()]);
	}

	// private

	private void saveToDisk()
	{
		try {
    		PrintWriter writer = new PrintWriter("interbank.dat", "UTF-8");
    		
    		// connected banks
    		writer.println(String.format("%d", connectedBanks.size()));
    		for (Integer i : connectedBanks.keySet())
    			writer.println(String.format("%d ", i));

    		// queues
			writer.println(String.format("%d", queues.size()));
    		for (Map.Entry<Integer, Queue<Transaction>> entry : queues.entrySet())
    		{
    			int bankNo = entry.getKey();
    			Queue<Transaction> queue = entry.getValue(); 
    			writer.println(String.format("%d", bankNo));
    			writer.println(String.format("%d", queue.size()));
    			while (queue.size() > 0)
    			{
    				Transaction t = queue.remove();
	    			writer.println(String.format("%d %d %d %d %d", t.dstBankNo, t.dstAccountNo, t.srcBankNo, t.srcAccountNo, t.amount));
	    		}
    		}

    		// history
    		writer.println(String.format("%d", history.size()));
    		for (Transaction t : history)
	    		writer.println(String.format("%d %d %d %d %d", t.dstBankNo, t.dstAccountNo, t.srcBankNo, t.srcAccountNo, t.amount));

    		writer.close();
		} 
		catch (IOException e) {
			System.err.println("Exception in BankServant.java..." + e);
            e.printStackTrace();
		}
	}

	private Transaction nextTransaction(Scanner s)
	{
		int dstBankNo = s.nextInt();
    	int dstAccountNo = s.nextInt();
		int srcBankNo = s.nextInt();
		int srcAccountNo = s.nextInt();
		int amount = s.nextInt();
		return new Transaction(dstBankNo, dstAccountNo, srcBankNo, srcAccountNo, amount);
	}

	private void restoreFromDisk(ORB orb)
	{
		File dbFile = new File("interbank.dat");
		if (!dbFile.exists())
			return;



		List<Integer> connectedBankNos = new ArrayList<>();

		try 
		{
			// connected bank numbers
			Scanner s = new Scanner(new FileInputStream(dbFile));
			int nbConnectedBanks = s.nextInt();
			for (int i = 0; i < nbConnectedBanks; i++)
				connectedBankNos.add(s.nextInt());

			// queues
			int nbQueues = s.nextInt();
    		for (int i = 0; i < nbQueues; i++)
    		{
    			int bankNo = s.nextInt();
    			int nbTransactions = s.nextInt();
    			Queue<Transaction> queue = new LinkedList<>(); 
    			for (int j = 0; j < nbTransactions; j++)
    				queue.add(nextTransaction(s));
    			queues.put(bankNo, queue);
    		}

    		// history
    		int historySize = s.nextInt();
    		for (int i = 0; i < historySize; i++)
    			history.add(nextTransaction(s));

			s.close();
		}
		catch (IOException e) {
			System.err.println("Exception in BankServant.java..." + e);
            e.printStackTrace();
            return;
		}

		try
		{
			NamingContextExt ncRef = NamingContextExtHelper.narrow(orb.resolve_initial_references("NameService"));

			for (Integer bankNo : connectedBankNos)
			{
				org.omg.CORBA.Object objRef = ncRef.resolve_str(String.format("Bank%dServer", bankNo));
				if (objRef != null)
					connectedBanks.put(bankNo, Bank_InterbankFacingHelper.narrow(objRef));
			}
		}
		catch (Exception e) 
        {
            System.err.println( "Exception in BankServer Startup " + e );
            e.printStackTrace(System.out);
            return;
        }
	}

}