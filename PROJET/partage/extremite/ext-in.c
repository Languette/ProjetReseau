/*  echo / client simple
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

#define MAXLIGNE 64
#define STR_SIZE 1024

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

int main(int argc, char *argv[])
{
  char * hote; /* nom d'hôte du  serveur */   
  char * port; /* port TCP du serveur */   
  char ip[INET6_ADDRSTRLEN]; /* adresse IPv4 en notation pointée */
  struct addrinfo *resol; /* struct pour la résolution de nom */
  int s; /* descripteur de socket */
  
  int tunfd;


  /* Traitement des arguments */
  if (argc!=4) {/* erreur de syntaxe */
    printf("Usage: %s hote port nom_d'interface\n",argv[0]);
    exit(1);
  }
  char commands[40];
  printf("Création de %s\n",argv[3]);
  tunfd = tun_alloc(argv[3]);
  sprintf(commands, "ip addr add 172.16.2.1/28 dev %s", argv[3]);
  system(commands);

  sprintf(commands, "ip link set dev %s up", argv[3]);
  system(commands);
  printf("Interface %s Configurée:\n", argv[3]);
  printf("Appuyez sur une touche pour terminer\n");
  getchar();

  hote=argv[1]; /* nom d'hôte du  serveur */   
  port=argv[2]; /* port TCP du serveur */   

  /* Résolution de l'hôte */
  if ( getaddrinfo(hote,port,NULL, &resol) < 0 ){
    perror("résolution adresse");
    exit(2);
  }
  
  /* On extrait l'addresse IP */
  if(resol->ai_family == AF_INET6){
    inet_ntop(AF_INET6,&(((struct sockaddr_in6*)resol->ai_addr)->sin6_addr), ip, sizeof(ip));
  } else {
    sprintf(ip, "%s", inet_ntoa(((struct sockaddr_in*)resol->ai_addr)->sin_addr));
  }
  /* Création de la socket, de type TCP / IP */
  /* On ne considère que la première adresse renvoyée par getaddrinfo */
  if ((s=socket(resol->ai_family,resol->ai_socktype, resol->ai_protocol))<0) {
    perror("allocation de socket");
    exit(3);
  }
  fprintf(stderr,"le n° de la socket est : %i\n",s);

  /* Connexion */
  fprintf(stderr,"Essai de connexion à %s (%s) sur le port %s\n\n",
	  hote,ip,port);
  if (connect(s,resol->ai_addr,sizeof(struct sockaddr_in6))<0) {
    perror("connexion");
    exit(4);
  }
  freeaddrinfo(resol); /* /!\ Libération mémoire */
  fprintf(stderr,"Connexion reussie\n\n");
  /* Session */
  char tampon[MAXLIGNE + 3]; /* tampons pour les communications */

  int fini=0;
  
  int envoi1 = 0;
  int code_read = 0;
  char buf[STR_SIZE];
  
  while( 1 ) { 

    if ( fini == 1 )
      break;  /* on sort de la boucle infinie*/

   code_read = read(tunfd, buf, STR_SIZE);
    if ( code_read == -1 ){
	  fini=1;
      fprintf(stderr,"Connexion terminée !!\n");
      fprintf(stderr,"Hôte distant informé...\n");
      shutdown(s, SHUT_WR);
    }else{   /* envoi des données */
      envoi1 = send(s,buf,code_read,0);
	  
	fprintf(stderr, "Taille du message : %d . Envoye : %d !\n", code_read, envoi1);
	  if(errno !=0){
		fprintf(stderr, "Erreur : %s ! \n", strerror(errno));
	  }
    }
  } 
  
  /* Destruction de la socket */
  close(s);

  fprintf(stderr,"Fin de la session.\n");
  return EXIT_SUCCESS;
}
