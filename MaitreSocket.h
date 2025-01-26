#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef MAITRESOCKET_H
#define MAITRESOCKET_H

/**
 * Creer une socket multicast connecte au groupe multicast avec le protocole UDP
 * permettant ainsi ainsi au element de la piece(sauf Air)
 * d'envoyer ou de recevoir des messages via le groupe multicast
 *
 * @param multicast_group l'adresse du groupe multicast
 * @param port le port du groupe multicast
 * @param multicast_addr la socket d'adresse du groupe multicast
 * @return la socket udp multicast ou -1 en cas d'erreur
 */
int configurerSocketUDP(const char *multicast_group, unsigned int port, struct sockaddr_in multicast_addr);

/**
 * Creer une socket connecté avec le protocole TCP pour
 * permettre la communication des elements de la piece(Sauf Air)
 * avec le Systeme de Gestion de Temperature Centralise sur l'element
 * Communication temperature
 *
 * @param serveur_addr l'adresse du serveur
 * @param port le port d'écoute du serveur
 * @return la socket tcp ou -1 en cas d'erreur
 */
int configuerSocketTCP(const char *serveur_addr, unsigned int port);

int configurerSocketUDP(const char *multicast_group, unsigned int port, struct sockaddr_in multicast_addr)
{
    int sockfd;
    struct ip_mreq multicastRequest;

    // Création du socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Erreur lors de la création du socket");
        return -1;
    }

    // Configurer l'adresse de groupe multicast
    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicast_group);
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    int reuse = 1;
    // Joindre le groupe multicast
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (int *)&reuse, sizeof(reuse)) < 0)
    {
        perror("Erreur lors de la configuration du groupe multicast");
        close(sockfd);
        return -1;
    }

    // Associer la socket à l'adresse multicast
    if (bind(sockfd, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0)
    {
        perror("Erreur lors de la liaison de la socket à l'adresse multicast");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int configuerSocketTCP(const char *serveur_addr, unsigned int port)
{
    int socket_tcp;
    struct sockaddr_in addr_serveur;

    // création de la socket TCP
    if ((socket_tcp = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Erreur lors de la creation de la socket tcp");
        return -1;
    }

    // création de l'identifiant de la socket d'écoute du serveur
    bzero((char *) &addr_serveur, sizeof(addr_serveur));
    addr_serveur.sin_family = AF_INET;
    addr_serveur.sin_addr.s_addr = inet_addr(serveur_addr);
    addr_serveur.sin_port = htons(port);

    // connexion de la socket client locale à la socket coté serveur
    if (connect(socket_tcp, (struct sockaddr *)&addr_serveur, sizeof(struct sockaddr_in)) == -1)
    {
        close(socket_tcp);
        return -1;
    }

    return socket_tcp;
}

#endif //MAITRESOCKET_H