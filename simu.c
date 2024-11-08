#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>

#define TempsSimulation 1000000000000 //temps en microsecondes
#define CMD "serveur"
#define LIGNE_MAX 256
#define Nbr_repas_max 100

// Structure pour stocker les informations du client
typedef struct {
    int clientSock;
    int id_philosophe;
    int etat_fourchettes;
} ClientInfo;

typedef struct {
	int id_philosophe;
	bool estConnecte;
	int nbr_repas;
	double tempsAttenteTotal;
	double tempAttenteMoyen;
	double tempsAttenteMax;
	double tempsMangerTotal;
	int tour;
	double tempsAttente[Nbr_repas_max];
	double tempsTotal;
} Philosophe;

Philosophe philosophe[5];
pthread_mutex_t fourchettes[5];

void simulation(int sock);

int ecrireLigne(int sock, const char *ligne);
int lireLigne(int sock, char *ligne, int tailleMax);
void *traitementClient(void *arg);
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void simulation(int sock){
   

     // Boucle pour accepter et traiter les connexions des clients
     for (int id_philosophe=0;id_philosophe<6;id_philosophe++) {
           struct sockaddr_in adrClient;
            socklen_t adrClientLen; 
	    adrClientLen = sizeof(adrClient);
	    int clientSock = accept(sock, (struct sockaddr *)&adrClient, &adrClientLen);
	    if (clientSock < 0) {
		perror("accept");
		printf("clientSock<0\n");
		exit(EXIT_FAILURE);
	    }

	    char clientAddr[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, &(adrClient.sin_addr), clientAddr, INET_ADDRSTRLEN);
	    printf("%s: connection accepted from %s:%hu\n", CMD, clientAddr, ntohs(adrClient.sin_port));

	    // structure pour passer les informations du client au thread
	    ClientInfo *clientInfo = (ClientInfo *)malloc(sizeof(ClientInfo));
	    clientInfo->clientSock = clientSock;
	    clientInfo->id_philosophe = id_philosophe;

	    // un thread pour traiter le client
	    pthread_t thread;
	    if (pthread_create(&thread, NULL, traitementClient, (void *)clientInfo) != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	    }

	    // Détachement du thread pour libérer automatiquement les ressources une fois terminé
	    if (pthread_detach(thread) != 0) {
		perror("pthread_detach");
		exit(EXIT_FAILURE);
	    }
	    //nbr_philosophe++;
	    //id_philosophe++;
	}

        close(sock);

	//fclose(fic);

   /* for (int i = 0; i < 5; i++) 
        sem_destroy(&fourchettes[i]);*/

}


//******************************************************************************************************************************************



void *traitementClient(void *arg)
{
	FILE * ficTemp[5];
	double debutTotal;
	for(int i=0;i<5;i++){
		char nomFichier[20] = "Philosophex";
		nomFichier[10] = (char)(i + '0');
		strcat(nomFichier, "TempsReel.txt");
		ficTemp[i]=fopen(nomFichier,"w");
		if (ficTemp[i]==NULL){perror("Erreur ouverture fichier\n");}
	}
	float start[5];
	float end[5];
 // Récupérer les informations du client depuis l'argument
    ClientInfo *clientInfo = (ClientInfo *)arg;

    // Extraire les informations spécifiques au client
    int clientSock = clientInfo->clientSock;
    int id_philosophe = clientInfo->id_philosophe;
	philosophe[id_philosophe].estConnecte = true;
	/*
	printf("etats des philosophes : \n");
	for(int i=0;i<5;i++){
		printf("%d\n",philosophe[i].estConnecte);
	}
	*/
    
    bool deconnexion = false;

    clientInfo->etat_fourchettes =0;
   int voisine_gauche = (id_philosophe) % 5; 
   int voisine_droite = (id_philosophe + 4) % 5; // fourchette du philosophe voisin à droite
   //printf("Je suis le philosophe n°%d.\n",id_philosophe);
   //printf("gauche %d droite %d\n",voisine_gauche,voisine_droite);
    
    int lgEcr;
    //lgEcr = ecrireLigne(clientSock, "Vous êtes le philosophe n°.\n");
    if (lgEcr == -1) {
          perror("ecrire ligne");
          exit(EXIT_FAILURE);
                }

     FILE * fic;
     fic=fopen("DînerTest.txt","a+");

	bool premiere_connexion_tous = false;
	bool to_continue = true;
    // Boucle continue pour traiter les commandes du client
    while (!deconnexion) {
		//verifier que tous les philosophes soient connectes
		if(!premiere_connexion_tous)
		{
			for(int k = 0; k < 5; k++)
			{
				if(!philosophe[k].estConnecte)
				{
				printf("Le philosophe n°%d n'est pas connecté.\n", k);
				to_continue = true;
				premiere_connexion_tous = false;
				break;
				}
				to_continue = false;
				premiere_connexion_tous = true;
			}
	    	debutTotal = omp_get_wtime();
			sleep(1);
		}
		if(to_continue)
		{
			continue;
		}
		fflush(stdout);	
	    // Lire la demande du client
	    char ligne[LIGNE_MAX];
		
	    if (lireLigne(clientSock, ligne, LIGNE_MAX) == -1) {
		perror("lire ligne");
		exit(EXIT_FAILURE);
	    }
		

	    bool recommencer = true; 
	    int k =0;
	    if (strcmp(ligne, "DEMANDE_FOURCHETTES\n") == 0) {
		fprintf(ficTemp[id_philosophe], "%f :J'ai demandé mes fourchettes\n", omp_get_wtime() - debutTotal);
		start[id_philosophe] = omp_get_wtime();


		       if (clientInfo->etat_fourchettes != 0){
				printf("Les fourchettes sont déjà à disposition du philosophe n°%d.\n", id_philosophe);
				lgEcr = ecrireLigne(clientSock, "Les fourchettes sont déjà à votre disposition. Veuillez vous en servir.\n");
				if (lgEcr == -1) {
				   perror("ecrire ligne");
				   exit(EXIT_FAILURE);
				}
				recommencer = false;

		       }
		       else{
			
		       while (recommencer){
			// Prendre une fourchette (lock)
			pthread_mutex_lock(&fourchettes[voisine_gauche]); // Lock left fork
			clientInfo->etat_fourchettes =1;
			fprintf(fic, "%d %d\n", id_philosophe,clientInfo->etat_fourchettes);
		
				// Essayer de prendre l'autre fourchette (trylock)
				if (pthread_mutex_trylock(&fourchettes[voisine_droite]) == 0) {
				    clientInfo->etat_fourchettes =2;
				    fprintf(fic, "%d %d\n", id_philosophe,clientInfo->etat_fourchettes);

				    // Envoyer la réponse au client
				    lgEcr = ecrireLigne(clientSock, "Fourchettes accordées. Vous pouvez manger\n");
					fprintf(ficTemp[id_philosophe], "%f :J'ai eu mes fourchettes\n", omp_get_wtime() - debutTotal);
				    if (lgEcr == -1) {
					perror("ecrire ligne");
					exit(EXIT_FAILURE);
				    }
					end[id_philosophe] = omp_get_wtime();
				    float waitTime = end[id_philosophe] - start[id_philosophe]; // Temps d'attente en secondes
				    int tour = philosophe[id_philosophe].tour;
          			    philosophe[id_philosophe].tempsAttente[tour]=waitTime;
					if(philosophe[id_philosophe].tempsAttente[tour]>philosophe[id_philosophe].tempsAttenteMax){
						philosophe[id_philosophe].tempsAttenteMax=philosophe[id_philosophe].tempsAttente[tour];
					}
					philosophe[id_philosophe].tour++;
					//print tableau temps attente pour chaque tour
					start[id_philosophe] = omp_get_wtime();
				    recommencer = false;
			  
				} else {

				    // Reposer la première fourchette (unlock)
				    pthread_mutex_unlock(&fourchettes[voisine_gauche]); // Unlock left fork

				    clientInfo->etat_fourchettes = 0;
				    fprintf(fic, "%d %d\n", id_philosophe,clientInfo->etat_fourchettes);

				    // Envoyer la réponse au client
				    lgEcr = ecrireLigne(clientSock, "Fourchettes indisponibles, veuillez patienter.\n");
				    if (lgEcr == -1) {
					perror("ecrire ligne");
					exit(EXIT_FAILURE);

				    }
			  		usleep(1000000);// 1s

				}
                          }
			//clock_t end = clock();
			//double waitTime = (double)(end - start) / CLOCKS_PER_SEC;

			//printf("Temps d'attente : %.6f secondes\n", waitTime);
		       }
	    } else if (strcmp(ligne, "POSER_FOURCHETTES\n") == 0 && (clientInfo->etat_fourchettes >=0)) {
		fprintf(ficTemp[id_philosophe], "%f :J'ai posé mes fourchettes\n", omp_get_wtime() - debutTotal);
		// Poser les fourchettes (unlock)
		end[id_philosophe] = omp_get_wtime();
		philosophe[id_philosophe].tempsMangerTotal += end[id_philosophe] - start[id_philosophe];
		philosophe[id_philosophe].nbr_repas++;
		printf("Philosophe %d:\nTemps passé à manger : %f secondes\n",id_philosophe, philosophe[id_philosophe].tempsMangerTotal);
		pthread_mutex_unlock(&fourchettes[voisine_gauche]); // Unlock left fork
		pthread_mutex_unlock(&fourchettes[voisine_droite]);
                clientInfo->etat_fourchettes =0;
		fprintf(fic, "%d 0\n", id_philosophe);
		lgEcr = ecrireLigne(clientSock, "Fourchettes posées.Vous pouvez philosopher.\n");
		if (lgEcr == -1) {
		    perror("ecrire ligne");
		    exit(EXIT_FAILURE);
		}
	    } 
		else if (strcmp(ligne, "QUITTER\n") == 0) {
		
		//printf("%s: %d bytes sent\n", CMD, lgEcr);
		deconnexion = true ;
		FILE * fic2;
		char nomFichier[20] = "Philosophex";
		nomFichier[10] = (char)(id_philosophe + '0');
		strcat(nomFichier, ".txt");
		fic2=fopen(nomFichier,"w");
		if (fic2==NULL){perror("Erreur ouverture fichier\n");}
		fprintf(fic2, "Temps d'attente maximum : %f secondes\n", philosophe[id_philosophe].tempsAttenteMax);
		fprintf(fic2, "Temps passé à manger : %f secondes\n", philosophe[id_philosophe].tempsMangerTotal);
		fprintf(fic2, "Nombre de repas : %d\n", philosophe[id_philosophe].nbr_repas);
		//calcul du temps d'attente moyen
		for(int i=0;i<philosophe[id_philosophe].nbr_repas;i++){
			philosophe[id_philosophe].tempsAttenteTotal += philosophe[id_philosophe].tempsAttente[i];
		}
		philosophe[id_philosophe].tempAttenteMoyen = philosophe[id_philosophe].tempsAttenteTotal/philosophe[id_philosophe].nbr_repas;
		fprintf(fic2, "Temps d'attente moyen : %f secondes\n", philosophe[id_philosophe].tempAttenteMoyen);
		double finTotal = omp_get_wtime();
		philosophe[id_philosophe].tempsTotal = finTotal - debutTotal;
		fprintf(fic2, "Temps total : %f secondes\n", philosophe[id_philosophe].tempsTotal);
		fclose(fic2);
		fclose(ficTemp[id_philosophe]);
		philosophe[id_philosophe].estConnecte = false;
		int nbr_philosophes_deconnectes = 0;
		for(int i=0;i<5;i++){
			if(!philosophe[i].estConnecte){
				nbr_philosophes_deconnectes++;
			}
		}
		if(nbr_philosophes_deconnectes==5){
			printf("Tous les philosophes sont déconnectés.\n");
			//ici on creet le fichier avec les statistiques sur tout les clients
			FILE * fic3;
			fic3=fopen("Statistiques.txt","w");
			if (fic3==NULL){perror("Erreur ouverture fichier\n");}
		}
		


		
	    } else {
		printf("Commande invalide. Veuillez envoyer une commande valide.\n");
	    }
    }

    // Fermer la socket du client
    close(clientSock);

    // Libérer la mémoire allouée pour les informations du client
    free(clientInfo);
    fclose(fic);
    pthread_exit(NULL);
}

//***********************************************************************************************************************************************

// Fonction pour envoyer une ligne de texte sur la socket
int ecrireLigne(int sock, const char *ligne)
{
    return write(sock, ligne, strlen(ligne));
}
//***********************************************************************************************************************************************

// Fonction pour lire une ligne de texte depuis la socket
int lireLigne(int sock, char *ligne, int tailleMax)
{
    ssize_t bytesRead = read(sock, ligne, tailleMax);

    if (bytesRead <= 0)
    {
        return -1;
    }

    ligne[bytesRead] = '\0';
    return bytesRead;
}

//***********************************************************************************************************************************************




int main(int argc, char *argv[])
{
    int sock, ret;
    struct sockaddr_in adrServ;
    socklen_t adrClientLen;
    int fin = 0;
    char ligne[LIGNE_MAX];
    int lgEcr;

    /*sem_t fourchettes[5];

    //Initialisation de tous les sémaphores à 1
    for (int i = 0; i < 5; i++) {
         sem_init(&fourchettes[i], 0, 1); // 
    }*/

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("%s: creating a socket\n", CMD);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("%s: binding to port %s\n", CMD, argv[1]);
    memset(&adrServ, 0, sizeof(adrServ));
    adrServ.sin_family = AF_INET;
    adrServ.sin_addr.s_addr = htonl(INADDR_ANY);
    adrServ.sin_port = htons(atoi(argv[1]));
    ret = bind(sock, (struct sockaddr *)&adrServ, sizeof(adrServ));
    if (ret < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("%s: listening\n", CMD);
    ret = listen(sock, 1);
    if (ret < 0)
    {
        perror("listen");printf("ret<0\n");
        exit(EXIT_FAILURE);
    }
    
    int id_philosophe =0;

     printf("%s: accepting connections\n", CMD);
    
     /*FILE * fic;
     fic=fopen("Dîner test.txt","a+");
     if (fic==NULL){perror("Erreur ouverture fichier\n"); return 1;}*/

     int nbr_philosophe=5;
     simulation(sock);
    return 0;
}






