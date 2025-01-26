#!/bin/bash

# Vérifier que le nombre d'arguments est correct
if [ "$#" -ne 7 ]; then
    echo "Usage: $0 <adresse_serveur> <multicast_addr> <port_multicast> <piece> <port_udp> <port_tcp> <max_client>"
    exit 1
fi

# Récupérer les arguments
ADRESSE_SERVEUR="$1"
MULTICAST_ADDR="$2"
PORT_MULTICAST="$3"
PIECE="$4"
PORT_UDP="$5"
PORT_TCP="$6"
MAX_CLIENT="$7"

# Chemin vers les exécutables des programmes
MULTICAST="./Multicast"
COMMANDE="./Commande"
CHAUFFAGE="./Chauffage"
CONSOLE="./Console"
CONSOLE_JAVA="java Console"
SYSTEM_CENTRAL="./GestionConsole"
COMMUNICATION_TEMPERATURE="./CommunicationTemperature"
RMI="java ServeurRMI"
THERMOMETRE="./Thermometre"
AIR="java Air"

# Fonction pour lancer un programme dans un terminal indépendant
lancer_terminal() {
    local titre="$1"
    local commande="$2"
    gnome-terminal --title="$titre" -- bash -c "$commande; exec bash" &
}

# Lancer tous les programmes en parallèle
echo "Lancement des programmes dans des terminaux indépendants..."
lancer_terminal "Air" "$AIR $MULTICAST_ADDR $PORT_MULTICAST $PIECE"
lancer_terminal "Multicast" "$MULTICAST $MULTICAST_ADDR $PORT_MULTICAST"
lancer_terminal "Système Central" "$SYSTEM_CENTRAL $MULTICAST_ADDR $PORT_TCP $PORT_UDP $MAX_CLIENT"
sleep 1
lancer_terminal "Communication Temperature" "$COMMUNICATION_TEMPERATURE $PORT_TCP $MAX_CLIENT"
sleep 1
lancer_terminal "Commande" "$COMMANDE $ADRESSE_SERVEUR $PORT_TCP"
sleep 1
lancer_terminal "Chauffage" "$CHAUFFAGE $MULTICAST_ADDR $PORT_UDP $ADRESSE_SERVEUR $PORT_TCP $PIECE"
sleep 1
lancer_terminal "Thermometre" "$THERMOMETRE $MULTICAST_ADDR $PORT_UDP $ADRESSE_SERVEUR $PORT_TCP $PIECE"
sleep 1
lancer_terminal "Console" "$CONSOLE $ADRESSE_SERVEUR $PORT_TCP"
sleep 1
lancer_terminal "RMI" "$RMI $MULTICAST_ADDR $PORT_MULTICAST"
sleep 1
lancer_terminal "Console Java" "$CONSOLE_JAVA"



echo "Tous les programmes ont été lancés simultanément !"