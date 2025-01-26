#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <errno.h>
#include "MessageTemperature.h"

#ifndef OSIRIS_SYSTEMECENTRAL_H
#define OSIRIS_SYSTEMECENTRAL_H

#define PIECE_FILE_LOCATION "message_piece.txt"
#define CONSOLE_FILE_LOCATION "message_console.txt"
#define BUFFER 128

// Mutex global pour les opérations sur les fichiers
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr_in configAdrr(u_int port);
int demarrerServeurTcp(u_int max_client, struct sockaddr_in addr_serveur);
int accepterConnexionClient(int socket_serveur);
void annoncerClient(int socket_client);
MessageTemperature *recevoirDemandeTcp(int socket);
int envoyerReponseTcp(int socket, MessageTemperature *msg);
MessageTemperature *recevoirDemandeUdp(int socket);
int envoyerReponseUdp(int socket, struct sockaddr_in addr, MessageTemperature *msg);
int ecrireDansFichier(MessageTemperature *msg, const char* file_location);
MessageTemperature *lireDansFichier(u_char *buffer, const char *filelocation);

// ------------------------------------------------------------
// Implementation
// ------------------------------------------------------------

struct sockaddr_in configAdrr(u_int port) {
    struct sockaddr_in ready_addr;
    memset(&ready_addr, 0, sizeof(ready_addr));
    ready_addr.sin_family = AF_INET;
    ready_addr.sin_port = htons(port);
    ready_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return ready_addr;
}

int demarrerServeurTcp(u_int max_client, struct sockaddr_in addr_serveur) {
    int socket_serveur;
    int option = 1;

    if ((socket_serveur = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erreur création socket");
        return -1;
    }

    // Configuration séparée des options
    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0) {
        perror("Erreur SO_REUSEADDR");
        close(socket_serveur);
        return -1;
    }

    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option)) < 0) {
        perror("Erreur SO_REUSEPORT");
        close(socket_serveur);
        return -1;
    }

    if (bind(socket_serveur, (struct sockaddr*)&addr_serveur, sizeof(addr_serveur)) < 0) {
        perror("Erreur bind");
        close(socket_serveur);
        return -1;
    }

    if (listen(socket_serveur, max_client) < 0) {
        perror("Erreur listen");
        close(socket_serveur);
        return -1;
    }

    return socket_serveur;
}

int accepterConnexionClient(int socket_serveur) {
    if (socket_serveur < 0) return -1;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int socket_client = accept(socket_serveur, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (socket_client < 0) {
        perror("Erreur accept");
        return -1;
    }

    return socket_client;
}

void annoncerClient(int socket_client) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    char ipAddress[INET_ADDRSTRLEN];

    if (getpeername(socket_client, (struct sockaddr*)&clientAddr, &clientAddrLen) == 0) {
        inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, INET_ADDRSTRLEN);
        printf("Nouvel appareil: %s:%d\n", ipAddress, ntohs(clientAddr.sin_port));
    }
}

MessageTemperature *recevoirDemandeTcp(int socket) {
    if (socket < 0) return NULL;

    u_char buffer[BUFFER];
    ssize_t res_code = recv(socket, buffer, sizeof(buffer), 0); // Retirer MSG_DONTWAIT

    if (res_code <= 0) {
        if (res_code == 0) {
            fprintf(stderr, "Connexion fermée\n");
        } else {
            perror("Erreur recv");
        }
        return NULL;
    }

    return fromBytes(buffer, res_code);
}

int envoyerReponseTcp(int socket, MessageTemperature *msg) {
    if (socket < 0 || !msg) return -1;

    u_char *data = toBytes(msg);
    ssize_t sent = send(socket, data, BUFFER, 0); // Retirer MSG_DONTWAIT

    if (sent <= 0) {
        if (errno == EBADF) perror("Socket invalide");
        else perror("Erreur send");
        return -1;
    }
    return 0;
}

MessageTemperature *recevoirDemandeUdp(int socket) {
    if (socket < 0) return NULL;

    u_char buffer[BUFFER];
    memset(buffer, 0, BUFFER); // Initialisation du buffer
    ssize_t res = recvfrom(socket, buffer, BUFFER, 0, NULL, NULL);

    if (res <= 0) {
        perror("Erreur recvfrom");
        return NULL;
    }
    return fromBytes(buffer, res);
}

int envoyerReponseUdp(int socket, struct sockaddr_in addr, MessageTemperature *msg) {
    if (socket < 0 || !msg) return -1;

    u_char *data = toBytes(msg);
    socklen_t addr_len = sizeof(addr);
    ssize_t sent = sendto(socket, data, BUFFER, 0, (struct sockaddr*)&addr, addr_len);

    if (sent <= 0) {
        perror("Erreur sendto");
        return -1;
    }
    return 0;
}

int ecrireDansFichier(MessageTemperature *msg, const char* file_location) {
    if (!msg) return -1;

    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen(file_location, "wb");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        perror("Erreur fopen");
        return -1;
    }

    u_char *data = toBytes(msg);
    size_t written = fwrite(data, 1, BUFFER, file);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    return (written == BUFFER) ? 0 : -1;
}

MessageTemperature *lireDansFichier(u_char *buffer, const char *filelocation) {
    pthread_mutex_lock(&file_mutex);
    FILE *file = fopen(filelocation, "rb");
    if (!file) {
        pthread_mutex_unlock(&file_mutex);
        perror("Erreur fopen");
        return NULL;
    }

    size_t read = fread(buffer, 1, BUFFER, file);
    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    return (read == BUFFER) ? fromBytes(buffer, BUFFER) : NULL;
}

void add_to_poll_fds(struct pollfd *poll_fds[], int new_fd, int *poll_count, int *poll_size) {
    // S'il n'y a pas assez de place, il faut réallouer le tableau de poll_fds
    if (*poll_count == *poll_size) {
        *poll_size *= 2; // Double la taille
// Dans SystemeCentral.h, ligne 214
*poll_fds = (struct pollfd*)realloc(*poll_fds, sizeof(**poll_fds) * (*poll_size));    }
    (*poll_fds)[*poll_count].fd = new_fd;
    (*poll_fds)[*poll_count].events = POLLIN;
    (*poll_count)++;
}

void del_from_poll_fds(struct pollfd **poll_fds, int i, int *poll_count) {
    // Copie le fd de la fin du tableau à cet index
    (*poll_fds)[i] = (*poll_fds)[*poll_count - 1];
    (*poll_count)--;
}

#endif //OSIRIS_SYSTEMECENTRAL_H
