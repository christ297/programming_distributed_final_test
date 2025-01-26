#ifndef CHAUFFAGE_H
#define CHAUFFAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "MessageTemperature.h"
#include "MaitreSocket.h"

/**
 * Réalise la logique d'envoi/réception du chauffage de l'air d'une pièce.
 * @param socket_tcp la socket du serveur de communication de température du système (Serveur CommunicationTemperature)
 * @param socket_udp la socket du groupe multicast de l'air de la pièce
 * @param addr l'adresse du groupe multicast
 * @param msg le message de température du type chauffage initial
 * @return 0 en cas de succès, -1 en cas d'erreur, -2 si la connexion TCP est perdue
 */
int ecrireSocket(int socket_tcp, int socket_udp, struct sockaddr *addr, MessageTemperature *msg);

/**
 * Crée un message de température du type chauffage pour une pièce et d'une puissance précise.
 * @param puissance la valeur du chauffage entre [0 ; 5]
 * @param piece le nom de la pièce dont on veut modifier le chauffage
 * @return un message de température du type chauffage, ou NULL en cas d'erreur
 */
MessageTemperature *ecrireDemandeChauffage(int puissance, const char *piece);

/**
 * Informe le système central de la réception avec succès d'un message de température du type chauffage.
 * @param socket la socket du système central (Serveur CommunicationTemperature) depuis laquelle le message est lu
 * @param demande_chauffage le message de température du type chauffage qui a été reçu
 * @return le nombre de bytes envoyés au serveur, ou -1 en cas d'erreur
 */
ssize_t envoyerAccuser(int socket, MessageTemperature *demande_chauffage);

/**
 * Reçoit un message de température du type chauffage depuis un serveur de préférence TCP.
 * @param socket la socket du système central (Serveur CommunicationTemperature) qui envoie le message
 * @return un message de température du type chauffage, ou NULL en cas d'erreur
 */
MessageTemperature *recevoirDemandeChauffage(int socket);

int ecrireSocket(int socket_tcp, int socket_udp, struct sockaddr *addr, MessageTemperature *msg) {
    // Dans ecrireSocket()
int reuse = 1;
setsockopt(socket_udp, SOL_SOCKET, SO_BROADCAST, &reuse, sizeof(reuse));
    if (socket_tcp == -1) return -1;

    MessageTemperature *demande_chauffage = recevoirDemandeChauffage(socket_tcp);

    if (demande_chauffage == ((void *) -1)) {
        return -2; // Connexion TCP perdue
    }
    if (demande_chauffage == NULL || strcmp(demande_chauffage->piece, msg->piece) != 0) {
        free(demande_chauffage);
        return -1; // Demande invalide
    }

    MessageTemperature *message_chauffage = ecrireDemandeChauffage(demande_chauffage->valeur, demande_chauffage->piece);
    if (message_chauffage == NULL) {
        free(demande_chauffage);
        return -1; // Erreur de création du message
    }

    // Envoi du message au groupe multicast
    if (sendto(socket_udp, toBytes(message_chauffage), BUFFER, 0, addr, sizeof(struct sockaddr)) < 0) {
        perror("Erreur lors de l'envoi de la demande de chauffage au groupe multicast de la pièce");
        free(demande_chauffage);
        free(message_chauffage);
        return -1;
    }

    msg->valeur = message_chauffage->valeur;
    printf("Demande de chauffage envoyée : Niveau de puissance = %d, Pièce = %s.\n",
           message_chauffage->valeur, message_chauffage->piece);

    if (envoyerAccuser(socket_tcp, demande_chauffage) == -1) {
        perror("Erreur lors de l'envoi de l'accusé de demande de chauffage au système central");
        free(demande_chauffage);
        free(message_chauffage);
        return -1;
    }

    free(demande_chauffage);
    free(message_chauffage);
    return 0;
}

MessageTemperature *ecrireDemandeChauffage(int puissance, const char *piece) {
    if (puissance < 0 || puissance > 5 || piece == NULL || strlen(piece) == 0) {
        return NULL;
    }

    MessageTemperature *demande_chauffage = (MessageTemperature *) malloc(sizeof(MessageTemperature));
    if (demande_chauffage == NULL) {
        perror("Erreur d'allocation mémoire");
        return NULL;
    }

    demande_chauffage->valeur = puissance;
    demande_chauffage->type = CHAUFFER;
    strncpy(demande_chauffage->piece, piece, sizeof(demande_chauffage->piece) - 1);
    demande_chauffage->piece[sizeof(demande_chauffage->piece) - 1] = '\0'; // Garantir la terminaison de la chaîne

    return demande_chauffage;
}

MessageTemperature *recevoirDemandeChauffage(int socket) {
    if (socket == -1) return NULL;

    u_char buffer[BUFFER];
    ssize_t octet = recv(socket, buffer, sizeof(buffer), MSG_DONTWAIT);

    if (octet == -1) {
        perror("Erreur lors de la réception de la demande de chauffage depuis le système central");
        return NULL;
    } else if (octet == 0) {
        return NULL; // Connexion fermée
    }

    MessageTemperature *demande_chauffage = fromBytes(buffer, BUFFER);
    if (demande_chauffage == NULL || demande_chauffage->type != CHAUFFER) {
        free(demande_chauffage);
        return NULL;
    }

    return demande_chauffage;
}

ssize_t envoyerAccuser(int socket, MessageTemperature *demande_chauffage) {
    if (socket == -1 || demande_chauffage == NULL) return -1;

    printf("La demande de chauffage pour la Pièce = %s, Puissance = %d a bien été reçue.\n",
           demande_chauffage->piece, demande_chauffage->valeur);

    return send(socket, toBytes(demande_chauffage), BUFFER, MSG_DONTWAIT);
}

#endif // CHAUFFAGE_H