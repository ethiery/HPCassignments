import java.util.Properties;
import org.omg.CORBA.Object;
import org.omg.CORBA.ORB;
import org.omg.CosNaming.NameComponent;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;
import org.omg.CORBA.Policy;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.*;
import org.omg.PortableServer.Servant;
import BankingApp.*;

public class InterbankServer 
{
    public static void main(String args[]) 
    {  
        try {
            // Instantiate the ORB, resolve rootPOA, create a persistant POA, 
            // and resolve root naming context (using persistent name service)
            ORB orb = ORB.init(args, null);
            POA rootPOA = POAHelper.narrow(orb.resolve_initial_references("RootPOA"));
            Policy[] persistentPolicy = new Policy[] { rootPOA.create_lifespan_policy(LifespanPolicyValue.PERSISTENT) };
            POA persistentPOA = rootPOA.create_POA("childPOA", null, persistentPolicy); 
            persistentPOA.the_POAManager().activate();
            NamingContextExt ncRef = NamingContextExtHelper.narrow(orb.resolve_initial_references("NameService"));

            // Instantiate, active and bind the servant
            InterbankServant interbankServant = new InterbankServant(orb);
            persistentPOA.activate_object(interbankServant);
            Interbank interbankRef = InterbankHelper.narrow(persistentPOA.servant_to_reference(interbankServant));
            ncRef.rebind(ncRef.to_name("InterbankServer"), interbankRef);

            // Start server
            orb.run();
        } 
        catch ( Exception e ) 
        {
            System.err.println( "Exception in InterbankServer Startup " + e );
            e.printStackTrace(System.out);
        }
    }
}