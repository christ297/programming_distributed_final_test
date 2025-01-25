


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include "SystemeCentral.h"

#ifndef OSIRIS_COMMUNICATIONTEMPERATURE_H
#define OSIRIS_COMMUNICATIONTEMPERATURE_H

/**
 * Relaie entre les pieces et le systeme central
 * @param socket_client la socket de la piece(Chauffage ou Thermometre)
 * @return 0 si tout se passe bien ou -1 en cas d'erreur et 1 si le client se deconnecte
 */
int serviceClient(int socket_client);

/**
 * Routine executer dans le thread des comminications avec les pieces
 * @param communication_socket l'adresse de la variable a traiter
 * @return un pointeur sur void
 */
void* communicationConsole(void *communication_socket);



int serviceClient(int socket_client)
{
    MessageTemperature *temp_from_cli_socket = recevoirDemandeTcp(socket_client);

    if (temp_from_cli_socket == NULL) return -1;//return ((void *) -1);
    if (temp_from_cli_socket == ((void *) -1)) return 1;

    printf("MESSAGE [ ");
    printf("Piece = %s, ", temp_from_cli_socket->piece);
    if(temp_from_cli_socket->type == MESURE) printf("Mesure : Temperature = %d°C ] \n", temp_from_cli_socket->valeur);
    else printf("Chauffage : Puissance = %d ] \n", temp_from_cli_socket->valeur);

    if (ecrireDansFichier(temp_from_cli_socket, PIECE_FILE_LOCATION) != 0) {
        perror("Erreur lors de l'ecriture dans le fichier ");
        return -1;
    }

    return 0;
}

void* communicationConsole(void *communication_socket)
{
    int socket_client = *(int *)&communication_socket;

    u_char buffer[BUFFER];
    MessageTemperature *msg = lireDansFichier(buffer, CONSOLE_FILE_LOCATION);

    if(msg == NULL) pthread_exit(((void *) -1));

    char test[BUFFER];

    if(recv(socket_client, test, BUFFER, MSG_DONTWAIT) == 0)
    {
        perror("Connexion perdue avec un appareil ");
        perror("Appareil deconnecter ");
        close(socket_client);
        return ((void *) -1);
    }

    if(envoyerReponseTcp(socket_client, msg) == -1) return ((void *) -1);

    return (void *)0;
}

#endif //OSIRIS_COMMUNICATIONTEMPERATURE_H
