#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define DEF_PORT "1234"
#define DEF_PAYLOAD ""
#define DEF_STDIN 0
#define BACKLOG 100

int Getaddrinfo(const char *hostname,const char *service,const struct addrinfo *hints,struct addrinfo **result){
	int err;
	if((err=getaddrinfo(hostname,service,hints,result)) != 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int Socket(int family,int type,int protocol){
	int err;
	if((err=socket(family,type,protocol)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int Bind(int sockfd,const struct sockaddr *myaddr,int addrlen){
	int err;
	if((err=bind(sockfd,myaddr,addrlen)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int Listen(int sockfd,int backlog){
	int err;
	if((err=listen(sockfd,backlog)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

ssize_t Recvfrom(int sockfd,void *buff,size_t nbytes,int flags,struct sockaddr *from,socklen_t *fromaddrlen){
	int err;
	if((err=recvfrom(sockfd,buff,nbytes,flags,from,fromaddrlen)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

ssize_t Sendto(int sockfd,void *buff,size_t nbytes,int flags,const struct sockaddr* to,socklen_t addrlen){
	int err;
	if((err=sendto(sockfd,buff,nbytes,flags,to,addrlen)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int Accept(int sockfd,struct sockaddr *cliaddr,socklen_t *addrlen){
	int err;
	if((err=accept(sockfd,cliaddr,addrlen)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int find_max(int a, int b, int c){
	int max=a;
	if(b > max) max=b;
	if(c > max) max=c;
	return max;
}

ssize_t Recv(int sockfd,void *buff,size_t nbytes,int flags){
	int err;
	if((err=recv(sockfd,buff,nbytes,flags)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

ssize_t Send(int sockfd,void *buff,size_t nbytes,int flags){
	int err;
	if((err=send(sockfd,buff,nbytes,flags)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int main(int argc, char *argv[]) {
	if(argc < 1){
		fprintf(stderr,"Usage: ./server [-t tcp_port] [-u udp_port] [-p popis]\n");
		exit(1);
	}
	
	int option;
	char tcp_port[22]=DEF_PORT; //naziv ili broj TCP porta na kojem slusa 
	char udp_port[22]=DEF_PORT; //naziv ili broj UDP porta na kojem slusa 
	char payload[1024+1]=DEF_PAYLOAD; //popis payloadova odvojenih dvotočkom (:) koje će program bot slati zrtvama
	
	while((option=getopt(argc,argv,"t:u:p:")) != -1){
		switch(option){
			case 't':	
						strcpy(tcp_port,optarg);
						break;
			case 'u':	
						strcpy(udp_port,optarg);
						break;
			case 'p':	
						strcpy(payload,optarg);
						break;
			default:	fprintf(stderr,"Wrong argument\n");
						exit(1);
		}
	}
	
	int udp_sock;
	int tcp_sock;
	int stdin_des=DEF_STDIN; // STDIN deskriptor
	
	struct addrinfo hints1; // radimo strukturu adrese za UDP port
	struct addrinfo *res1;
	memset(&hints1,0,sizeof(hints1));
	hints1.ai_family=AF_INET;
	hints1.ai_socktype=SOCK_DGRAM;
	hints1.ai_flags=AI_PASSIVE;
	Getaddrinfo(NULL,udp_port,&hints1,&res1);
	udp_sock=Socket(res1->ai_family,res1->ai_socktype,res1->ai_protocol); // postavljamo socket za UDP
	Bind(udp_sock,res1->ai_addr,res1->ai_addrlen); // povezujemo socket i adresu
	
	struct addrinfo hints2; // radimo strukturu adrese za TCP port
	struct addrinfo *res2;
	memset(&hints2,0,sizeof(hints2));
	hints2.ai_family=AF_INET;
	hints2.ai_socktype=SOCK_STREAM;
	hints2.ai_flags=AI_PASSIVE;
	Getaddrinfo(NULL,tcp_port,&hints2,&res2);
	tcp_sock=Socket(res2->ai_family,res2->ai_socktype,res2->ai_protocol); // postavljamo socket za TCP
	Bind(tcp_sock,res2->ai_addr,res2->ai_addrlen); // povezujemo socket i adresu
	Listen(tcp_sock,BACKLOG); 
	
	int max=find_max(tcp_sock,udp_sock,stdin_des);
	
	fd_set readfds; // skup deskriptora koje zelimo citati
	
	while(1){
		
		struct timeval tv; // vrijeme blokiranja
		FD_ZERO(&readfds); // ocistimo skup
		FD_SET(udp_sock, &readfds); // dodajemo deskriptore koje zelimo nadzirati
		FD_SET(tcp_sock, &readfds);
		FD_SET(stdin_des, &readfds);
		tv.tv_sec = 3;
		tv.tv_usec = 500000;
		int rv = select(max+1, &readfds, NULL, NULL, &tv);
		if (rv == -1) {
			perror("select");
		}
		if (FD_ISSET(udp_sock, &readfds)) {
			struct sockaddr client;
			socklen_t clilen=sizeof(client);
			char buffer[256];
			memset(buffer,0,sizeof(buffer));
			Recvfrom(udp_sock,buffer,sizeof(buffer),0,&client,&clilen);
			printf("primljeno preko udp:\n%s\n",buffer);
			if(! strcmp(buffer,"HELLO\n")){
				char payload_new[1024+1];
				strcpy(payload_new,strcat(payload,"\n"));
				Sendto(udp_sock,payload_new,strlen(payload_new),0,&client,clilen);
				printf("poslano preko udp:\n%s\n",payload_new);
				payload_new[strlen(payload_new)-1]='\0';
			}
		}
		if (FD_ISSET(tcp_sock, &readfds)) {
			struct sockaddr_in client;
			socklen_t clilen;
			clilen = sizeof client;
			int newsock=Accept(tcp_sock,(struct sockaddr *)&client,&clilen); 
			char buffer[256];
			memset(buffer,0,sizeof(buffer));
			Recv(newsock,buffer,sizeof(buffer),0);
			printf("primljeno preko tcp:\n%s\n",buffer);
			if(! strcmp(buffer,"HELLO\n")){
				char payload_new[1024+1];
				strcpy(payload_new,strcat(payload,"\n"));
				Send(newsock,payload_new,strlen(payload_new),0);
				printf("poslano preko tcp:\n%s\n",payload_new);
				payload_new[strlen(payload_new)-1]='\0';
			}
		}
		if (FD_ISSET(stdin_des, &readfds)) {
			char buffer[256];
			gets(buffer);
			printf("primljeno preko stdin:\n%s\n",buffer);
			if(! strcmp(buffer,"QUIT")){ // prestaje s radom uz izlazni status jednak 0
				exit(0);
			}
			if(! strcmp(buffer,"PRINT")){ // na standardni izlaz (stdout) ispisuje trenutno spremljeni popis payloadova
				char payload_new[1024+1];
				strcpy(payload_new,strcat(payload,"\n"));
				printf("trenutno spremljeni popis payloadova:\n%s\n",payload_new);
				payload_new[strlen(payload_new)-1]='\0';
			}
			if(! strncmp(buffer,"SET",3)){ // SET novi_popis, popis payloadova postavlja na "novi_popis"
				char payload_new[1024+1];
				int i=4;
				for(;buffer[i] != '\0';i++){
					payload_new[i-4]=buffer[i];
				}
				payload_new[i-4]='\0';
				strcpy(payload,payload_new);
			}
		}
	}
}
