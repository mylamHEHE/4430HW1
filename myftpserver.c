# include <stdio.h>
# include <stdlib.h>
# include "myftp.h"
#include <dirent.h>
//# define PORT 12346
char* list(){
	struct dirent *pStResult = NULL;
	struct dirent *pStEntry = NULL;
	int len = 0;

	DIR *pDir = opendir("./data");
	if(NULL == pDir)
	{
		printf("Opendir failed!\n");
		exit(0);
	}

	len = offsetof(struct dirent, d_name) + pathconf("./data", _PC_NAME_MAX) + 1;
	pStEntry = (struct dirent*)malloc(len);
	char *returningstring=(char *)malloc(len);

	while(! readdir_r(pDir, pStEntry, &pStResult) && pStResult != NULL)
	{
		strcat(returningstring,pStResult->d_name);
		strcat(returningstring,"\n");
	}

	free(pStEntry);
	closedir(pDir);
	return returningstring;
}

struct message_s makepack(int type,char *length)
{
	struct message_s msg;
	int payload = strlen(length);

	int packet_size;
	char protocolname[5]="myftp";

	switch(type)
	{
		case LIST_REQUEST:

		strncpy(msg.protocol,protocolname,5);
		msg.protocol[5]='\0';
		msg.type=LIST_REPLY;
		
		break;

	} 
	packet_size = sizeof(struct message_s)+payload;
	msg.length=packet_size; 
	return msg;
}

int main(int argc, char* argv[]){
	int sd=socket(AF_INET,SOCK_STREAM,0);
	long val=1;
	if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(long))==-1)
	{
		perror("setsockopt");exit(1);
	}
	int client_sd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(atoi(argv[1]));
	if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0){
		printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	if(listen(sd,3)<0){
		printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	int addr_len=sizeof(client_addr);
	if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
		printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	while(1){
		char buff[100];
		int len;
		struct message_s revmsg;
		struct message_s sendmsg;
		
		if((len=recvn(client_sd,&revmsg,sizeof(revmsg)))<0){
			printf("receive error1: %s (Errno:%d)\n", strerror(errno),errno);
			exit(0);
		}
		if(sizeof(revmsg)!=0){
			if(revmsg.type==LIST_REQUEST){
			//not fixed

				char sendliststring[strlen(list())];

				strcpy(sendliststring,list());
				sendmsg=makepack(LIST_REQUEST,sendliststring);
				if(len=sendn(client_sd,&sendmsg,sizeof(sendmsg))<0)
				{
					printf("receive error2: %s (Errno:%d)\n", strerror(errno),errno);fflush(stdout);
					exit(0);

				}
				if(len=sendn(client_sd,sendliststring,sizeof(sendliststring))<0)
				{
					printf("receive error3: %s (Errno:%d)\n", strerror(errno),errno);fflush(stdout);
					exit(0);

				}

			}




		}		

		
	}
	close(sd);
	return 0;
}
