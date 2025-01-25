

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "MessageTemperature.h"
#include "MaitreSocket.h"


#ifndef THERMOMETRE_H
#define THERMOMETRE_H

/**
 * Lit les messages envoyer les sockets udp
 * et d'ecrire les données lues dans un tableau de byte
 *
 * @param socket_udp la socket de la piece
 * @param buffer le tableau de byte dans le quel sera ecrit les informations lus
 * @return le nombre de byte lues ou 0 si la socket est deconnecter et -1 en cas d'erreur
 */
ssize_t lireSocket(int socket_udp, u_char *buffer);

/**
 * Lit dans un tableau et retourne un message de temperature
 * du type mesure
 * 
 * @param buffer le tableau contenant les informations a lire
 * @return un pointeur vers un message de temperature du type mesure
 * ou NULL si le message de temperature est de type different
 */
struct MessageTemperature *lireMessageTemperature(unsigned char *buffer);

/**
 * Envoye au systeme central un message de temperature du type mesure
 * avec la temperature de la piece et le nom de la piece
 *
 * @param socket la socket du systeme central (Serveur CommunicationTemperature)
 * @param msg le message de temperature du type mesure reçue du groupe multicast
 * @return 0 si la ReponseTemperature est envoye avec succes ou -1 si erreur ou ECONNRESET si la connection est perdu
 */
int envoyerTemperature(int socket, MessageTemperature *msg);


ssize_t lireSocket(int socket_udp, u_char *buffer)
{

    ssize_t received_bytes = recvfrom(socket_udp, buffer, BUFFER, 0, NULL, 0);
    if (received_bytes <= 0)
    {
        if (received_bytes == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    return received_bytes;
}

int envoyerTcp(int socket_tcp, u_char *buffer)
{
    if(socket_tcp == -1) return -1;

    MessageTemperature *mesure_temperature = lireMessageTemperature(buffer);

    if(mesure_temperature == NULL) return -1;

    return envoyerTemperature(socket_tcp, mesure_temperature);
}

MessageTemperature *lireMessageTemperature(unsigned char *buffer)
{
    // Conversion du tableau de bytes en struct MessageTemperature
    MessageTemperature *mesure_temperature = fromBytes(buffer, BUFFER);

    // Vérification du type du message
    if (mesure_temperature -> type != MESURE) return NULL;

    printf("Température reçue pour la pièce %s : %.3f°C \n", mesure_temperature -> piece,
           (float)mesure_temperature -> valeur);

    return mesure_temperature;
}

int envoyerTemperature(int socket, MessageTemperature *msg)
{
    if (socket == -1 || msg == NULL) return -1;

    ssize_t octet = send(socket, toBytes(msg), BUFFER, MSG_DONTWAIT);

    if(octet == -1)
    {
        if(errno == ECONNRESET)
        {
            perror("Connexion perdue avec le systeme central ");
            perror("Systeme Central deconnecter [Serveur CommunicationTemperature hors ligne] ");
            return ECONNRESET;
        }

        perror("Erreur lors de l'envoie de la temperature au systeme central [Serveur CommunicationTemperature] ");
        return -1;
    }

    return EXIT_SUCCESS;

}
#endif //THERMOMETRE_H
