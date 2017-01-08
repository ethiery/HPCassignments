import org.omg.CORBA.*;
import org.omg.CosNaming.*;
import org.omg.PortableServer.*;
import BankingApp.*;
import java.io.*;

public class BankServer {

    public static void main( String args[] ) 
    {   
        int bankNo = 0;
        try
        {
            BufferedReader bufferRead = new BufferedReader(new InputStreamReader(System.in));
            System.out.print("Enter bankNo: ");
            bankNo = Integer.parseInt(bufferRead.readLine());
        }
        catch (Exception e)
        {
            System.out.println("Invalid bankNo.");
            System.err.println( "Exception in BankServer Startup " + e );
            e.printStackTrace(System.out);
            System.exit(-1);
        }

        try {
            // Instantiate the ORB, resolve rootPOA and root naming context
            ORB orb = ORB.init(args, null);   
            POA rootPOA = POAHelper.narrow(orb.resolve_initial_references("RootPOA"));    
            rootPOA.the_POAManager().activate();
            NamingContextExt ncRef = NamingContextExtHelper.narrow(orb.resolve_initial_references("NameService"));

            // Resolve interbank
            // Interbank interbankRef = InterbankHelper.narrow(orb.string_to_object("corbaname::localhost:1050#InterbankServer"));
            Interbank interbankRef = InterbankHelper.narrow(ncRef.resolve_str("InterbankServer"));

            // Instantiate the servant, get corresponding reference and connect to the interbank
            BankServant bankServant = new BankServant(orb, bankNo, interbankRef);
            Bank bankRef = BankHelper.narrow(rootPOA.servant_to_reference(bankServant));
            interbankRef.connect(bankNo, bankRef);

            // Bind the reference in the naming service
            ncRef.rebind(ncRef.to_name(String.format("Bank%dServer", bankNo)), bankRef);

            // Start server
            System.out.println(String.format("Bank %d server started, kill the process to stop it.", bankNo));
            orb.run();
        } 
        catch ( Exception e ) 
        {
            System.err.println( "Exception in BankServer Startup " + e );
            e.printStackTrace(System.out);
        }
    }
}