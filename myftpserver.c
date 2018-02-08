/*
Group 40
LAM Ming Yuen 1155083016
LEE Ho Yin 1155085665
*/
#include <stdio.h>
#include <stdlib.h>
#include "myftp.h"
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
//# define PORT 12346
unsigned int inet_addrx(char *str)
{
	int a, b, c, d;
	char arr[4];
	sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);
	arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
	return *(unsigned int *)arr;
}

void sigHandler(int n) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
struct message_s makepack(int type,int length){
	struct message_s msg;
	int payload = length;

	int packet_size;
	char protocolname[5]="myftp";

	switch(type){
		case LIST_REPLY:

		strncpy(msg.protocol,protocolname,5);
		msg.protocol[5]='\0';
		msg.type=LIST_REPLY;

		break;
		case GET_REPLY_EXIST_FILE:
		strncpy(msg.protocol,protocolname,5);
		msg.protocol[5]='\0';
		msg.type=GET_REPLY_EXIST_FILE;
		break;
		case GET_REPLY_NOT_EXIST:
		strncpy(msg.protocol,protocolname,5);
		msg.protocol[5]='\0';
		msg.type=GET_REPLY_NOT_EXIST;
		break;
		case FILE_DATA:
		strncpy(msg.protocol,protocolname,5);
		msg.protocol[4]='\0';
		msg.type=FILE_DATA;
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
	memset(returningstring,0,len);

	while(! readdir_r(pDir, pStEntry, &pStResult) && pStResult != NULL)
	{
		strcat(returningstring,pStResult->d_name);
		strcat(returningstring,"\n");
	}

	free(pStEntry);
	closedir(pDir);
	return returningstring;
}
int fileexist(char *name){
	char dir[strlen(name)+7];
	strcpy(dir,"./data/");
	strcat(dir,name);
	if( access( dir, F_OK ) != -1 ) {
		struct stat buffer;
		if(stat(dir,&buffer)==0){
			if(S_ISDIR(buffer.st_mode)==1)return -1;
			return (int)buffer.st_size;
		}

	} else {

		return -1;
	}
}
void downloadreader(char *fname,int size,int client_sd)
{
	int len;
	char dir[strlen(fname)+7];
	int sizecount=0;
	long remainingBytes;
	int buf;
	size_t buflen;
	FILE *file;
	strcpy(dir,"./data/");
	strcat(dir,fname);
	file = fopen(dir, "rb");


	if (file) {
//?


		while (1) {
			buflen = fread(&buf,1, sizeof(buf), file);
			if (buflen < 1) {

				if (!feof(file)) {
					exit(0);
				}
				break;
			}
			if(buflen<1){
				buf='\0';
			}
//*send
			if(len=sendn(client_sd,&buf,sizeof(buf))<0)
			{
				printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
				exit(0);

			}

		}
		fclose(file);

	}

	return;
}

void *ChildTask(void *client_sd)
{
	int len;
	struct message_s revmsg;

	int finished = 0;
	do{

		if((len=recvn((int)client_sd,&revmsg,sizeof(revmsg)))<0){

			pthread_exit((void *)1);
		}
		char* filename;
		if(sizeof(revmsg)!=0){
			if(revmsg.type==LIST_REQUEST){
			//not fixed
				printf("LIST command received\n");
				printf("--------------\n");
				char* ls = list();
				char sendliststring[strlen(ls)+1];

				strcpy(sendliststring,ls);
				struct message_s LIST__REPLY=makepack(LIST_REPLY,strlen(sendliststring));

				if(len=sendn((int)client_sd,&LIST__REPLY,sizeof(LIST__REPLY))<0)
				{

					pthread_exit((void *)1);

				}
				if(len=sendn((int)client_sd,sendliststring,sizeof(sendliststring))<0)
				{


					pthread_exit((void *)1);
				}
				finished = 1;
			}
			else if(revmsg.type==GET_REQUEST){
				char filenamequery[revmsg.length-sizeof(revmsg)];

				if(len=recvn((int)client_sd,&filenamequery,sizeof(filenamequery))<0){

					pthread_exit((void *)1);
				}

				filenamequery[revmsg.length-sizeof(revmsg)]='\0';
				int checkexist= fileexist(filenamequery);
				printf("GET command,target directory:%s",filenamequery);
				if(checkexist!=-1){
					struct message_s GET__REPLY=makepack(GET_REPLY_EXIST_FILE,0);
					if(len=sendn((int)client_sd,&GET__REPLY,sizeof(GET__REPLY))<0)
					{

						pthread_exit((void *)1);

					}


					struct message_s FILE__DATA=makepack(FILE_DATA,checkexist);
					if(len=sendn((int)client_sd,&FILE__DATA,sizeof(FILE__DATA))<0)
					{

						pthread_exit((void *)1);
					}
					printf("(Exist)\nSize:%d\n",checkexist);
					downloadreader(filenamequery,checkexist,(int)client_sd);

				}
				else{
					struct message_s GET__REPLY=makepack(GET_REPLY_NOT_EXIST,0);
					if(len=sendn((int)client_sd,&GET__REPLY,sizeof(GET__REPLY))<0)
					{
						pthread_exit((void *)1);

					}
					printf("(Not Exist)\n");
				}
				printf("--------------\n");
				finished = 1;
			}
			else if(revmsg.type==PUT_REQUEST)
			{	
				struct message_s sendmsg;
				printf("received with thanks\n");
				int listlength=revmsg.length-sizeof(struct message_s);
				char receivelist[listlength+1];
				if((len=recvn((int)client_sd,&receivelist,listlength)<0))
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);

					pthread_exit((void *)1);
				}
				printf("Filename is :%s", receivelist);
				char* dir = "data/";
				filename = malloc(strlen(dir)+strlen(receivelist)+1);
				strcpy(filename, dir);
				strcat(filename, receivelist);
				char protocolname[6]="myftp";

				strcpy(sendmsg.protocol,protocolname);
				sendmsg.type=PUT_REPLY;
				sendmsg.length=sizeof(struct message_s);
				if(len=sendn((int)client_sd,&sendmsg,sizeof(sendmsg))<0)
				{
					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
					fflush(stdout);

					pthread_exit((void *)1);
				}
				printf("Server send success! Yeah!\n");
			}else if(revmsg.type==FILE_DATA)
			{	
				printf("FILE RECEIVED\n");
				int filelength=revmsg.length-sizeof(struct message_s);
				FILE* receiveFile = fopen(filename, "wb");
				for(int i=0; i<filelength; i++){
					char a='\0';
					if((len=recvn((int)client_sd,&a,1)<0))
					{
						printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);

						pthread_exit((void *)1);
					}
					fwrite(&a, 1, 1, receiveFile);
				}
				// printf("\n");

				printf("Saved to %s...", filename);
				fclose(receiveFile);
				finished = 1;
			}
		}
		revmsg.type = NONE;
	}while(finished == 0);
	pthread_t thId = pthread_self();
	printf("\nThread %u transfer finished\n",(int) thId);

	pthread_exit((void *)1);

}



int main(int argc, char** argv){
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

	signal(SIGCHLD, sigHandler);
	int pid;
	while(1){
		int addr_len=sizeof(client_addr);
		if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0){
			printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
			exit(0);
		}
		char str[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &client_addr, str, INET_ADDRSTRLEN );
		printf("%s",str);
		int num_thread=1;
		pthread_t thread_id;
		void *res;
		if( pthread_create( &thread_id , NULL ,  ChildTask , (void*) client_sd) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		printf("--------------\n");
		printf("From thread %u\n",(int) thread_id);


		fflush(stdout);

	}

	return 0;
}



// int main(int argc, char* argv[])
// {
// 	int sd=socket(AF_INET,SOCK_STREAM,0);
// 	long val=1;
// 	if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(long))==-1)
// 	{
// 		perror("setsockopt");exit(1);
// 	}
// 	int client_sd;
// 	struct sockaddr_in server_addr;
// 	struct sockaddr_in client_addr;
// 	memset(&server_addr,0,sizeof(server_addr));
// 	server_addr.sin_family=AF_INET;
// 	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
// 	server_addr.sin_port=htons(atoi(argv[1]));
// 	if(bind(sd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0)
// 	{
// 		printf("bind error: %s (Errno:%d)\n",strerror(errno),errno);
// 		exit(0);
// 	}
// 	if(listen(sd,3)<0)
// 	{
// 		printf("listen error: %s (Errno:%d)\n",strerror(errno),errno);
// 		exit(0);
// 	}
// 	int addr_len=sizeof(client_addr);
// 	if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0)
// 	{
// 		printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
// 		exit(0);
// 	}

// 	char* filename;

// 	while(1){
// 		char buff[100];
// 		int len;
// 		struct message_s revmsg;
// 		struct message_s sendmsg;

// 		if((len=recvn(client_sd,&revmsg,sizeof(revmsg)))<0)
// 		{
// 			printf("receive error1: %s (Errno:%d)\n", strerror(errno),errno);
// 			exit(0);
// 		}else{
// 			if(revmsg.type==LIST_REQUEST)
// 			{
// 				char sendliststring[strlen(list())];

// 				strcpy(sendliststring,list());
// 				sendmsg=makepack(LIST_REQUEST,sendliststring);

// 				if(len=sendn(client_sd,&sendmsg,sizeof(sendmsg))<0)
// 				{
// 					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
// 					fflush(stdout);
// 					exit(0);

// 				}
// 				if(len=sendn(client_sd,sendliststring,sizeof(sendliststring))<0)
// 				{
// 					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
// 					fflush(stdout);
// 					exit(0);
// 				}

// 				if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0)
// 				{
// 					printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
// 					exit(0);
// 				}
// 			}
// 			if(revmsg.type==PUT_REQUEST)
// 			{	
// 				printf("received with thanks\n");
// 				int listlength=revmsg.length-sizeof(struct message_s);
// 				char receivelist[listlength+1];
// 				if((len=recvn(client_sd,&receivelist,listlength)<0))
// 				{
// 					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
// 					exit(0);
// 				}
// 				printf("Filename is :%s", receivelist);
// 				char* dir = "data/";
// 				filename = malloc(strlen(dir)+strlen(receivelist)+1);
// 				strcpy(filename, dir);
// 				strcat(filename, receivelist);
// 				char protocolname[6]="myftp";
// 				strcpy(sendmsg.protocol,protocolname);
// 				sendmsg.type=PUT_REPLY;
// 				sendmsg.length=sizeof(struct message_s);
// 				sendmsg=makepack(PUT_REQUEST, "\0");
// 				if(len=sendn(client_sd,&sendmsg,sizeof(sendmsg))<0)
// 				{
// 					printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
// 					fflush(stdout);
// 					exit(0);
// 				}
// 				printf("Server send success! Yeah!\n");
// 			}
// 			if(revmsg.type==FILE_DATA)
// 			{	
// 				printf("FILE RECEIVED\n");
// 				int filelength=revmsg.length-sizeof(struct message_s);
// 				FILE* receiveFile = fopen(filename, "wb");
// 				// if((len=recvn(client_sd,receiveFile,filelength)<0))
// 				// {
// 				// 	printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
// 				// 	exit(0);
// 				// }
// 				for(int i=0; i<filelength; i++){
// 					char a='\0';
// 					if((len=recvn(client_sd,&a,1)<0))
// 					{
// 						printf("receive error: %s (Errno:%d)\n", strerror(errno),errno);
// 						exit(0);
// 					}
// 					fwrite(&a, 1, 1, receiveFile);
// 				}
// 				// printf("\n");

// 				printf("Saved to %s...", filename);
// 				fclose(receiveFile);
// 				if((client_sd=accept(sd,(struct sockaddr *) &client_addr,&addr_len))<0)
// 				{
// 					printf("accept erro: %s (Errno:%d)\n",strerror(errno),errno);
// 					exit(0);
// 				}
// 			}
// 		}
// 		revmsg.type = NONE;
// 	}
// 	close(sd);
// 	return 0;
// }
