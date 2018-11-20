/* echo / serveur simpliste
   Master Informatique 2012 -- Université Aix-Marseille  
   Emmanuel Godard
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>

/* taille maximale des lignes */
#define MAXLIGNE 60
#define MAX_BUFF 1500
#define CIAO "Au revoir ...\n"

int tunfd;
int connected = 0;
int s_in; /* descripteur de socket */

int tun_alloc(char *dev){
  struct ifreq ifr;
  int fd, err;

  if( (fd = open("/dev/net/tun", O_RDWR)) < 0 ){
    perror("alloc tun");
    exit(-1);
  }

  memset(&ifr, 0, sizeof(ifr));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
   *        IFF_TAP   - TAP device  
   *
   *        IFF_NO_PI - Do not provide packet information  
   */ 
  ifr.ifr_flags = IFF_TUN; 
  if( *dev )
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

  if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
    close(fd);
	fprintf(stderr, "\n\nerreur tunalloc\n\n\n");
    return err;
  }
  strcpy(dev, ifr.ifr_name);
  return fd;
}


void echo(int f, char* hote, char* port);

int main(int argc, char *argv[])
{
	
  int s,n; /* descripteurs de socket */
  int len,on; /* utilitaires divers */
  struct addrinfo * resol; /* résolution */
  struct addrinfo indic = {AI_PASSIVE, /* Toute interface */
                           PF_INET,SOCK_STREAM,0, /* IP mode connecté */
                           0,NULL,NULL,NULL};
  struct sockaddr_in client; /* adresse de socket du client */
  char * port; /* Port pour le service */
  int err; /* code d'erreur */
  
  char * hote; 
  char ip[INET6_ADDRSTRLEN]; /* adresse IPv4 en notation pointée */
  struct addrinfo *resol_in; /* struct pour la résolution de nom */
  int i; // boucle for
  
  
  /* Traitement des arguments */
  if (argc!=4) { /* erreur de syntaxe */
    printf("Usage: %s addresse port interface_tun0\n",argv[0]);
    exit(1);
  }
  
  char commands[40];
  printf("Création de %s\n",argv[3]);
  tunfd = tun_alloc(argv[3]);
  sprintf(commands, "ip addr add 172.16.2.1/28 dev %s", argv[3]);
  system(commands);

  sprintf(commands, "ip link set dev %s up", argv[3]);
  system(commands);
  printf("Interface %s Configurée ! FD : %d\n", argv[3], tunfd);
  printf("Appuyez sur une touche pour terminer\n");
  getchar();

  hote=argv[1]; /* nom d'hôte du  serveur */   
  port=argv[2]; /* port TCP du serveur */   

  /* Résolution de l'hôte */
  if ( getaddrinfo(hote,port,NULL, &resol_in) < 0 ){
    perror("résolution adresse");
    exit(2);
  }
  
  /* On extrait l'addresse IP */
  if(resol_in->ai_family == AF_INET6){
    inet_ntop(AF_INET6,&(((struct sockaddr_in6*)resol_in->ai_addr)->sin6_addr), ip, sizeof(ip));
  } else {
    sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in*)resol_in->ai_addr)->sin_addr));
  }
  /* Création de la socket, de type TCP / IP */
  /* On ne considère que la première adresse renvoyée par getaddrinfo */
  if ((s_in=socket(resol_in->ai_family,resol_in->ai_socktype, resol_in->ai_protocol))<0) {
    perror("allocation de socket");
    exit(3);
  }
  fprintf(stderr,"le n° de la socket est : %i\n",s);

  /* Connexion */
  fprintf(stderr,"Essai de connexion à %s (%s) sur le port %s\n\n",
	  hote,ip,port);
  for(i=0; i<5; i++){
	  if(connected == 0){
			fprintf(stderr,"Tentative de connection %d ...\n", i);
			if (connect(s_in,resol_in->ai_addr,sizeof(struct sockaddr_in6))>=0) {
				connected = 1;
				fprintf(stderr,"Connecté !\n");
				freeaddrinfo(resol_in);
			}
			sleep(1);
	  }
  }
  fprintf(stderr,"\n\tMode serveur ...\n\n");
  
  hote=argv[1]; /* nom d'hôte du  serveur */   
  port=argv[2]; 
  fprintf(stderr,"Ecoute sur le port %s\n",port);
  
  
  err = getaddrinfo(NULL,port,&indic,&resol); 
  if (err<0){
    fprintf(stderr,"Résolution: %s\n",gai_strerror(err));
    exit(2);
  }
  
  
  /* Création de la socket, de type TCP / IP */
  if ((s=socket(resol->ai_family,resol->ai_socktype,resol->ai_protocol))<0) {
    perror("allocation de socket");
    exit(3);
  }

  /* On rend le port réutilisable rapidement /!\ */
  on = 1;
  if (setsockopt(s_in,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0) {
    perror("option socket");
    exit(4);
  }
  fprintf(stderr,"Option(s) OK!\n");

  /* Association de la socket s à l'adresse obtenue par résolution */
  if (bind(s,resol->ai_addr,sizeof(struct sockaddr_in))<0) {
    perror("bind");
    exit(5);
  }
  freeaddrinfo(resol); /* /!\ Libération mémoire */
  fprintf(stderr,"bind!\n");

  /* la socket est prête à recevoir */
  if (listen(s,SOMAXCONN)<0) {
    perror("listen");
    exit(6);
  }
  fprintf(stderr,"listen!\n");

  while(1) {
    /* attendre et gérer indéfiniment les connexions entrantes */
    len=sizeof(struct sockaddr_in);
    if( (n=accept(s,(struct sockaddr *)&client,(socklen_t*)&len)) < 0 ) {
      perror("accept");
      exit(7);
    }
    /* Nom réseau du client */
    char hotec[NI_MAXHOST];  char portc[NI_MAXSERV];
    err = getnameinfo((struct sockaddr*)&client,len,hotec,NI_MAXHOST,portc,NI_MAXSERV,0);
    if (err < 0 ){
      fprintf(stderr,"résolution client (%i): %s\n",n,gai_strerror(err));
    }else{ 
      fprintf(stderr,"accept! (%i) ip=%s port=%s\n",n,hotec,portc);
	  for(i=0; i<5; i++){
		if(connected == 0){
			fprintf(stderr,"Tentative de connection %d ...\n", i);
			if (connect(s_in,resol_in->ai_addr,sizeof(struct sockaddr_in6))>=0) {
				connected = 1;
				fprintf(stderr,"Connecté !\n");
				freeaddrinfo(resol_in);
			}
			sleep(1);
		  }
	  }
	  if(connected == 0){
		  fprintf(stderr,"Connexion echouée.\n");
		  exit(8);
	  }
    }
    /* traitement */
    echo(n,hotec,portc);
  }
  
   /* /!\ Libération mémoire */
  return EXIT_SUCCESS;
}

/* echo des messages reçus (le tout via le descripteur f) */
void echo(int f, char* hote, char* port)
{
  ssize_t lu; /* nb d'octets reçus */
  
  int pid = getpid(); /* pid du processus */
  int compteur=0;
  char tampon[MAXLIGNE + 3]; /* tampons pour les communications */
  int fini=0;
  
  int envoi1 = 0;
  int code_read = 0;
  char buf[MAX_BUFF];
  fcntl(f, F_SETFL, 0 | O_NONBLOCK);
  fcntl(tunfd, F_SETFL, 0 | O_NONBLOCK);
  do { /* Faire echo et logguer */
  
	if ( fini == 1 )
      break;  /* on sort de la boucle infinie*/
  
    lu = recv(f,tampon,MAX_BUFF,0);
    if (lu > 0 )
      {  
		char msg_recv[lu +1];
		char partial_msg[1];
		//strcpy(msg_recv, tampon);
        compteur++;
		int i;
		for(i = 0; i<lu; i++){
			sprintf(partial_msg, "%c", tampon[i]);
			strcat(msg_recv, partial_msg);
		}
		msg_recv[lu]= '\0';
		fprintf(stderr,"Taille de message recu : %d\nMessage : \n %s \n",lu, msg_recv);
        /* log */
        
        write(tunfd, msg_recv, strlen(msg_recv));
      } else {
        //Noting is received OR there is an error
      }

    code_read = read(tunfd, buf, MAX_BUFF);
    if ( code_read == -1 ){
      //Noting is sent OR there is an error
    }else{   /* envoi des données */
      envoi1 = send(s_in,buf,code_read,0);
	  
	  fprintf(stderr, "Taille du message : %d . Envoye : %d !\n", code_read, envoi1);
	  if(errno !=0){
		fprintf(stderr, "Erreur : %s ! \n", strerror(errno));
	  }
    }
  } while ( 1 );
  /* le correspondant a quitté */
  close(f);
  fprintf(stderr,"[%s:%s](%i): Terminé.\n",hote,port,pid);
}
