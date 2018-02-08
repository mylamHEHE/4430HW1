/*
Group 40
LAM Ming Yuen 1155083016
LEE Ho Yin 1155085665
*/
# include <stdio.h>
# include <stdlib.h>
#include <string.h>
# include "myftp.h"
//# define PORT 12346

int main(int argc, char** argv){
	int sd=socket(AF_INET,SOCK_STREAM,0);
	printf("client_main\n");
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
		printf("client_while: %s\n", argv[3]);
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
		else if(strcmp(buff,"put")==0)
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

				printf("%x\n", msg.type);
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
				printf("Sended, thx\n");
			}
			else{
				printf("The file doesn't exist.\n");break;
			}
		}
		else if(strcmp(argv[3],"get")==0)
		{
			char protocolname[5]="myftp";
			char filename[strlen(argv[4])];
			strcpy(filename,argv[4]);
			strncpy(msg.protocol,protocolname,5);
			msg.protocol[5]='\0';
			msg.type=GET_REQUEST;
			msg.length=sizeof(msg)+strlen(filename);

			if(len=sendn(sd,&msg,sizeof(msg))<0){
				printf("send error: %s (Errno:%d)\n", strerror(errno),errno);
				exit(0);
			}

			if(len=sendn(sd,&filename,sizeof(filename))<0){

				exit(0);
			}



		}


		if((len=recvn(sd,&recvmsg,sizeof(struct message_s))<0))
		{
			printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
			exit(0);

		}	
		if(sizeof(recvmsg)!=0){	
			printf("Something Received\n");
			if(recvmsg.type==LIST_REPLY)
			{
				int listlength=recvmsg.length-sizeof(recvmsg);
				char receivelist[listlength+1];
				if((len=recvn(sd,&receivelist,sizeof(receivelist))<0))
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					exit(0);
				}	
				printf("Files in \\data directory:\n\n");
				printf("%s",receivelist);
				fflush(stdout);
			}
			if(recvmsg.type==PUT_REPLY)
			{
				printf("Reply received\n");

				char protocolname[5]="myftp";
				strncpy(msg.protocol,protocolname,5);
				FILE* file = fopen(argv[4], "r");
				fseek(file, 0L, SEEK_END);
				int fileSize = ftell(file);
				fseek(file, 0L, SEEK_SET);

				struct message_s sendmsg;

				sendmsg.protocol[5]='\0';
				sendmsg.type=FILE_DATA;
				sendmsg.length=sizeof(sendmsg)+fileSize;

				printf("file Size:%d\n ", fileSize);
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
				printf("\n");
				unsigned char code;
				for(int i=0; i<fileSize; i++)
				{
					fread(&code, 1, 1, file);
					if(code == EOF) break;
					//check contect printf("%c", *code);
					sendn(sd, &code, 1);
				}

				//just added end
				printf("Success\n");
				fflush(stdout);
			}
			if(recvmsg.type==GET_REPLY_EXIST_FILE)
			{
			//FILE_DATA
				if(len=recvn(sd,&recvmsg,sizeof(recvmsg))<0)
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					exit(0);

				}
				printf("CHUNK:%d",recvmsg.length-sizeof(recvmsg));fflush(stdout);
				if(sizeof(recvmsg)!=0){
			//payload file_data
					FILE* fp;
					fp = fopen(argv[4], "wb+");
					if (fp == NULL) {
						exit(0);
					}
					char downloaddata;
					int chunk=0;
					int p=recvmsg.length-sizeof(recvmsg);
					printf("XCHUNK:%d\n",p);
					printf("Downloading... File size is %d byte.",p);
					while(chunk<(recvmsg.length-sizeof(recvmsg)))
					{

						if(len=recvn(sd,&downloaddata,sizeof(downloaddata))<0)
						{
							printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
							exit(0);

						}

						chunk+=1;

						fprintf( fp,  "%c" , downloaddata  );
						fflush(stdout);

					}	printf("Download completed.\n");
			//downloadedaction(argv[2],downloaddata);
					fclose(fp);


				}

			}
			if(recvmsg.type==GET_REPLY_NOT_EXIST)
			{
				printf("%s\n","File not exists.");
				exit(0);
			}
		}
	}
	while(count!=0);
	// while(1);
	return 0;
}
