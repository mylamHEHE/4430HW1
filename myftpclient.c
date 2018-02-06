# include <stdio.h>
# include <stdlib.h>
#include <string.h>
# include "myftp.h"
//# define PORT 12346

int main(int argc, char** argv){
	int sd=socket(AF_INET,SOCK_STREAM,0);

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
	printf("%x\n",le);
	le=htonl(le);
	printf("%x\n",le);
	while(1){
		char buff[100],server_rep[2000];
		struct message_s msg;
		struct message_s recvmsg;
		memset(buff,0,100);
		memset(msg.protocol,0,5);
		scanf("%s",buff);
		int len;                
		if(strcmp(buff,"list")==0){

			char protocolname[5]="myftp";

			strncpy(msg.protocol,protocolname,5);
			msg.protocol[5]='\0';
			msg.type=LIST_REQUEST;
			msg.length=sizeof(msg);
			sendn(sd,&msg,msg.length);

		}
		if((len=sendn(sd,&msg,sizeof(msg)))<0){
			printf("send error: %s (Errno:%d)\n", strerror(errno),errno);
			exit(0);
		}	
		if((len=recvn(sd,&recvmsg,sizeof(recvmsg))<0))
		{
			printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
			exit(0);

		}	

		if(sizeof(recvmsg)!=0){	
			printf("%d\n",recvmsg.length);

			if(recvmsg.type==LIST_REPLY)
			{
				int listlength=recvmsg.length-sizeof(recvmsg);
				char receivelist[listlength];
				if((len=recvn(sd,&receivelist,sizeof(receivelist))<0))
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					exit(0);

				}	printf("Files in \\data directory:\n\n");
				receivelist[listlength]='\0';printf("%s",receivelist);fflush(stdout);

			}
		}


	}
	return 0;
}
