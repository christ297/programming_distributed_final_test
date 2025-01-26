#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    // Vérification des arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <adresse_multicast> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *multicast_group = argv[1]; // Adresse multicast passée en argument
    int port = atoi(argv[2]);              // Port passé en argument

    int sockfd;
    struct sockaddr_in addr;
    struct ip_mreq mreq;
    char buffer[1024];

    // Création d'une socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    // Activer SO_REUSEADDR pour permettre à plusieurs sockets de se lier à la même adresse et port
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("Erreur lors de l'activation de SO_REUSEADDR");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Activer SO_REUSEPORT pour permettre à plusieurs sockets de se lier exactement au même port
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        perror("Erreur lors de l'activation de SO_REUSEPORT");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Remplir la structure d'adresse
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // Écoute sur toutes les interfaces
    addr.sin_port = htons(port);              // Port spécifié en argument

    // Lier la socket à l'adresse et au port
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Erreur lors de la liaison de la socket");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Configurer l'adresse de groupe multicast
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_group); // Adresse multicast spécifiée
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);          // Interface par défaut

    // Joindre le groupe multicast
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("Erreur lors de la configuration du groupe multicast");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Afficher l'identification du groupe multicast
    printf("Le groupe multicast a été créé avec succès.\n");
    printf("Adresse IP du groupe multicast : %s\n", multicast_group);
    printf("Port du groupe multicast : %d\n", port);

    // Attendre continuellement des messages
    while (1) {
        ssize_t received_bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, 0);
        if (received_bytes < 0) {
            perror("Erreur lors de la réception des données");
            break;
        }

        // Traitement des données reçues, vous pouvez ajouter votre logique ici
        printf("Message reçu : %.*s\n", (int)received_bytes, buffer);
    }

    // Fermer la socket (jamais atteint dans cette boucle infinie)
    close(sockfd);
    return 0;
}