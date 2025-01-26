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

pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

int serviceClient(int socket_client) {
    MessageTemperature *temp_from_cli_socket = recevoirDemandeTcp(socket_client);

    if (temp_from_cli_socket == NULL) return -1;
    if (temp_from_cli_socket == (MessageTemperature*)-1) return 1;

    printf("MESSAGE [ Piece = %s, ", temp_from_cli_socket->piece);
    if (temp_from_cli_socket->type == MESURE) {
        printf("Mesure : Temperature = %d°C ]\n", temp_from_cli_socket->valeur);
    } else {
        printf("Chauffage : Puissance = %d ]\n", temp_from_cli_socket->valeur);
    }

    // Utilisation de 'const char*'
    if (ecrireDansFichier(temp_from_cli_socket, PIECE_FILE_LOCATION) != 0) {
        perror("Erreur écriture fichier piece");
        return -1;
    }

    return 0;
}

void* communicationConsole(void *communication_socket) {
    int socket_client = *((int *)communication_socket);
    free(communication_socket);

    u_char buffer[BUFFER];
    MessageTemperature *msg = NULL;
    
    pthread_mutex_lock(&socket_mutex);
    if (socket_client < 0) {
        pthread_mutex_unlock(&socket_mutex);
        return (void*)-1;
    }
    
    // Utilisation de 'const char*'
    msg = lireDansFichier(buffer, CONSOLE_FILE_LOCATION);
    pthread_mutex_unlock(&socket_mutex);

    if (!msg) {
        perror("Erreur lecture fichier console");
        return (void*)-1;
    }

    char test[BUFFER];
    ssize_t rc = recv(socket_client, test, BUFFER, 0);
    
    pthread_mutex_lock(&socket_mutex);
    if (rc <= 0) {
        if (rc == 0) {
            printf("Connexion fermée normalement\n");
        } else {
            perror("Erreur recv");
        }
        
        if (socket_client != -1) {
            close(socket_client);
            socket_client = -1;
        }
        pthread_mutex_unlock(&socket_mutex);
        return (void*)-1;
    }

    if (envoyerReponseTcp(socket_client, msg) == -1) {
        perror("Erreur envoi réponse");
        close(socket_client);
        socket_client = -1;
    }
    pthread_mutex_unlock(&socket_mutex);

    return NULL;
}

#endif // OSIRIS_COMMUNICATIONTEMPERATURE_H