

import java.io.IOException;
import java.rmi.Remote;
import java.util.concurrent.ExecutionException;

/**
 * Service distant pour les consoles connecter au Systeme Central [Serveur Rmi]
 * pour communiquer
 */
public interface Services extends Remote
{
    /**
     * Recevoir un message de temperature depuis le Systeme Central [Serveur GestionConsole]
     *
     * @return Un message de temperature {@link MessageTemperature}
     * @throws IOException si une erreur survient lors de la reception des packets
     * @throws ExecutionException si erreur survient dans le thread de reception
     * @throws InterruptedException si le thread est interromput en plein execution
     */
    MessageTemperature getMessageTemperature() throws IOException, ExecutionException, InterruptedException;

    /**
     * Informer le Systeme Central [Serveur GestionConsole]
     * que le Serveur Rmi [Systeme Central] est lancer et pret a recevoir les données
     *
     * @throws IOException si une erreur survient lors de l'envoi du packets
     */
    void centralCall() throws IOException;
}
