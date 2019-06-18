#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>
#include <string.h>

int Inet_pton(int af,const char *src,void *dst){
	int err;
	if((err=inet_pton(af,src,dst)) != 1){
		fprintf(stderr,"IP adress is not valid\n");
		exit(1);
	}
	return err;
}

int Getnameinfo(const struct sockaddr *sockaddr,socklen_t addrlen,
				char *host,size_t hostlen,char *serv,size_t servlen,int flags){
	int err;
	if((err=getnameinfo((struct sockaddr *)sockaddr,addrlen,
						host,hostlen,serv,servlen,flags)) != 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int Getaddrinfo(const char *hostname,const char *service,const struct addrinfo *hints,struct addrinfo **result){
	int err;
	if((err=getaddrinfo(hostname,service,hints,result)) != 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int main(int argc, char *argv[]) {
	if(argc < 3){
		fprintf(stderr,"prog [-r] [-t|-u] [-x] [-h|-n] [-46] {hostname|IP_address} {servicename|port}\n");
		exit(1);
	}
	
	int option;
	int t=1;
	int u=0;
	int x=0;
	int h=1;
	int n=0;
	int ip4=1;
	int ip6=0;
	int r=0;
	
	while((option=getopt(argc,argv,"tuxhnr64")) != -1){
		switch(option){
			case 't':	t=1;
						break;
			case 'u':	u=1;
						t=0;
						break;
			case 'x':	x=1;
						break;
			case 'h':	h=1;
						break;
			case 'n': 	n=1;
						h=0;
						break;
			case 'r':	r=1;
						break;
			case '6':	ip6=1;
						ip4=0;
						break;
			case '4':	ip4=1;
						break;
			default:	fprintf(stderr,"Wrong argument\n");
						exit(1);
		}
	}
	
	if(r == 1){
		char hostname[NI_MAXHOST];
		char servicename[NI_MAXSERV];		
		char *adress=argv[argc-2];
		char *ptr=argv[argc-1];
		for(;*ptr != '\0'; ++ptr);
		unsigned short int port=(unsigned short int)strtoul(argv[argc-1],&ptr,10);
		if((ip4==1) && (ip6==0)){
			struct sockaddr_in sa;
			sa.sin_family=AF_INET;
			inet_pton(AF_INET,adress,&(sa.sin_addr));
			sa.sin_port=htons(port);			
			
			if((t==1) && (u==0)){
				Getnameinfo((struct sockaddr *)&sa,sizeof(struct sockaddr_in),hostname,sizeof(hostname),
								servicename,sizeof(servicename),NI_NAMEREQD);
			}
			if((u==1) && (t==0)){
				Getnameinfo((struct sockaddr *)&sa,sizeof(struct sockaddr_in),hostname,sizeof(hostname),
								servicename,sizeof(servicename),NI_NAMEREQD | NI_DGRAM);
			}
		}
		if((ip6==1) && (ip4==0)){
			struct sockaddr_in6 sa;
			sa.sin6_family=AF_INET6;
			inet_pton(AF_INET6,adress,&(sa.sin6_addr));
			sa.sin6_port=htons(port);			
			
			if((t==1) && (u==0)){
				Getnameinfo((struct sockaddr *)&sa,sizeof(struct sockaddr_in6),hostname,sizeof(hostname),
								servicename,sizeof(servicename),NI_NAMEREQD);
			}
			if((u==1) && (t==0)){
				Getnameinfo((struct sockaddr *)&sa,sizeof(struct sockaddr_in6),hostname,sizeof(hostname),
								servicename,sizeof(servicename),NI_NAMEREQD | NI_DGRAM);
			}
		}
		printf("%s (%s) %s\n", adress,hostname,servicename);
		return 0;
	}
	else{
		char *hostname=argv[argc-2];
		char *servicename=argv[argc-1];
		struct addrinfo hints;
		struct addrinfo *res;
		memset(&hints,0,sizeof(hints));
		if((ip4==1) && (ip6==0)){
			char ip[INET_ADDRSTRLEN];									
			hints.ai_family=AF_INET;
			hints.ai_flags=AI_CANONNAME;
						
			if((t==1) && (u==0)){
				hints.ai_socktype=SOCK_STREAM;
				hints.ai_protocol=IPPROTO_TCP;
			}
			if((u==1) && (t==0)){
				hints.ai_socktype=SOCK_DGRAM;
				hints.ai_protocol=IPPROTO_UDP;
			}
			Getaddrinfo(hostname,servicename,&hints,&res);
			inet_ntop(res->ai_family,&((struct sockaddr_in *)res->ai_addr)->sin_addr,
					ip,sizeof(ip));
			printf("%s (%s) ",ip,res->ai_canonname);
			if(x==0){
				if((h==1) && (n==0)){
					printf("%d\n",ntohs(*(&((struct sockaddr_in *)res->ai_addr)->sin_port)));
				}
				if((n==1) && (h==0)){
					printf("%d\n",*(&((struct sockaddr_in *)res->ai_addr)->sin_port));
				}
			}
			else{
				if((h==1) && (n==0)){
					printf("%04x\n",ntohs(*(&((struct sockaddr_in *)res->ai_addr)->sin_port)));
				}
				if((n==1) && (h==0)){
					printf("%04x\n",*(&((struct sockaddr_in *)res->ai_addr)->sin_port));
				}
			}						
		}
		if((ip6==1) && (ip4==0)){
			char ip[INET6_ADDRSTRLEN];
			hints.ai_family=AF_INET6;
			hints.ai_flags=AI_CANONNAME;
			
			if((t==1) && (u==0)){
				hints.ai_socktype=SOCK_STREAM;
				hints.ai_protocol=IPPROTO_TCP;
			}
			if((u==1) && (t==0)){
				hints.ai_socktype=SOCK_DGRAM;
				hints.ai_protocol=IPPROTO_UDP;
			}
			Getaddrinfo(hostname,servicename,&hints,&res);
			inet_ntop(res->ai_family,&((struct sockaddr_in6 *)res->ai_addr)->sin6_addr,
					ip,sizeof(ip));
			printf("%s (%s) ",ip,res->ai_canonname);
			if(x==0){
				if((h==1) && (n==0)){
					printf("%d\n",ntohs(*(&((struct sockaddr_in6 *)res->ai_addr)->sin6_port)));
				}
				if((n==1) && (h==0)){
					printf("%d\n",*(&((struct sockaddr_in6 *)res->ai_addr)->sin6_port));
				}
			}
			else{
				if((h==1) && (n==0)){
					printf("%04x\n",ntohs(*(&((struct sockaddr_in6 *)res->ai_addr)->sin6_port)));
				}
				if((n==1) && (h==0)){
					printf("%04x\n",*(&((struct sockaddr_in6 *)res->ai_addr)->sin6_port));
				}
			}		
		}
		freeaddrinfo(res);
		return 0;
	}	
	return 0;
}
