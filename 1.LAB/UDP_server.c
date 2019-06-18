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
#define DEF_PAYLOAD NULL

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

int main(int argc, char *argv[]) {
	if(argc < 1){
		fprintf(stderr,"Usage: ./UDP_server [-l port] [-p payload]\n");
		exit(1);
	}
	
	int option;
	char port[22]={'\0'};
	char payload[512]={'\0'};
	
	while((option=getopt(argc,argv,"l:p:")) != -1){
		switch(option){
			case 'l':	
						strcpy(port,optarg);
						break;
			case 'p':	
						strcpy(payload,optarg);
						break;
			default:	fprintf(stderr,"Wrong argument\n");
						exit(1);
		}
	}
	
	int mysock;
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr client;
	socklen_t clilen=sizeof(client);
	
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_DGRAM;
	hints.ai_flags=AI_PASSIVE;
	
	if(port[0] == '\0'){
		Getaddrinfo(NULL,DEF_PORT,&hints,&res);
	}
	else{
		Getaddrinfo(NULL,port,&hints,&res);
	}
	
	mysock=Socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	Bind(mysock,res->ai_addr,res->ai_addrlen);
	
	while(1){
		int recv;
		char buffer[256];
		recv=Recvfrom(mysock,buffer,sizeof(buffer),0,&client,&clilen);
		printf("primljeno: %s",buffer);
		if(payload[0] == '\0'){
			Sendto(mysock,DEF_PAYLOAD,0,0,&client,clilen);
		}
		else{
			int size=0;
			for(;payload[size] != '\0'; ++size);
			size *=sizeof(payload[0]);
			Sendto(mysock,payload,size+1,0,&client,clilen);
		}		
	}
	
}
