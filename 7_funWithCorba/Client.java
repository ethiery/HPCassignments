import org.omg.CORBA.*;
import org.omg.CosNaming.*;
import org.omg.PortableServer.*;
import BankingApp.*;
import java.io.*;

public class Client {

    private static BufferedReader bufferRead = new BufferedReader(new InputStreamReader(System.in));

    private static int getInt(String prompt, String errorMessage)
    {
        int val = -1;
        while (val == -1)
        {
            try
            {
            System.out.print(prompt);
            val = Integer.parseInt(bufferRead.readLine());
            }
            catch (Exception e)
            {
                System.out.println(errorMessage);
            }
        }
        return val;
    }

    private static Bank getBankRef(String args[], int bankNo)
    {
        Bank bankRef = null;
        try {
            // Instantiate the ORB, resolve root naming context
            ORB orb = ORB.init(args, null);   
            NamingContextExt ncRef = NamingContextExtHelper.narrow(orb.resolve_initial_references("NameService"));

            // Resolve bank
            bankRef = BankHelper.narrow(ncRef.resolve_str(String.format("Bank%dServer", bankNo)));
            System.out.println(String.format("Connected to the server of bank %d", bankNo));
        } 
        catch ( Exception e ) 
        {
            System.out.println(String.format("Error while trying to contact the server of bank %d", bankNo));
            System.err.println("Exception in Client:getBankRef() " + e );
            e.printStackTrace(System.out);
        }
        return bankRef;
    }

    private static void accountCreation(Bank bankRef)
    {
        int secret = getInt("Please enter a digital code to protect your account: ", "Invalid code");
        int accountNo = bankRef.createAccount(secret);
        System.out.println(String.format("Account created successfuly, your account number is %d", accountNo));
    }

    private static Account connection(Bank bankRef)
    {
        Account accountRef = null;
        while (accountRef == null)
        {
            int accountNo = getInt("Please enter your account number: ", "Invalid number");
            int secret = getInt("Please enter your digital code: ", "Invalid code");
            accountRef = bankRef.connect(accountNo, secret);
            if (accountRef == null)
                System.out.println("Invalid credientials, please try again.");
            else
                System.out.println("Connection succesful");
        }
        return accountRef;
    }

    private static Account connectionMenu(Bank bankRef)
    {
        Account accountRef = null;
        while (accountRef == null)
        {
            System.out.println("1. Create an account");
            System.out.println("2. Connect to your account");
            int choice = getInt("Please enter your choice: ", "Invalid choice");
            switch (choice)
            {
                case 1:
                    accountCreation(bankRef);
                    break;

                case 2:
                    accountRef = connection(bankRef);
                    break;

                default:
                    System.out.println("Invalid choice");
                    break;
            }
        }
        return accountRef;
    }

    private static void actionMenu(Account accountRef)
    {
        boolean stop = false;
        int choice, amount, dstBankNo, dstAccountNo;
        while (!stop)
        {
            System.out.println("1. Get balance");
            System.out.println("2. Get account no");
            System.out.println("3. Get bank no");
            System.out.println("4. Get interbank transfer history");
            System.out.println("5. Withdrawal");
            System.out.println("6. Deposit");
            System.out.println("7. Transfer");
            System.out.println("8. Quit");

            choice = getInt("Please enter your choice: ", "Invalid choice");
            switch (choice)
            {
                case 1:
                    System.out.println(String.format("Your balance is %d", accountRef.getBalance()));
                    break;

                case 2:
                    System.out.println(String.format("Your account no is %d", accountRef.getAccountNo()));
                    break;

                case 3:
                    System.out.println(String.format("Your bank no is %d", accountRef.getBankNo()));
                    break;

                case 4:
                    Transaction[] history = accountRef.getHistory();
                    System.out.println("Account interbank transfer history:");
                    for (Transaction t : history)
                        System.out.println(String.format("Bank %d account %d -> Bank %d account %d, amount %d", 
                            t.srcBankNo, t.srcAccountNo, t.dstBankNo, t.dstAccountNo, t.amount));
                    break;

                case 5:
                    amount = getInt("Please enter the amount of your withdrawal: ", "Invalid amount");
                    accountRef.withdraw(amount);
                    System.out.println(String.format("Your new balance is %d", accountRef.getBalance()));
                    break;

                case 6:
                    amount = getInt("Please enter the amount of your deposit: ", "Invalid amount");
                    accountRef.deposit(amount);
                    System.out.println(String.format("Your new balance is %d", accountRef.getBalance()));
                    break;

                case 7:
                    amount = getInt("Please enter the amount of your transfer: ", "Invalid amount");
                    dstBankNo = getInt("Please enter the destination bank number: ", "Invalid number");
                    dstAccountNo = getInt("Please enter the destination account number: ", "Invalid number");
                    accountRef.transfer(dstBankNo, dstAccountNo, amount);
                    System.out.println("Transfer initiated");
                    break;

                case 8:
                    stop = true;
                    break;

                default:
                    System.out.println("Invalid choice");
                    break;
            }
        }
    }

    public static void main( String args[] ) 
    {   
        System.out.print("WELCOME\n\n");

        int bankNo = getInt("Please enter the bankNo of your bank: ", "Invalid bankNo.");
        Bank bankRef = getBankRef(args, bankNo);
        Account accountRef = connectionMenu(bankRef);

        actionMenu(accountRef);
        System.out.print("GOODBYE\n\n");

    }
}