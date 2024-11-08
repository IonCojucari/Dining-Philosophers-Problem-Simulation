#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <omp.h>

#define MAX_WAIT_TIME 5
#define CMD   "client"
#define LIGNE_MAX 256
#define dureeSimulation 40

// Helper function to generate a random wait time between 1 and 3 seconds
int random_wait_time() {
    return (rand() % 3) + 1;
}


// Fonction pour envoyer une ligne de texte sur la socket
int ecrireLigne(int sock, const char* ligne) {
    return write(sock, ligne, strlen(ligne));
}

// Fonction pour lire une ligne de texte depuis la socket
int lireLigne(int sock, char* ligne, int tailleMax) {
    ssize_t bytesRead = read(sock, ligne, tailleMax);
    if (bytesRead <= 0) {
        return -1;
    }
    ligne[bytesRead] = '\0';
    return bytesRead;
}




int main(int argc, char *argv[]) {
    int sock;
    int ret = 8; // utilisée pour stocker le descripteur de socket et valeur de retour 
    struct sockaddr_in adrServ; // adresse du serveur auquel le client se connecte
    int fin = 0;
    char ligne[LIGNE_MAX];
    int lgEcr; // pour stocker le nombre d'octets écrits ou envoyés sur le socket 

    if (argc != 3) { // boucle pour vérifier que le client fournit l'adresse du serveur et le port
        fprintf(stderr, "usage: %s machine port\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }




    memset(&adrServ, 0, sizeof(adrServ));
    adrServ.sin_family = AF_INET;
    adrServ.sin_port = htons(atoi(argv[2]));


ret = inet_pton(AF_INET, "127.0.0.1", &(adrServ.sin_addr));
if (ret == 1) {
    printf("inet_pton succeeded\n");
} else if (ret == 0) {
    printf("inet_pton failed: Invalid address\n");
} else {
    printf("inet_pton failed: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}

printf("ret vaut %d\n",ret);


    printf("%s: adr %s, port %hu\n", CMD,inet_ntoa(adrServ.sin_addr),ntohs(adrServ.sin_port));

    printf("%s: connecting the socket\n", CMD);
printf("\n\n");

/*struct sockaddr_in adrClient;
memset(&adrClient, 0, sizeof(adrClient));
adrClient.sin_family = AF_INET;
adrClient.sin_addr.s_addr = inet_addr("127.0.0.1"); // Remplacez "127.0.0.1" par l'adresse IP appropriée pour chaque client
adrClient.sin_port = htons(atoi(argv[2])); // Remplacez argv[2] par le port approprié pour chaque client
printf("%s: adr %s, port %hu\n", CMD, inet_ntoa(adrClient.sin_addr), ntohs(adrClient.sin_port));*/

printf("\n\n");





    ret = connect(sock, (struct sockaddr *)&adrServ, sizeof(adrServ));
    if (ret < 0) {
        perror("connect");printf("ret vaut %d\n",ret);
        exit(EXIT_FAILURE);
    }
    float debut = omp_get_wtime();  // début du chronomètre
    int choix = 1;


    while (!fin) {
        float finboucle = omp_get_wtime();  // fin du chronomètre
        
        
        /*
        printf("\nMenu:\n");
        printf("1. Demander les fourchettes\n");
        printf("2. Poser les fourchettes\n");
        printf("3. Quitter\n");
        printf("Choix : ");

        if (fgets(ligne, LIGNE_MAX, stdin) == NULL) {
            fprintf(stderr, "saisie fin de fichier\n");
            exit(EXIT_FAILURE);
        }

        int choix = atoi(ligne);
        */
        
        switch (choix) {
            case 1:
                lgEcr = ecrireLigne(sock, "DEMANDE_FOURCHETTES\n");
                if (lgEcr == -1) {
                    perror("ecrire ligne");
                    exit(EXIT_FAILURE);
                }
		// Lire la réponse du serveur
		        if (lireLigne(sock, ligne, LIGNE_MAX) == -1) {
   			        perror("lire ligne");
    			    exit(EXIT_FAILURE);
		        }
                int tempsAttente = 0;
                while(strncmp(ligne,"Fourchettes accordées.",18) != 0)
                {
                    sleep(1);
                    tempsAttente++;
                    if (tempsAttente == MAX_WAIT_TIME)  // Use a constant for max wait time
                    {
                        break;
                    }
                }
                
                choix = 2;
                break;
            case 2:
                lgEcr = ecrireLigne(sock, "POSER_FOURCHETTES\n");
                if (lgEcr == -1) {
                    perror("ecrire ligne");
                    exit(EXIT_FAILURE);
                }
                if (lireLigne(sock, ligne, LIGNE_MAX) == -1) {
   			    perror("lire ligne");
    			exit(EXIT_FAILURE);
                }
                choix = 1;
                if(finboucle-debut > dureeSimulation)
                    choix = 3;
                break;
           
            case 3:
                lgEcr = ecrireLigne(sock, "QUITTER\n");
                if (lgEcr == -1) {
                    perror("ecrire ligne");
                    exit(EXIT_FAILURE);
                }
                fin = 1;
                break;
            default:
                printf("Choix invalide. Veuillez choisir une option valide.\n");
                break;
        }
        int random = random_wait_time();
        sleep(random);

    }

    if (close(sock) == -1) {
        perror("close socket");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
