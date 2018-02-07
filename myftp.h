# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#include <stddef.h>
#define NONE 0x00
#define LIST_REQUEST 0xA1
#define LIST_REPLY 0xA2
#define GET_REQUEST 0xB1
#define GET_REPLY 0xB2
#define PUT_REQUEST 0xC1
#define PUT_REPLY 0xC2
#define FILE_DATA 0xFF
#define MAX_PAC_SIZE 10000
struct message_s 
{
	unsigned char protocol[5];
	unsigned char type;
	unsigned int length;
} __attribute__ ((packed));

extern int sendn(int sd,void *buf,int buf_len);
extern int recvn(int sd,void *buf,int buf_len);