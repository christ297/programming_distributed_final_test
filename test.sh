#!/bin/bash

# Afficher une boîte de dialogue pour saisir les paramètres
PARAMETRES=$(zenity --forms --title="Lancer les programmes" \
                    --text="Entrez les paramètres" \
                    --add-entry="Adresse IP du serveur" \
                    --add-entry="Port pour Commande" \
                    --add-entry="Port pour Chauffage" \
                    --add-entry="Port pour Console")

# Vérifier si l'utilisateur a annulé
if [ $? -ne 0 ]; then
    echo "Annulé par l'utilisateur."
    exit 1
fi

# Extraire les valeurs saisies
ADRESSE_SERVEUR=$(echo "$PARAMETRES" | cut -d'|' -f1)
PORT_COMMANDE=$(echo "$PARAMETRES" | cut -d'|' -f2)
PORT_CHAUFFAGE=$(echo "$PARAMETRES" | cut -d'|' -f3)
PORT_CONSOLE=$(echo "$PARAMETRES" | cut -d'|' -f4)

# Chemin vers les exécutables des programmes
COMMANDE="./Commande"
CHAUFFAGE="./Chauffage"
CONSOLE="./Console"

# Lancer les programmes en arrière-plan
zenity --info --title="Lancement" --text="Lancement du programme Commande..."
$COMMANDE $ADRESSE_SERVEUR $PORT_COMMANDE &

zenity --info --title="Lancement" --text="Lancement du programme Chauffage..."
$CHAUFFAGE $ADRESSE_SERVEUR $PORT_CHAUFFAGE "salon1" &

zenity --info --title="Lancement" --text="Lancement du programme Console..."
$CONSOLE $ADRESSE_SERVEUR $PORT_CONSOLE &

# Attendre que tous les programmes soient lancés
zenity --info --title="En attente" --text="En attente que les programmes démarrent..."
sleep 5

# Vérifier que les programmes sont en cours d'exécution
zenity --info --title="Vérification" --text="Vérification des processus en cours d'exécution..."
ps aux | grep -E "Commande|Chauffage|Console"

zenity --info --title="Terminé" --text="Tous les programmes ont été lancés."