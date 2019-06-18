#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define DEF_PORT "1234"
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

int Accept(int sockfd,struct sockaddr *cliaddr,socklen_t *addrlen){
	int err;
	if((err=accept(sockfd,cliaddr,addrlen)) < 0){
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

ssize_t Sendto(int sockfd,void *buff,size_t nbytes,int flags,const struct sockaddr* to,socklen_t addrlen){
	int err;
	if((err=sendto(sockfd,buff,nbytes,flags,to,addrlen)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
} 

void append(char* s, char c){
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

int main(int argc, char *argv[]) {
	if(argc < 1){
		fprintf(stderr,"Usage: ./tcpserver [-p port]\n");
		exit(1);
	}
	
	int option;
	char port[22]=DEF_PORT;
	
	while((option=getopt(argc,argv,"p:")) != -1){
		switch(option){
			case 'p':	
						strcpy(port,optarg);
						break;
			default:	fprintf(stderr,"Usage: ./tcpserver [-p port]\n");
						exit(1);
		}
	}
	
	printf("%s\n",port);
	
	int mysock, newsock;
	struct addrinfo hints;
	struct addrinfo *res;	
	
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;
	
	Getaddrinfo(NULL,port,&hints,&res);
	mysock=Socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	Bind(mysock,res->ai_addr,res->ai_addrlen);
	Listen(mysock,BACKLOG);
	
	while(1){
		struct sockaddr_in their_addr;
		socklen_t sin_size;
		sin_size = sizeof their_addr;
		newsock=Accept(mysock,(struct sockaddr *)&their_addr,&sin_size);
	
		char buffer[256];
		memset(buffer,0,sizeof(buffer));
		Recvfrom(newsock,buffer,sizeof(buffer),0,(struct sockaddr *)&their_addr,&sin_size);
		printf("primljeno: %s\n",buffer);
		
		char offset[4+1];
		substring(buffer,offset,5,1);
		printf("offset: %s\n",offset);
		char naziv_datoteke[200];
		int j=0;
		for(int i=5;buffer[i] != '\0';i++){
		naziv_datoteke[j]=buffer[i];
		j++;
		}
		naziv_datoteke[j]='\0';
		printf("naziv_datoteke: %s\n",naziv_datoteke);
		uint32_t off=ntohl((uint32_t)atoi(offset));
		
		int error;
		if((error=access(naziv_datoteke,F_OK)) < 0){
			char sendBuff[256]="0x01";
			strcat(sendBuff,strerror(errno));
			Sendto(newsock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&their_addr,sin_size);
		}
		if((error=access(naziv_datoteke,R_OK)) < 0){
			char sendBuff[256]="0x02";
			strcat(sendBuff,strerror(errno));
			Sendto(newsock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&their_addr,sin_size);
		}
		if((error=access(naziv_datoteke,0)) < 0){
			char sendBuff[256]="0x03";
			strcat(sendBuff,strerror(errno));
			Sendto(newsock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&their_addr,sin_size);
		}
		char extension[256];
		substring(naziv_datoteke,extension,4,strlen(naziv_datoteke)-3);
		
		if(strcmp(extension,".txt") == 0){
			FILE *tokDat=fopen(naziv_datoteke,"r");
			if(tokDat == NULL){
				char sendBuff[256]="0x03";
				strcat(sendBuff,"Ne mogu otvoriti datoteku.\n");
				Sendto(newsock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&their_addr,sin_size);
			}
			fseek(tokDat,(long)off,SEEK_CUR);
			int c;
			char *content=(char *) malloc(sizeof(char));
			while((c=fgetc(tokDat)) != EOF){
				append(content,(char)c);
			}
			Sendto(newsock,content,strlen(content),0,(struct sockaddr *)&their_addr,sin_size);
			free(content);
			fclose(tokDat);
		}
		else{
			FILE *tokDat=fopen(naziv_datoteke,"rb");
			if(tokDat == NULL){
				char sendBuff[256]="0x03";
				strcat(sendBuff,"Ne mogu otvoriti datoteku.\n");
				Sendto(newsock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&their_addr,sin_size);
			}
			fseek(tokDat,(long)off,SEEK_CUR);
			long content[50000];
			int n;
			n=fread(content,sizeof(content[0]),50000,tokDat);
			Sendto(newsock,content,n,0,(struct sockaddr *)&their_addr,sin_size);
			fclose(tokDat);
		}
		close(newsock);
	}	
}
