

import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;

public class Console
{
    public static void main(String[] args)
    {
        System.out.println("----------Lancement de ConsoleTemperature----------");
        
        try
        {
            Registry registry = LocateRegistry.getRegistry("localhost", 1099);

            Services service =
                    (Services) registry.lookup("service");

            System.out.println();

            System.out.println(service);

            System.out.println();

            System.out.println("----------ConsoleTemperature lancer avec succes--------");


            while (true)
            {
                MessageTemperature msg = service.getMessageTemperature();

                System.out.println(msg);

                System.out.println("Piece = " + msg.getPiece());

                if (msg.getType() == MessageTemperature.MESURE) System.out.printf("Mesure : Temperature = %d°C \n", msg.getValeur());
                else
                {
                    System.out.print("Chauffage : Etat = ");
                    if (msg.getValeur() == 0)
                        System.out.println("[ETEINT], (puissance = 0)");
                    else System.out.printf("[ALLUMER], puissance = %d", msg.getValeur());
                }

                Thread.sleep(1000);
            }
        }
        catch(Exception e)
        {
            System.err.println(e.getMessage() + " " + e.getCause());
        }

        System.out.println("----------Arret de ConsoleTemperature----------");
    }
}
