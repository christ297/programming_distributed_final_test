#include "MaitreSocket.h"
#include "MessageTemperature.h"
#include "Console.h"
#include <errno.h>
#include <sys/select.h> // Pour select
#include <unistd.h>     // Pour close

int main(int argc, const char *argv[])
{
    // Vérification des arguments
    if (argc < 3)
    {
        perror("Console lancé avec un nombre d'arguments insuffisants.\n"
               "Erreur dans les arguments \n"
               "Usage: $ ./Console adresseServeurTcp portTcp");
        exit(EXIT_FAILURE);
    }

    // Adresse du système central (Serveur GestionConsole)
    const char *serveur_tcp_addr = argv[1];
    // Port d'écoute du système central (Serveur GestionConsole)
    unsigned int port_tcp = strtol(argv[2], NULL, 10);

    printf("----------------------- CONSOLE --------------------------------------------------------------------------------- \n");
    printf("-------------- Systeme Central [Serveur GestionConsole] : %s, Port TCP : %d ------------------------------------- \n \n", serveur_tcp_addr, port_tcp);

    int socket_tcp;

    while (1)
    {
        printf("En attente de connexion au système central [Serveur GestionConsole] \n");

        int count = 0;
        do
        {
            socket_tcp = configuerSocketTCP(serveur_tcp_addr, port_tcp);

            if (count != 0)
            {
                printf("... \n");
            }

            sleep(1);
            count++;
        } while (socket_tcp == -1);

        printf("\nConnexion établie avec le système central [Serveur GestionConsole] \n");

        while (1)
        {
            // Utilisation de select pour surveiller la socket
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(socket_tcp, &read_fds);

            // Définir un timeout de 1 seconde
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            // Surveiller la socket
            int activity = select(socket_tcp + 1, &read_fds, NULL, NULL, &timeout);

            if (activity < 0)
            {
                perror("Erreur lors de l'appel à select");
                break;
            }
            else if (activity == 0)
            {
                // Aucune donnée disponible avant la fin du timeout
                printf("Aucune donnée disponible pour l'instant.\n");
                continue;
            }

            // Des données sont disponibles sur la socket
            if (FD_ISSET(socket_tcp, &read_fds))
            {
                u_char information[BUFFER];
                ssize_t octet = recv(socket_tcp, information, BUFFER, 0); // Appel bloquant

                if (octet == 0)
                {
                    perror("Connexion perdue avec le système central");
                    printf("Système Central déconnecté [Serveur GestionConsole hors ligne]\n");
                    break;
                }
                else if (octet < 0)
                {
                    perror("Erreur lors de la lecture du message d'information depuis le système central");
                    break;
                }

                // Traitement des données reçues
                MessageTemperature *message_temperature = fromBytes(information, BUFFER);
                if (message_temperature != NULL)
                {
                    printf("Pièce = %s\n", message_temperature->piece);
                    if (message_temperature->type == MESURE)
                    {
                        printf("Mesure : Température = %d°C\n", message_temperature->valeur);
                    }
                    else
                    {
                        printf("Chauffage : Etat = ");
                        if (message_temperature->valeur == 0)
                        {
                            printf("[ETEINT], (puissance = 0)\n");
                        }
                        else
                        {
                            printf("[ALLUMÉ], puissance = %d\n", message_temperature->valeur);
                        }
                    }
                }
                else
                {
                    printf("Erreur : Données reçues invalides.\n");
                }
            }
        }

        // Fermer la socket avant de réessayer
        close(socket_tcp);
        printf("Tentative de reconnexion au système central...\n");
    }

    exit(EXIT_FAILURE);
}