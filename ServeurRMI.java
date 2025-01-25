

import java.io.IOException;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class ServeurRMI
{
    /**
     * Taille du buffer des messages reçues
     */
    private static final int BUFFER_SIZE = 128;

    /**
     *
     * @param args les arguments du ServeurRMI
     */
    public static void main(String[] args)
    {
        if (args.length < 2)
        {
            System.err.println("Erreur dans les arguments !");
            System.err.println("Usage : $ java sources.java.ServeurRMI groupeMulticast portMulticast");
            System.exit(1);
        }

        String multicast = args[0];
        int port = Integer.parseInt(args[1]);

        new ServeurRMI(multicast, port);

    }

    /**
     *
     * @param multicast l'adrresse multicast
     * @param port le port multicast
     */
    public ServeurRMI(String multicast, int port)
    {
        byte[] data = new byte[BUFFER_SIZE];

        try
        {
            Registry registry = LocateRegistry.createRegistry(1099);

            Services consoleServices =
                    new ServicesImpl(data, multicast, port);

            System.out.println(consoleServices);

            registry.rebind("service", consoleServices);

            consoleServices.centralCall();

        }
        catch (IOException e)
        {
            System.err.println("Erreur lors du lancement du serveur : " + e.getMessage());
        }
    }
}
