



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CommunicationTemperature.h"


int main(int argc, char *argv[])
{
    if( argc < 3)
    {
        perror("Serveur CommunicationTemperature lancer avec un nombre d'arguments insuffisants.\n"
               "Erreur dans les arguments \n"
               "Usage: $ ./CommunicationTemperature portTcp maxClient ");

        exit(EXIT_FAILURE);
    }
    //Port d'ecoute tcp du Systeme Central
    unsigned int port = strtol(argv[1],NULL, 10);
    //Nombre maximum de chauffage/thermometre pouvant se connecter au systeme central (Serveur CommunicationTemperature)
    unsigned int max_client = strtol(argv[2],NULL, 10);

    // Pour surveiller les sockets clients :
    struct pollfd *poll_fds; // Tableau de descripteurs
    int poll_size; // Taille du tableau de descipteurs
    int poll_count; // Nombre actuel de descripteurs dans le tableau

    // Préparation du tableau des descripteurs de fichier pour poll()
    // On va commencer avec assez de place pour 8 fds dans le tableau,
    // on réallouera si nécessaire
    poll_size = 8;
    poll_fds = calloc(poll_size + 1, sizeof *poll_fds);
    if (!poll_fds)
    {
        return (4);
    }

    printf("-------------- Systeme Central [Serveur CommunicationTemperature] ------------------------ \n");
    printf("-------------- Ecoute sur le port : %d --------------------------------------------- \n", port);
    printf("-------------- Nombre d'appareil maximum : %d ----------------------------------- \n \n", max_client);

    int socket_tcp;
    if((socket_tcp = demarrerServeurTcp(max_client, configAdrr(port))) != -1)
    {
        // Ajoute la socket du serveur au tableau
        // avec alerte si la socket peut être lue
        poll_fds[0].fd = socket_tcp;
        poll_fds[0].events = POLLIN;
        poll_count = 1;

        int status;

        printf("En attente de connexion d'un appareil \n");

        while (1)
        {
            // Sonde les sockets prêtes (avec timeout de 1 secondes)
            status = poll(poll_fds, poll_count, 1000);

            if (status == -1)
            {
                perror("Erreur avec le poll descripteur ");
                exit(EXIT_FAILURE);
            }
            else if (status == 0)
            {
                // Aucun descipteur de fichier de socket n'est prêt
                printf("... \n");
                continue;
            }
            // Boucle sur notre tableau de sockets
            for (int i = 0; i < poll_count; i++)
            {
                if ((poll_fds[i].revents & POLLIN) != 1)
                {
                    continue;
                }

                if (poll_fds[i].fd == socket_tcp)
                {
                    // La socket est notre socket serveur qui écoute le port
                    int socket_client = accepterConnexionClient(socket_tcp);
                    add_to_poll_fds(&poll_fds, socket_client, &poll_count, &poll_size);
                    annoncerClient(&socket_client);
                }
                else
                {
                    // La socket est une socket client, on va la lire
                    int socket_cli = (poll_fds)[i].fd;

                    if (serviceClient(socket_cli) == 1)
                    {
                        close(socket_cli);
                        del_from_poll_fds(&poll_fds, i, &poll_count);
                    }
                    else
                    {
                        pthread_t demande_chauffage_thread;

                        int resultat;

                        pthread_create(&demande_chauffage_thread, NULL, communicationConsole, (void *) socket_cli);

                        pthread_join(demande_chauffage_thread, (void *)&resultat);

                        if(resultat == -1)
                        {
                            close(socket_cli);
                            del_from_poll_fds(&poll_fds, i, &poll_count);
                        }
                    }
                }
            }

            sleep(1);
        }
    }

    perror("Echec du lancement de CommunicationTemperature.");

    return EXIT_FAILURE;
}

