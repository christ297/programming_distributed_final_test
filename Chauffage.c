


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SystemeCentral.h"
#include "Chauffage.h"


int main(int argc, const char *argv[])
{
    if( argc < 6)
    {
        perror("Chauffage lancer avec un nombre d'arguments insuffisants.\n"
               "Erreur dans les arguments \n"
               "Usage: $ ./Chauffage groupeMulticast portUDP adresseServeurTcp portTcp NomPiece ");

        exit(EXIT_FAILURE);
    }
    //Adresse du groupe multicast de la piece
    const char* multicast_group = argv[1];
    //Adresse du systeme central (Serveur CommunicationTemperature)
    const char* serveur_tcp_addr = argv[3];
    //Nom de la piece dont le chauffage est gerer
    const char* piece = argv[5];
    //Port réseau du groupe multicast de la piece
    unsigned int port_udp = strtol(argv[2],NULL, 10);
    //Port réseau du systeme central (Serveur CommunicationTemperature)
    unsigned int port_tcp = strtol(argv[4],NULL, 10);

    printf("-------------- Chauffage --------------------------------------------- \n");
    printf("-------------- Piece: %s ---------------------------------- \n", piece);
    printf("-------------- Groupe Multicast : %s, Port UDP : %d ------------------------------- \n", multicast_group, port_udp);
    printf("-------------- Systeme Central [Serveur CommunicationTemperature] : %s, Port TCP : %d ------------------------------------- \n \n", serveur_tcp_addr, port_tcp);

    int socket_udp, socket_tcp;
    struct sockaddr_in multicastAddr = configAdrr(port_udp);

    if((socket_udp = configurerSocketUDP(multicast_group, port_udp, multicastAddr)) != -1)
    {
        //Etat initiale du chauffage de la piece
        MessageTemperature chauffage_initiale;
        memcpy(chauffage_initiale.piece, piece, sizeof (chauffage_initiale.piece));
        chauffage_initiale.valeur = 0;
        chauffage_initiale.type = CHAUFFER;

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

                    printf("Chauffage pour la piece %s : Etat = ", chauffage_initiale.piece);
                    if (chauffage_initiale.valeur == 0)
                        printf("[ETEINT], (puissance = 0) \n");
                    else printf("[ALLUMER], puissance = %d \n", chauffage_initiale.valeur);

                }

                sleep(1);
                count++;
            }
            while (socket_tcp == -1);

            if(count > 1) printf("Service Auxilliaire [ARRETER] \n");

            printf("\nConnexion établie avec le systeme central [Serveur CommunicationTemperature en ligne] \n");

            while (1)
            {
                if(ecrireSocket(socket_tcp, socket_udp, (struct sockaddr *) &multicastAddr, &chauffage_initiale) == -2)
                {
                    perror("Connexion perdue avec le systeme central ");
                    break;
                }

                send(socket_tcp, toBytes(&chauffage_initiale), BUFFER, 0);

                printf("Chauffage pour la piece %s : Etat = ", chauffage_initiale.piece);
                if (chauffage_initiale.valeur == 0)
                    printf("[ETEINT], (puissance = 0) \n");
                else printf("[ALLUMER], puissance = %d \n", chauffage_initiale.valeur);

                sleep(1);
            }
        }
    }

    perror("Echec du lancement de Chauffage.");

    exit(EXIT_FAILURE);
}