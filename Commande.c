
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "Commande.h"


int main(int argc, const char *argv[])
{
    if( argc < 3)
    {
        perror("Commande lancer avec un nombre d'arguments insuffisants.\n"
               "Erreur dans les arguments \n"
               "Usage: $ ./Commande adresseServeurTcp portTcp ");

        exit(EXIT_FAILURE);
    }

    //Adresse du systeme central (Serveur GestionConsole)
    const char* serveur_tcp_addr = argv[1];
    //Port d'ecoute du systeme central (Serveur GestionConsole)
    unsigned int port_tcp = strtol(argv[2],NULL, 10);
    //
    srand(time(NULL));

    printf("----------------------- COMMANDE -------------------------------------------------------------------------------- \n");
    printf("-------------- Systeme Central [Serveur GestionConsole] : %s, Port TCP : %d ------------------------------------- \n \n", serveur_tcp_addr, port_tcp);

    int socket_tcp;

    while (1)
    {
        printf("En attente de connexion au systeme central [Serveur GestionConsole] \n");

        int count = 0;
        do
        {
            socket_tcp = configuerSocketTCP(serveur_tcp_addr, port_tcp);

            if(count != 0) printf("... \n");

            sleep(1);
            count++;
        }
        while (socket_tcp == -1);

        printf("\nConnexion établie avec le systeme central [Serveur GestionConsole] \n");

        while (1)
        {
            printf("Choisissez le mode de gestion : \n");
            char mode;
            do
            {
                printf("Tapez (M) pour Manuel ou Tapez (A) pour Auto \n");
                scanf("%s", &mode);
                printf("#COMMANDE  ");

                if (strcasecmp(&mode, "M") != 0 && strcasecmp(&mode, "A") != 0) perror("Entrez un mode valide (M) ou (A) \n");
            }
            while (strcasecmp(&mode, "M") != 0 && strcasecmp(&mode, "A") != 0);

            printf("\n");

            int error = 0;

            if(strcasecmp(&mode, "M") == 0)
            {
                printf("Mode Manuel [ACTIVER] \n");

                while (1)
                {
                    char piece[100];

                    do
                    {
                        printf("Entrez le nom la piece : \n");

                        printf("#COMMANDE  ");
                        scanf("%s", piece);
                        if (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0) perror("Entrez invalide ressayer \n");
                    }
                    while (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0);

                    int niveau = choisirNiveau();

                    printf("\n");

                    MessageTemperature chauffage;
                    chauffage.valeur = niveau;
                    chauffage.type = CHAUFFER;
                    memcpy(chauffage.piece, piece, sizeof piece);

                    CommandeChauffage commande;
                    commande.temperature = chauffage;
                    commande.socket = socket_tcp;

                    u_char test[BUFFER];
                    if(recv(socket_tcp, test, BUFFER, MSG_DONTWAIT) == 0)
                    {
                        perror("Connexion perdue : System Central [Serveur GestionConsole] hors ligne");
                        error = -1;
                        break;
                    }

                    pthread_t demande_thread;
                    pthread_create(&demande_thread, NULL, envoyerChauffage, (void *)&commande);
                    pthread_join(demande_thread, NULL);

                    char choix = continuerDemande();
                    if(strcasecmp(&choix, "N") == 0) break;
                }

                printf("Mode Manuel [DESACTIVER] \n");
            }
            else
            {
                printf("Mode Auto [ACTIVER] \n");

                while (1)
                {
                    char piece[100];

                    do
                    {
                        printf("Entrez le nom la piece : \n");

                        printf("#COMMANDE  ");
                        scanf("%s", piece);
                        if (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0) perror("Entrez invalide ressayer \n");
                    }
                    while (strcmp(piece, "\n") == 0 || strcmp(piece, "\t") == 0);

                    int temperature = choisirTemperature();

                    u_char information[BUFFER];
                    ssize_t octet = recv(socket_tcp, information, BUFFER, MSG_DONTWAIT);

                    if (octet > 0)
                    {
                        MessageTemperature *message_temperature = fromBytes(information, BUFFER);

                        if (message_temperature != NULL && message_temperature->type == MESURE && strcmp(message_temperature->piece, piece) == 0)
                        {
                            MessageTemperature chauffage;
                            chauffage.type = CHAUFFER;
                            memcpy(chauffage.piece, piece, sizeof piece);

                            if(message_temperature->valeur < temperature)
                                chauffage.valeur = rand() % 5 + 1;
                            else
                            {
                                chauffage.valeur = 0;
                            }

                            CommandeChauffage commande;
                            commande.temperature = chauffage;
                            commande.socket = socket_tcp;

                            pthread_t demande_thread;
                            pthread_create(&demande_thread, NULL, envoyerChauffage, (void *)&commande);
                            pthread_join(demande_thread, NULL);
                        }
                    }
                    else if (octet == 0)
                    {
                        perror("Connexion perdue avec le systeme central ");
                        perror("Systeme Central deconnecter [Serveur GestionConsole hors ligne] ");
                        error = -1;
                        break;
                    }
                    else
                    {
                        perror("Erreur lors de la lecture du message d'information depuis le systeme central");
                    }

                    char choix = continuerDemande();
                    if(strcasecmp(&choix, "N") == 0) break;
                }

                printf("Mode Auto [DESACTIVER] \n");
            }

            sleep(1);

            if(error == -1) break;
        }
    }

    return 0;
}