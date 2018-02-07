# include <stdio.h>
# include <stdlib.h>
# include "myftp.h"
#include <dirent.h>
//# define PORT 12346
struct message_s makepack(int type,char *length)
{
	struct message_s msg;
	int payload = strlen(length);

	int packet_size;
	char protocolname[5]="myftp";
//	printf("server_main\n");
	switch(type)
	{
		//printf("server_switch\n");
		case LIST_REQUEST:

		strncpy(msg.protocol,protocolname,5);
		msg.protocol[5]='\0';
		msg.type=LIST_REPLY;
		
		break;

		case PUT_REQUEST:

		strncpy(msg.protocol,protocolname,5);
		msg.protocol[5]='\0';
		msg.type=PUT_REPLY;
		
		break;
	} 
	packet_size = sizeof(struct message_s)+payload;
	msg.length=packet_size; 
	return msg;
}
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

int main(int argc, char* argv[])
{
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
	if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0)
	{
		printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	if(listen(sd,3)<0)
	{
		printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	int addr_len=sizeof(client_addr);
	if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0)
	{
		printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}

	char* filename;

	while(1){
		char buff[100];
		int len;
		struct message_s revmsg;
		struct message_s sendmsg;
		
		if((len=recvn(client_sd,&revmsg,sizeof(revmsg)))<0)
		{
			printf("receive error1: %s (Errno:%d)\n", strerror(errno),errno);
			exit(0);
		}else{
			if(revmsg.type==LIST_REQUEST)
			{
				char sendliststring[strlen(list())];

				strcpy(sendliststring,list());
				sendmsg=makepack(LIST_REQUEST,sendliststring);

				if(len=sendn(client_sd,&sendmsg,sizeof(sendmsg))<0)
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					fflush(stdout);
					exit(0);

				}
				if(len=sendn(client_sd,sendliststring,sizeof(sendliststring))<0)
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					fflush(stdout);
					exit(0);
				}
			}
			if(revmsg.type==PUT_REQUEST)
			{	
				//printf("received with thanks\n");
				int listlength=revmsg.length-sizeof(struct message_s);
				char receivelist[listlength+1];
				if((len=recvn(client_sd,&receivelist,listlength)<0))
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					exit(0);
				}
			//	printf("Filename is :%s", receivelist);
				char* dir = "data/";
				filename = malloc(strlen(dir)+strlen(receivelist)+1);
				strcpy(filename, dir);
				strcat(filename, receivelist);
				char protocolname[6]="myftp";
				strcpy(sendmsg.protocol,protocolname);
				sendmsg.type=PUT_REPLY;
				sendmsg.length=sizeof(struct message_s);
				// sendmsg=makepack(PUT_REQUEST, "\0");
				if(len=sendn(client_sd,&sendmsg,sizeof(sendmsg))<0)
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					fflush(stdout);
					exit(0);
				}
				//printf("Server send success! Yeah!\n");
			}
			if(revmsg.type==FILE_DATA)
			{	
				//printf("FILE RECEIVED\n");
				int filelength=revmsg.length-sizeof(struct message_s);
				FILE* receiveFile = fopen(filename, "w");
				// if((len=recvn(client_sd,receiveFile,filelength)<0))
				// {
				// 	printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
				// 	exit(0);
				// }
				for(int i=0; i<filelength; i++){
					char a='\0';
					if((len=recvn(client_sd,&a,1)<0))
					{
						printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
						exit(0);
					}
				//	printf("%c", a);
					fprintf(receiveFile, "%c", a);
				}
			//	printf("\n");

			//	printf("Saved to %s...", filename);
				fclose(receiveFile);
			}
		}
		revmsg.type = NONE;
	}
	close(sd);
	return 0;
}
