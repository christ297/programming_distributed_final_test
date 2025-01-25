

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "SystemeCentral.h"
#include "Thermometre.h"

int main(int argc, const char *argv[])
{
    if( argc < 6)
    {
        perror("Thermometre lancer avec un nombre d'arguments insuffisants.\n"
               "Erreur dans les arguments \n"
               "Usage: $ ./Thermometre groupeMulticast portUDP adresseServeurTcp portTcp NomPiece ");

        exit(EXIT_FAILURE);
    }
    //Adresse du groupe multicast de la piece
    const char* multicast_group = argv[1];
    //Adresse du systeme central (Serveur CommunicationTemperature)
    const char* serveur_tcp_addr = argv[3];
    //Nom de la piece dont le temperature est mesurer
    const char* piece = argv[5];
    //Port réseau du groupe multicast de la piece
    unsigned int port_udp = strtol(argv[2],NULL, 10);
    //Port réseau du systeme central (Serveur CommunicationTemperature)
    unsigned int port_tcp = strtol(argv[4],NULL, 10);

    printf("-------------- Thermometre --------------------------------------------- \n");
    printf("-------------- Piece: %s ----------------------------------------- \n", piece);
    printf("-------------- Groupe Multicast : %s, Port UDP : %d -------------------- \n", multicast_group, port_udp);
    printf("-------------- Systeme Central [Serveur CommunicationTemperature] : %s, Port TCP : %d ------------------------------------- \n \n", serveur_tcp_addr, port_tcp);

    int socket_udp, socket_tcp;

    if((socket_udp = configurerSocketUDP(multicast_group, port_udp, configAdrr(port_udp))) != -1)
    {
        while (1)
        {
            printf("En attente de connexion au systeme central [Serveur CommunicationTemperature] \n");
            int count = 0;
            do
            {
                socket_tcp = configuerSocketTCP(serveur_tcp_addr, port_tcp);
                if(count == 1) printf("Service Auxilliaire [DEMARRER] \n");
                if(count != 0)
                {
                    printf("... \n");
                    printf("Service Auxilliaire [EN COURS] \n");

                    u_char buffer[BUFFER];

                    if(lireSocket(socket_udp, buffer) == -1) perror("Erreur de reception depuis la piece ");

                    lireMessageTemperature(buffer);
                }

                sleep(1);
                count++;
            }
            while (socket_tcp == -1);

            if(count > 1) printf("Service Auxilliaire [ARRETER] \n");

            printf("\nConnexion établie avec le systeme central [Serveur CommunicationTemperature en ligne] \n");

            while (1)
            {
                u_char buffer[BUFFER];

                if(lireSocket(socket_udp, buffer) == -1)
                {
                    perror("Erreur de reception depuis la piece ");
                    continue;
                }

                char test[BUFFER];

                if(recv(socket_tcp, test, BUFFER, MSG_DONTWAIT) == 0)
                {
                    perror("Connexion perdue avec le systeme central ");
                    perror("Systeme Central deconnecter [Serveur CommunicationTemperature hors ligne] ");
                    close(socket_tcp);
                    break;
                }

                int res_code = envoyerTcp(socket_tcp, buffer);

                if (res_code == -1)
                {
                    perror("Erreur lors de l'envoie de la temperature au systeme central [Serveur CommunicationTemperature] ");
                    continue;
                }
                else if(res_code == ECONNRESET)
                {
                    perror("Connexion perdue avec le systeme central ");
                    perror("Systeme Central deconnecter [Serveur CommunicationTemperature hors ligne] ");
                    close(socket_tcp);
                    break;
                }

                sleep(1);
            }
        }
    }

    perror("Echec du lancement de Thermometre.");

    exit(EXIT_FAILURE);
}

