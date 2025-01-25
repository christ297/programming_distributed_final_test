


#include "MessageTemperature.h"
#include "SystemeCentral.h"
#include "MaitreSocket.h"


#ifndef COMMANDE_H
#define COMMANDE_H


struct CommandeChauffage{
    MessageTemperature temperature;
    int socket;
};


typedef struct CommandeChauffage CommandeChauffage;



void *envoyerChauffage(void *ptr)
{
    CommandeChauffage commandeChauffage = *(CommandeChauffage *)ptr;

    int res_code = envoyerReponseTcp(commandeChauffage.socket, &commandeChauffage.temperature);

    pthread_exit((void *)&res_code);
}


int choisirNiveau()
{
    int niveau;

    do
    {
        printf("Entrez le niveau de chauffage [0 - 5] : \n");

        printf("#COMMANDE  ");
        scanf("%d", &niveau);
        if (niveau < 0 || niveau > 5) perror("Entrez un niveau de chauffage valide compris en 0 - 5 \n");
    }
    while (niveau < 0 || niveau > 5);

    return niveau;
}


int choisirTemperature()
{
    int temp;

    do
    {
        printf("Entrez le temperature souhaitez : \n");

        printf("#COMMANDE  ");
        scanf("%d", &temp);
        if (temp <= 0) perror("Entrez une temperature superieur à 0 \n");
    }
    while (temp <= 0);

    return temp;
}


char continuerDemande()
{
    char choix;

    do
    {
        printf("Voulez vous faire une nouvelle demande? \n Tapez O pour (Oui) ou Tapez N pour (Non) \n");
        scanf("%s", &choix);
        printf("#COMMANDE  ");

        if (strcasecmp(&choix, "O") != 0 && strcasecmp(&choix, "N") != 0) perror("Entrez un option valide (O) ou (N) \n");
    }
    while (strcasecmp(&choix, "O") != 0 && strcasecmp(&choix, "N") != 0);

    return choix;
}

#endif //COMMANDE_H
