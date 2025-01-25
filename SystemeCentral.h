


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
#include "MessageTemperature.h"


#ifndef OSIRIS_SYSTEMECENTRAL_H
#define OSIRIS_SYSTEMECENTRAL_H

#define PIECE_FILE_LOCATION "message_piece.txt"
#define CONSOLE_FILE_LOCATION "message_console.txt"

/**
 * Creer un adresse sur laquelle peut etre associe une socket
 * @param port le port tcp sur lequel l'adresse du serveur sera configurer
 * @return sockaddr_in initialise et pret a l'emploie
 */
struct sockaddr_in configAdrr(u_int port);

/**
 * Configure une socket qui va servir de socket serveur pour un nombre specifique de client
 * et sur un adresse donne
 * @param max_client le nombre maximum de client qui peuvent se connecter au serveur
 * @param addr_serveur l'adresse du serveur
 * @return la socket serveur sous forme d'entier ou -1 en cas d'erreur
 */
int demarrerServeurTcp(u_int max_client, struct sockaddr_in addr_serveur);

/**
 * Permet a un client réseau de se connecter à une socket serveur
 * @param socket_serveur la socket serveur sur à laquelle le client va se connecter et communiquer avec le serveur
 * @return un pointeur d'entier vers l'adresse de la socket client
 */
int accepterConnexionClient(int socket_serveur);

/**
 * Informe le serveur lors de la connexion d'un client
 * @param socket_client l'adresse de la socket cliente
 */
void annoncerClient(int *socket_client);


/**
 * Permet de lire dans une socket tcp client/serveur les messages de temperature
 * @param socket il s'agit de la socket d'ou sera lu le message de temperature
 * @return un pointeur vers l'adresse d'une reference de MessageTemperature
 */
MessageTemperature *recevoirDemandeTcp(int socket);

/**
 * Envoyer un message de temperature sur une socket tcp serveur/client
 * @param socket la socket tcp ou sera ecrit le message de temperature
 * @param msg le message de temperature avec les informations adequate
 * @return 0 si le message a ete correctement envoyer et -1 en cas d'erreur
 */
int envoyerReponseTcp(int socket, MessageTemperature *msg);

/**
 * Permet de lire dans une socket udp client/serveur les messages de temperature
 * @param socket il s'agit de la socket d'ou sera lu le message de temperature
 * @return un pointeur vers l'adresse d'une reference de MessageTemperature
 */
MessageTemperature *recevoirDemandeUdp(int socket);

/**
 * Envoyer un message de temperature sur une socket udp serveur/client
 * @param socket la socket tcp ou sera ecrit le message de temperature
 * @param msg le message de temperature avec les informations adequate
 * @return 0 si le message a ete correctement envoyer et -1 en cas d'erreur
 */
int envoyerReponseUdp(int socket, struct sockaddr_in addr, MessageTemperature *msg);

int ecrireDansFichier(MessageTemperature *msg, char* file_location);

MessageTemperature *lireDansFichier(u_char *buffer, char *filelocation);


struct sockaddr_in configAdrr(u_int port)
{
    struct sockaddr_in ready_addr;

    // liaison de la socket d'écoute sur le port
    bzero((char *) &ready_addr, sizeof(ready_addr));
    memset(&ready_addr, 0, sizeof(ready_addr));
    ready_addr.sin_family = AF_INET;
    ready_addr.sin_port = htons(port);
    ready_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    return ready_addr;
}

int demarrerServeurTcp(u_int max_client, struct sockaddr_in addr_serveur)
{
    int socket_serveur;
    int option = 1;

    // création socket TCP d'écoute
    if ((socket_serveur = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erreur lors de la création de la socket");
        return -1;
    }

    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option,
                   sizeof(option)) != 0) {
        perror("Erreur lors du parametrage de la socket");
        return -1;
    }

    if(bind(socket_serveur, (struct sockaddr*)&addr_serveur, sizeof(addr_serveur)) != 0 )
    {
        perror("Erreur lors de la liaison de la socket d'ecoute");
        return -1;
    }
  if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) != 0) {
        perror("Erreur lors de l'activation de SO_REUSEADDR pour la socket TCP");
        close(socket_serveur);
        return -1;
    }

    // Activer SO_REUSEPORT
    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option)) != 0) {
        perror("Erreur lors de l'activation de SO_REUSEPORT pour la socket TCP");
        close(socket_serveur);
        return -1;
    }
    // configuration socket écoute : 8 connexions max en attente
    if (listen(socket_serveur, max_client) < 0)
    {
        perror("Erreur lors de l'ecoute des clients");
        return -1;
    }

    return socket_serveur;
}

int accepterConnexionClient(int socket_serveur)
{
    if(socket_serveur == -1) return -1;

    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(struct sockaddr_in);

    int socket_client = accept(socket_serveur, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (socket_client == -1)
    {
        perror("Erreur lors de l'acceptation de la connexion");
        return -1;
    }

    setsockopt(socket_client, SOL_SOCKET, SO_KEEPALIVE, (int *)(1),  1);

    return socket_client;
}

void annoncerClient(int *socket_client)
{
    char ipAddress[INET_ADDRSTRLEN];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    getpeername( *socket_client, (struct sockaddr*)&clientAddr, &clientAddrLen);
    inet_ntop(AF_INET, &(clientAddr.sin_addr), ipAddress, INET_ADDRSTRLEN);

    printf("Nouvel Appareil connecté depuis %s:%d \n", ipAddress, ntohs(clientAddr.sin_port));
}

MessageTemperature *recevoirDemandeTcp(int socket)
{
    if(socket == -1) return NULL;

    u_char buffer[BUFFER];
    ssize_t res_code = recv(socket, buffer, sizeof buffer, MSG_DONTWAIT);

    if(res_code <= 0)
    {
        if(res_code == 0)
        {
            perror("Connection fermer/interrompue par un appareil...");
            return ((void*) -1);
        }
        else
        {
            perror("Erreur lors de la lecture du message de temperature");
            return NULL;
        }
    }

    return fromBytes(buffer, sizeof(buffer));
}

int envoyerReponseTcp(int socket, MessageTemperature *msg)
{
    if (socket != -1 && msg != NULL)
    {
        u_char *reponseTemperature = toBytes(msg);

        if(send(socket, reponseTemperature, BUFFER, MSG_DONTWAIT) == -1)
        {
            perror("Erreur lors de l'envoie du message de temperature");
            return -1;
        }

        return 0;
    }

    return -1;
}

MessageTemperature *recevoirDemandeUdp(int socket)
{
    if(socket < 0) return NULL;

    u_char buffer[BUFFER];
    if (recvfrom(socket, buffer, sizeof buffer, 0, NULL, 0) < 0)
    {
        perror("Erreur lors de la réception des données");
        return NULL;
    }

    return fromBytes(buffer, BUFFER);
}

int envoyerReponseUdp(int socket, struct sockaddr_in addr, MessageTemperature *msg)
{
    if(socket <= 0) return -1;

    socklen_t socklen = sizeof(addr);

    if (sendto(socket, toBytes(msg), BUFFER, MSG_DONTWAIT,
               (struct sockaddr *) &addr, socklen) == -1) return -1;

    return 0;
}

int ecrireDansFichier(MessageTemperature *msg, char* file_location)
{
    pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&file_mutex);

    // Écrire dans le fichier
    FILE *file = fopen(file_location, "w"); //(message_piece.txt", "wb+");
    if (file == NULL)
    {
        perror("Erreur lors de la lecteur du fichier partage");
        return -1;
    }

    // Écrire la structure dans le fichier
    if (fwrite(toBytes(msg), BUFFER, 1, file) != 1)
    {
        perror("Erreur lors de l'ecriture dans le fichier");
        return -1;
    }

    // Fermer le fichier et déverrouiller le mutex
    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    return 0;
}

MessageTemperature *lireDansFichier(u_char *buffer, char *filelocation)
{
    pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&file_mutex);

    // Écrire dans le fichier
    FILE *file = fopen(filelocation, "r"); //(message_console.txt", "r");
    if (file == NULL)
    {
        perror("Erreur lors de la lecteur du fichier partage");
        return NULL;
    }

    // lire la structure dans le fichier
    fread(buffer, BUFFER, 1, file);

    // Fermer le fichier et déverrouiller le mutex
    fclose(file);
    pthread_mutex_unlock(&file_mutex);

    return fromBytes(buffer, BUFFER);
}


void add_to_poll_fds(struct pollfd *poll_fds[], int new_fd, int *poll_count, int *poll_size) {
    // S'il n'y a pas assez de place, il faut réallouer le tableau de poll_fds
    if (*poll_count == *poll_size) {
        *poll_size *= 2; // Double la taille
        *poll_fds = realloc(*poll_fds, sizeof(**poll_fds) * (*poll_size));
    }
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
