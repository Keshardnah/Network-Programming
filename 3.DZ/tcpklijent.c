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
#define DEF_SERVER "127.0.0.1"

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

int Connect(int sockfd,const struct sockaddr *server,socklen_t addrlen){
	int err;
	if((err=connect(sockfd,server,addrlen)) < 0){
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
	if(argc < 2){
		fprintf(stderr,"Usage: ./tcpklijent [-s server] [-p port] [-c] filename\n");
		exit(1);
	}
	
	int option;
	char port[22]=DEF_PORT;
	char server[256]=DEF_SERVER;
	int c=0;
	
	while((option=getopt(argc,argv,"s:p:c")) != -1){
		switch(option){
			case 's':	
						strcpy(server,optarg);
						break;
			case 'p':	
						strcpy(port,optarg);
						break;
			case 'c':	
						c=1;
						break;
			default:	fprintf(stderr,"Usage: ./tcpklijent [-s server] [-p port] [-c] filename\n");
						exit(1);
		}
	}
	
	char *filenameP=argv[argc-1];
	char filename[200];
	strcpy(filename,filenameP);
	int mysock;
	struct addrinfo hints;
	struct addrinfo *res;
	
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	
	Getaddrinfo(server,port,&hints,&res);
	mysock=Socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	Connect(mysock,res->ai_addr,res->ai_addrlen);
		
	if(c == 0){	
		int error;
		if((error=access(filename,F_OK)) == 0){
			errx(1,"%s","Datoteka vec postoji");
		}	
		struct off_file{
			char offset[4+1];
			char filenameS[200];
		} buff;
		uint32_t off=htonl((uint32_t)0);
		sprintf(buff.offset,"%u",off);
		strcpy(buff.filenameS,filename);
		Sendto(mysock,&buff,205,0,(struct sockaddr *)res->ai_addr,res->ai_addrlen);
	}
	else{
		int error;
		struct off_file{
			char offset[4+1];
			char filenameS[200];
		} buff;
		uint32_t off;
		if((error=access(filename,F_OK)) < 0){
			off=htonl((uint32_t)0);
		}
		else{
			FILE *tokDat=fopen(filename,"r");
			off=(uint32_t)ftell(tokDat);
		}	
		sprintf(buff.offset,"%u",off);
		strcpy(buff.filenameS,filename);
		Sendto(mysock,&buff,205,0,(struct sockaddr *)res->ai_addr,res->ai_addrlen);
	}
	char buffer[50000+1];
	Recvfrom(mysock,buffer,sizeof(buffer),0,(struct sockaddr *)res->ai_addr,&(res->ai_addrlen));
	char status[4+1];
	substring(buffer,status,4,1);
	char message[49996+1];
	int j=0;
	for(int i=4;buffer[i] != '\0';i++){
		message[j]=buffer[i];
		j++;
	}
	message[j]='\0';
	
	if(strcmp(status,"0x01") == 0){
		errx(1,"%s",message);
	}
	if(strcmp(status,"0x02") == 0){
		errx(2,"%s",message);
	}
	if(strcmp(status,"0x03") == 0){
		errx(3,"%s",message);
	}
	if(strcmp(status,"0x00") == 0){
		char extension[256];
		substring(filename,extension,4,strlen(filename)-3);
		if(strcmp(extension,".txt") == 0){
			if(c == 0){
				FILE *tokDat=fopen(filename,"w");
				fputs(message,tokDat);
				exit(0);
			}
			else{
				int error;
				if((error=access(filename,F_OK)) < 0){
					FILE *tokDat=fopen(filename,"w");
					fputs(message,tokDat);
					exit(0);
				}
				else{
					if((error=access(filename,W_OK)) < 0){
						errx(4,"%s","Nemam dozvolu pisanja u datoteku");
					}
					FILE *tokDat=fopen(filename,"a");
					fputs(message,tokDat);
					exit(0);
				}
			}
		}
		else{
			if(c == 0){
				FILE *tokDat=fopen(filename,"wb");
				fwrite(message,sizeof(message[0]),49996,tokDat);
				exit(0);
			}
			else{
				int error;
				if((error=access(filename,F_OK)) < 0){
					FILE *tokDat=fopen(filename,"wb");
					fwrite(message,sizeof(message[0]),49996,tokDat);
					exit(0);
				}
				else{
					if((error=access(filename,W_OK)) < 0){
						errx(4,"%s","Nemam dozvolu pisanja u datoteku");
					}
					FILE *tokDat=fopen(filename,"ab");
					fwrite(message,sizeof(message[0]),49996,tokDat);
					exit(0);
				}
			}
		}
	}
}
