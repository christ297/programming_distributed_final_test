

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

#ifndef OSIRIS_GESTIONCONSOLE_H
#define OSIRIS_GESTIONCONSOLE_H

struct sock_com_udp{
    int socket;
    struct sockaddr_in addr;
    socklen_t addr_size;
};

/**
 * Creer une ecoute serveur avec le protocole udp
 * @param addr l'adresse
 * @return la socket cree ou -1 en cas d'erreur
 */
int demarrerServeurUdp(struct sockaddr_in addr);

/**
 * Routine executer dans le thread des comminications avec les consoles tcp
 * @param ptr l'adresse de la variable a traiter
 * @return un pointeur sur void
 */
void *communicationTcp(void *ptr);

/**
 * Routine executer dans le thread des comminications avec le serveur rmi
 * @param ptr l'adresse de la variable a traiter
 * @return un pointeur sur void
 */
void *udpAction(void *ptr);

int demarrerServeurUdp(struct sockaddr_in addr)
{
    int socket_serveur;
    int opt = 1;

    // Création de la socket en UDP
    if ((socket_serveur = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("Erreur lors de la création de la socket");
        return -1;
    }
if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("Erreur lors de l'activation de SO_REUSEPORT pour la socket UDP");
        close(socket_serveur);
        return -1;
    }
    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erreur lors de l'activation de SO_REUSEADDR pour la socket UDP");
        close(socket_serveur);
        return -1;
    }

    
//    // Paramètrage de la socket
//    if (setsockopt(socket_serveur, SOL_SOCKET, SO_KEEPALIVE, &opt,
//                   sizeof(opt)) != 0) {
//        perror("Erreur lors du parametrage de la socket");
//        return -1;
//    }

    // Attachement de la socket sur le port et l'adresse IP
    if (bind(socket_serveur, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        perror("Erreur lors de la liaison de la socket d'ecoute");
        return -1;
    }

    return socket_serveur;
}

int demarrerMulticastUdp(struct sockaddr_in addr, const char *multicast_group)
{
    int socket_serveur;
    struct ip_mreq mreq;
    int opt=1;

    // Création de la socket en UDP
    if ((socket_serveur = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("Erreur lors de la création de la socket");
        return -1;
    }
if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erreur lors de l'activation de SO_REUSEADDR pour la socket UDP");
        close(socket_serveur);
        return -1;
    }

    // Activer SO_REUSEPORT (facultatif, selon l'OS et le besoin)
    if (setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("Erreur lors de l'activation de SO_REUSEPORT pour la socket UDP");
        close(socket_serveur);
        return -1;
    }
    // Lier la socket à l'adresse et au port
    if (bind(socket_serveur, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Erreur lors de la liaison de la socket");
        close(socket_serveur);
        return -1;
    }

    // Configurer l'adresse de groupe multicast
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    // Joindre le groupe multicast
    if (setsockopt(socket_serveur, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Erreur lors de la configuration du groupe multicast");
        close(socket_serveur);
        return -1;
    }

    return socket_serveur;
}

void *communicationTcp(void *ptr)
{
    while(1)
    {
        u_char buffer[BUFFER];
        MessageTemperature *msg = lireDansFichier(buffer, PIECE_FILE_LOCATION);

        if(msg != NULL)
        {
            envoyerReponseTcp(*((int *)&ptr), msg);
        }

        sleep(3);
    }
}

void *udpAction(void *ptr)
{
    struct sock_com_udp udpsock = *(struct sock_com_udp *)ptr;
    u_char hello[BUFFER];

    ssize_t received_bytes = recvfrom(udpsock.socket, hello, BUFFER, 0, (struct sockaddr *)&udpsock.addr, &udpsock.addr_size);
    if (received_bytes <= 0)
    {
        if (received_bytes == 0)
        {
            perror(0);
            pthread_exit(NULL);
        }
        else
        {
            perror("-1");
            pthread_exit(NULL);
        }
    }
    else
    {
        printf("Systeme Central [SERVEUR RMI] est en ligne : %s", hello);
    }

    u_char buffer[BUFFER];

    while (1)
    {
        MessageTemperature *msg = lireDansFichier(buffer, PIECE_FILE_LOCATION);

        if ((envoyerReponseUdp(udpsock.socket, udpsock.addr, msg)) == -1) perror("Echec de l'envoi au systeme central [Serveur RMI]");

        sleep(1);
    }
}

#endif //OSIRIS_GESTIONCONSOLE_H
