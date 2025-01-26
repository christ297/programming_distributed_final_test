#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include "GestionConsole.h"
#include "SystemeCentral.h"

int main(int argc, const char *argv[])
{
    if (argc < 5)
    {
        perror("Serveur GestionConsole lancé avec un nombre d'arguments insuffisants.\n"
               "Erreur dans les arguments \n"
               "Usage: $ ./GestionConsole adresseMulticastUdp portTcp portUdp maxClient");
        exit(EXIT_FAILURE);
    }

    // Adresse du groupe multicast pour le serveur RMI
    const char *multicast_group = argv[1];
    // Port d'écoute TCP du système central (Serveur GestionConsole)
    u_int port_tcp = strtol(argv[2], NULL, 10);
    // Port de communication UDP avec le serveur RMI
    u_int port_udp = strtol(argv[3], NULL, 10);
    // Nombre maximum de consoles pouvant se connecter au système central (Serveur GestionConsole)
    u_int max_client = strtol(argv[4], NULL, 10);

    // Pour surveiller les sockets clients :
    struct pollfd *poll_fds; // Tableau de descripteurs
    int poll_size;           // Taille du tableau de descripteurs
    int poll_count;          // Nombre actuel de descripteurs dans le tableau

    // Préparation du tableau des descripteurs de fichier pour poll()
    // On commence avec assez de place pour 8 fds dans le tableau,
    // on réallouera si nécessaire
    poll_size = 8;
    poll_fds = calloc(poll_size + 1, sizeof *poll_fds);
    if (!poll_fds)
    {
        perror("Erreur d'allocation mémoire pour poll_fds");
        exit(EXIT_FAILURE);
    }

    printf("-------------- Serveur GestionConsole ------------------------------- \n");
    printf("-------------- Port TCP : %d, Port UDP : %d ---------------------- \n", port_tcp, port_udp);
    printf("-------------- Nombre d'appareils maximum : %d -------------------- \n \n", max_client);

    struct sockaddr_in udp_adrr = configAdrr(port_udp);
    struct sockaddr_in rmi_addr;
    rmi_addr.sin_family = AF_INET;
    rmi_addr.sin_port = htons(port_udp); // Correction : Utilisation de htons pour le port
    rmi_addr.sin_addr.s_addr = inet_addr(multicast_group);
    socklen_t rmi_taille = sizeof(rmi_addr);

    int socket_tcp = demarrerServeurTcp(max_client, configAdrr(port_tcp));
    int socket_udp = demarrerMulticastUdp(udp_adrr, multicast_group);
    int status;

    pthread_t pthread_1, pthread_2;

    if (socket_tcp == -1 || socket_udp == -1)
    {
        perror("Erreur lors de la création des sockets");
        free(poll_fds);
        exit(EXIT_FAILURE);
    }

    // Ajoute la socket du serveur au tableau avec alerte si la socket peut être lue
    poll_fds[0].fd = socket_tcp;
    poll_fds[0].events = POLLIN;
    poll_count = 1;

    struct sock_com_udp sock_one;
    sock_one.socket = socket_udp;
    sock_one.addr = rmi_addr;
    sock_one.addr_size = rmi_taille;

    pthread_create(&pthread_2, NULL, udpAction, (void *)&sock_one);
    pthread_detach(pthread_2);

    while (1)
    {
        // Sonde les sockets prêtes (avec timeout de 1 seconde)
        status = poll(poll_fds, poll_count, 1000);

        if (status == -1)
        {
            perror("Erreur avec le poll descripteur");
            close(socket_tcp);
            close(socket_udp);
            free(poll_fds);
            exit(EXIT_FAILURE);
        }
        else if (status == 0)
        {
            // Aucun descripteur de fichier de socket n'est prêt
            printf("... \n");
            continue;
        }

        // Boucle sur notre tableau de sockets
        for (int i = 0; i < poll_count; i++)
        {
            if (!(poll_fds[i].revents & POLLIN))
            {
                continue;
            }

            if (poll_fds[i].fd == socket_tcp)
            {
                // La socket est notre socket serveur qui écoute le port
                int socket_cli = accepterConnexionClient(socket_tcp);
                if (socket_cli == -1)
                {
                    perror("Erreur lors de l'acceptation de la connexion client");
                    continue;
                }

                add_to_poll_fds(&poll_fds, socket_cli, &poll_count, &poll_size);
                annoncerClient(socket_cli);

                int *socket_cli_ptr = malloc(sizeof(int));
                if (!socket_cli_ptr)
                {
                    perror("Erreur d'allocation mémoire pour socket_cli");
                    close(socket_cli);
                    continue;
                }

                *socket_cli_ptr = socket_cli;

                // Créer un thread pour gérer la communication avec le client
                pthread_create(&pthread_1, NULL, communicationTcp, (void *)socket_cli_ptr);
                pthread_detach(pthread_1);
            }
            else
            {
                // La socket est une socket client, on va la lire
                int socket_cli = poll_fds[i].fd;

                MessageTemperature *msg = recevoirDemandeTcp(socket_cli);
                if (msg == (MessageTemperature *)-1)
                {
                    close(socket_cli);
                    del_from_poll_fds(&poll_fds, i, &poll_count);
                }
                else if (msg == NULL)
                {
                    perror("Erreur lors de la réception du message");
                }
                else
                {
                    if (ecrireDansFichier(msg, CONSOLE_FILE_LOCATION) == -1)
                    {
                        perror("Erreur lors de l'écriture dans le fichier");
                    }
                }

                sleep(1);
            }
        }
    }

    // Nettoyage (jamais atteint dans cette boucle infinie)
    close(socket_tcp);
    close(socket_udp);
    free(poll_fds);

    return 0;
}