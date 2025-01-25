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

# Fonction pour lancer un programme dans un nouvel onglet
lancer_onglet() {
    local titre="$1"
    local commande="$2"
    gnome-terminal --tab --title="$titre" -- bash -c "$commande; exec bash"
}

# Lancer chaque programme dans un onglet séparé
echo "Lancement des programmes dans des onglets séparés..."

lancer_onglet "Communication Temperature" "$COMMUNICATION_TEMPERATURE $PORT_TCP $MAX_CLIENT"
lancer_onglet "Commande" "$COMMANDE $ADRESSE_SERVEUR $PORT_TCP"
lancer_onglet "Air" "$AIR $MULTICAST_ADDR $PORT_MULTICAST $PIECE"
lancer_onglet "Chauffage" "$CHAUFFAGE $MULTICAST_ADDR $PORT_UDP $ADRESSE_SERVEUR $PORT_TCP $PIECE"
lancer_onglet "Thermometre" "$THERMOMETRE $MULTICAST_ADDR $PORT_UDP $ADRESSE_SERVEUR $PORT_TCP $PIECE"
lancer_onglet "Console" "$CONSOLE $ADRESSE_SERVEUR $PORT_TCP"
lancer_onglet "Console Java" "$CONSOLE_JAVA"
lancer_onglet "RMI" "$RMI $MULTICAST_ADDR $PORT_MULTICAST"
lancer_onglet "Système Central" "$SYSTEM_CENTRAL $MULTICAST_ADDR $PORT_TCP $PORT_UDP $MAX_CLIENT"

echo "Tous les programmes ont été lancés dans des onglets séparés."