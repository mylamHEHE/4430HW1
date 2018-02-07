# include <stdio.h>
# include <stdlib.h>
#include <string.h>
# include "myftp.h"
//# define PORT 12346

int main(int argc, char** argv){
	int sd=socket(AF_INET,SOCK_STREAM,0);
	//checkprintf("client_main\n");
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=inet_addr(argv[1]);
	server_addr.sin_port=htons(atoi(argv[2]));

	if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		printf("connection error: %s (Errno:%d)\n",strerror(errno),errno);
		exit(0);
	}
	int le=0x12345678;
//	printf("%x\n",le);
	le=htonl(le);
//	printf("%x\n",le);
	int count=0;

	do{
	//check	printf("client_while: %s\n", argv[3]);
		char buff[100],server_rep[2000];
		struct message_s msg;
		struct message_s recvmsg;
		memset(buff,0,100);
		memset(msg.protocol,0,5);
		strcpy(buff,argv[3]);
		int len,client_sd=socket(AF_INET,SOCK_STREAM,0);                
		if(strcmp(buff,"list")==0)
		{
			char protocolname[5]="myftp";

			strncpy(msg.protocol,protocolname,5);
			msg.protocol[5]='\0';
			msg.type=LIST_REQUEST;
			msg.length=sizeof(msg);
			if((len=sendn(sd,&msg,sizeof(msg)))<0){
				printf("send error: %s (Errno:%d)\n", strerror(errno),errno);
				exit(0);
			}	
		}
		if(strcmp(buff,"put")==0)
		{
			char protocolname[5]="myftp";

			strncpy(msg.protocol,protocolname,5);
			msg.protocol[5]='\0';
			char sendputstring[strlen(argv[4])+1];
			if(access(argv[4], R_OK)==0)
			{
				strcpy(sendputstring,argv[4]);
				char protocolname[6]="myftp";

				strcpy(msg.protocol,protocolname);
				msg.type=PUT_REQUEST;
				msg.length=sizeof(struct message_s)+sizeof(sendputstring);

				//checkprintf("%x\n", msg.type);
				if(len=sendn(sd,&msg,sizeof(struct message_s))<0)
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					fflush(stdout);
					exit(0);

				}
				if(len=sendn(sd,sendputstring,sizeof(sendputstring))<0)
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					fflush(stdout);
					exit(0);

				}
				//checkprintf("Sended, thx\n");
			}
			else{
				printf("The file doesn't exist.\n");break;
			}
		}


		if((len=recvn(sd,&recvmsg,sizeof(struct message_s))<0))
		{
			printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
			exit(0);

		}	
		if(sizeof(recvmsg)!=0){	
			//checkprintf("Something Received\n");
			if(recvmsg.type==LIST_REPLY)
			{
				int listlength=recvmsg.length-sizeof(recvmsg);
				char receivelist[listlength];
				if((len=recvn(sd,&receivelist,sizeof(receivelist))<0))
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					exit(0);
				}	
				printf("Files in \\data directory:\n\n");
				receivelist[listlength]='\0';
			//check	printf("%s",receivelist);
				fflush(stdout);
			}
			if(recvmsg.type==PUT_REPLY)
			{
				//checkprintf("Reply received\n");

				char protocolname[5]="myftp";
				strncpy(msg.protocol,protocolname,5);
				char *code = malloc(1000 * sizeof(char));//just added
				FILE* file = fopen(argv[4], "r");
				fseek(file, 0L, SEEK_END);
				int fileSize = ftell(file);
				fseek(file, 0L, SEEK_SET);

				struct message_s sendmsg;

				sendmsg.protocol[5]='\0';
				sendmsg.type=FILE_DATA;
				sendmsg.length=sizeof(sendmsg)+fileSize;

				//checkprintf("file Size:%d\n ", fileSize);
				if(len=sendn(sd,&sendmsg,sizeof(sendmsg))<0)
				{
					printf("receive errorA: %s (Errno:%d)\n", strerror(errno),errno);
					fflush(stdout);
					exit(0);
				}
				// if(len=sendn(sd,file,fileSize)<0)
				// {
				// 	printf("receive errorB: %s (Errno:%d)\n", strerror(errno),errno);
				// 	fflush(stdout);
				// 	exit(0);
				// }


				//just added
			//check	printf("\n");
				do 
				{
					*code = (char)fgetc(file);
					if(*code == EOF) break;
					//check contect printf("%c", *code);
					sendn(sd, code, 1);
					code++;
				} while(1);

			//check	printf("\n");

				//just added end
			//check	printf("Success\n");
			}
		}
	}
	while(count!=0);
	// while(1);
	return 0;
}
