



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MULTICAST_GROUP "239.0.0.1"
#define PORT 6668

int main() {
    int sockfd;
    struct sockaddr_in addr;
    struct ip_mreq mreq;
    char buffer[1024];

    // Création d'une socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Remplir la structure d'adresse
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    // Lier la socket à l'adresse et au port
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Erreur lors de la liaison de la socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse de groupe multicast
    mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    // Joindre le groupe multicast
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Erreur lors de la configuration du groupe multicast");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Afficher l'identification du groupe multicast
    printf("Le groupe multicast a été créé avec succès.\n");
    printf("Adresse IP du groupe multicast : %s\n", MULTICAST_GROUP);

    // Attendre continuellement des messages
    while (1)
    {
        ssize_t received_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, 0);
        if (received_bytes < 0)
        {
            perror("Erreur lors de la réception des données");
            break;
        }

        // Traitement des données reçues, vous pouvez ajouter votre logique ici
        printf("Message reçu : %.*s\n", (int)received_bytes, buffer);
    }

    // Notez que le code pour fermer la socket ne sera jamais atteint dans ce scénario,
    // mais vous pouvez ajouter une logique appropriée si nécessaire.

    return 0;
}
