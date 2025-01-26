#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Type.h"

#define BUFFER 128
#define MAX_PIECE_LENGTH 99 // 100 - 1 pour le terminateur NULL

#ifndef MESSAGETEMPERATURE_H
#define MESSAGETEMPERATURE_H

typedef struct MessageTemperature {
    char piece[100];
    int valeur;
    unsigned char type;
} MessageTemperature;

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

unsigned char* toBytes(struct MessageTemperature *msg) {
    // Taille calculée : 4(int) + 1(char) + strlen(piece) + 1(NULL)
    size_t piece_len = strnlen(msg->piece, MAX_PIECE_LENGTH);
    unsigned char *tab = (unsigned char*)malloc(5 + piece_len + 1);
    
    // Écriture little-endian de la valeur
    for (int i = 0; i < 4; i++) {
        tab[i] = (msg->valeur >> (i * 8)) & 0xFF;
    }
    
    tab[4] = msg->type;
    
    // Copie sécurisée avec termination NULL
    strncpy((char*)tab + 5, msg->piece, piece_len);
    tab[5 + piece_len] = '\0'; // Termination explicite
    
    return tab;
}

struct MessageTemperature* createMessageTemperature(int valeur, unsigned char type, char *piece) {
    struct MessageTemperature *msg = (struct MessageTemperature*)malloc(sizeof(MessageTemperature));
    msg->valeur = valeur;
    msg->type = type;
    
    // Copie sécurisée avec termination NULL
    strncpy(msg->piece, piece, MAX_PIECE_LENGTH);
    msg->piece[MAX_PIECE_LENGTH] = '\0';
    
    return msg;
}

struct MessageTemperature* fromBytes(unsigned char *tab, int length) {
    if (length < 5) return NULL; // Taille minimale
    
    // Décodage little-endian
    int valeur = 0;
    for (int i = 0; i < 4; i++) {
        valeur |= (tab[i] << (i * 8));
    }
    
    // Calcul longueur pièce sécurisé
    size_t piece_len = strnlen((char*)tab + 5, length - 5);
    
    // Allocation avec espace pour NULL
    char *piece = (char*)malloc(piece_len + 1);
    strncpy(piece, (char*)tab + 5, piece_len);
    piece[piece_len] = '\0';
    
    return createMessageTemperature(valeur, tab[4], piece);
}

#endif