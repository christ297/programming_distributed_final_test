

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Type.h"

#define BUFFER 128

#ifndef MESSAGETEMPERATURE_H
#define MESSAGETEMPERATURE_H

/**
 * Définition du type @MessageTemperature
 *
 * Message contenant des informations ou des demandes sur l'air d'une
 * pièce. S'il est de type "mesure", il contient alors la valeur de la
 * température courante de l'air. S'il est de type "chauffer", il
 * contient une demande de chauffage à effectuer.
 * Voici une implementation de message en C
 *
 */
typedef struct MessageTemperature MessageTemperature;

struct MessageTemperature
{
    char piece[100];
    int valeur;
    unsigned char type;
};

/**
 * Retourne la valeur stockée dans le message.
*/
int getValeur(struct MessageTemperature *msg)
{
    return msg -> valeur;
}

/**
 * Retourne le type du message (<code>MESURE</code> ou
 * <code>CHAUFFER</code>)
*/
unsigned char getType(struct MessageTemperature *msg)
{
    return msg -> type;
}

/**
 * Retourne le nom de la pièce
*/
char* getPiece(struct MessageTemperature *msg)
{
    return msg -> piece;
}

/**
 * Convertit le message en son équivalent en tableau de byte.
*/
unsigned char* toBytes(struct MessageTemperature *msg)
{
    unsigned char *tab = (unsigned char*)malloc(strlen(msg -> piece) + 5);
    int val = msg -> valeur;
    for (int i = 0; i < 4; i++)
    {
        tab[i] = (unsigned char)(val & 0x000000FF);
        val = val >> 8;
    }
    tab[4] = msg -> type;
    unsigned char *tabPiece = (unsigned char*)msg -> piece;

    for (int i = 0; i < strlen(msg -> piece); i++)
    {
        tab[i + 5] = tabPiece[i];
    }
    return tab;
}


/**
 * Crèe un nouveau message.
 * @param valeur le niveau de température ou la puissance du chauffage
 * @param type le type du message (<code>MESURE</code> ou <code>CHAUFFER</code>)
 * @param piece le nom de le pièce considérée
*/
struct MessageTemperature* createMessageTemperature(int valeur, unsigned char type, char *piece)
{
    struct MessageTemperature *msg = malloc(sizeof(MessageTemperature));
    msg -> valeur = valeur;
    msg -> type = type;
    strncpy(msg -> piece, piece, strlen(piece));
    return msg;
}


/**
 * Retourne un message à partir de son équivalent en tableau de byte.
 * @param tab le tableau de byte contenant le message
 * @param length le nombre de cases à considérer dans le tableau
 * @return une instance de message initialisée avec le contenu du
 * tableau
*/
struct MessageTemperature* fromBytes(unsigned char *tab, int length)
{
    int val[4];
    for (int i = 0; i < 4; i++)
    {
        if (tab[i] < 0)
        {
            val[i] = (tab[i] + 256) << (i * 8);
        }
        else
        {
            val[i] = tab[i] << (i * 8);
        }
    }
    int valeur = val[0] | val[1] | val[2] | val[3];

    char *piece = (char*)malloc(length - 5);

    strncpy(piece, (char*)tab + 5, length - 5);

    return createMessageTemperature(valeur, tab[4], piece);
}

#endif //MESSAGETEMPERATURE_H
