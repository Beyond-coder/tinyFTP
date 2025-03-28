/* common header.  Tabs are set for 4 spaces, not 8 */

#ifndef _TINYFTP_COMMON_H_
#define _TINYFTP_COMMON_H_

#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>     // sockaddr_in{} and other Internet definitions
#include <arpa/inet.h>      // inet(3) functions
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <termios.h>

#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// c++ header
// #include <cstdio>
// #include <cstdlib>
// #include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <queue>
#include <sstream>
#include <utility>
using namespace std;

// using std::map;
// using std::string;
// using std::iostream;

#define DEBUG 1

#define DBFILENAME		"tinyFTP.db"
#define ROOTDIR			"/home/tinyFTP/"
#define ROOTDIR_LEN		strlen(ROOTDIR)
#define KERNELDIR		"/home/tinyFTP/.tinyFTP/"
#define GHOSTDIR		"/home/tinyFTP/.tinyFTP/ghost/"

#define PASSSALT0		"&5@f#fe)"	// password salt
#define PASSSALT1		"@tinyFTP"	// password salt	

// 单元分隔符
#define DELIMITER		"\x1F" 			// UD(unit separator)

// MACRO constants
#define	LISTENQ		1024		// 2nd argument(backlog) to listen()

// Miscellaneous constants
#define	MAXLINE		256			// max text line length
//#define	BUFFSIZE	8192		// 8K Bytes buffer size for reads and writes

//#define PORT		2121		// server: listening port
#define CTRPORT		2121    	// server: control listening port
#define DATPORT		2020		// server: data pistening port

// Following shortens all the typecasts of pointer arguments
#define	SA	struct sockaddr

class Database;
typedef struct threadArg
{
    int fd;  // 文件描述符，用来存储客户端的连接
    uint32_t sesid;		// Session id
 
    char buf[MAXLINE];
    Database * pdb;
    
} ThreadArg;

typedef enum sockType
{
	SRV_SOCKET,					// server socket type
	CLI_SOCKET					// client socket type
} SockType;

/*********************************************************
 ******************* packet ******************************
 *********************************************************/
// 包存储类型
typedef enum packetStoreType
{
	HPACKET,					// Host storage type
	NPACKET						// Network storage type
} PacketStoreType;


// typedef enum packetType
// {
// 	SRV_PACKET,					// server socket tyoe
// 	CLI_PACKET					// client socket type
// } PacketType;

/************************** CtrPacket **********************************/

#define PHEADSIZE		20			// packet header size
#define	PBODYCAP		512			// packet body capcacity
#define	SLICECAP		512			// slice capcacity
#define	SSLICECAP		"512"		// slice capcacity
#define	MAXNSLICE		4294967295	// max slice count: the maximum positive integer value
// TCP control packet deinition, communication protocol
// 设置对齐为4字节,当前文件中起效，对于包的设计来讲至关重要
 #pragma pack(4)

// 这个结构体作为一个包发送
// 提升为使用protobuf
// 也就是说每个包是固定长度，但是body的长度由bsize决定了
typedef struct packetStruct
{
	/* packet header */
	uint32_t sesid;			// Session id
	uint16_t tagid;				// different packet type: CMD, DATA, INFO 

	uint16_t cmdid;			// Command: ID
	uint16_t statid;		// status code id
	uint16_t dataid;		// data type id

	// 整个数据片数
	uint32_t nslice;		// Data: whole number of file slices
	// 数据片
	uint32_t sindex;		// Data: slice index

	uint16_t bsize;			// the real size of body
	/* packet body */
	char body[PBODYCAP];	// packet body
	
} PacketStruct;

// // tagid: CMD
// #pragma pack(4)
// typedef struct cmdBodyStruct
// {
// 	uint16_t cmdid;			// Command: ID
	
// } CmdBodyStruct;

// // tagid: INFO
// #pragma pack(4)
// typedef struct infoBodyStruct
// {
// 	uint16_t status;			// status code
	
// } InfoBodyStruct;

// // tagid: Data
// #pragma pack(4)
// typedef struct dataBodyStruct
// {
// 	uint16_t nslice;		// Data: whole number of file slices
// 	uint16_t sindex;		// Data: slice index
// 	char slice[SLICECAP];	// packet body
	
// } DataBodyStruct;

#define PACKSIZE sizeof(PacketStruct)

typedef enum tagID
{
	TAG_CMD = 1,
	TAG_STAT,
	TAG_DATA
} TagID;

// different file transfer control commands
typedef enum cmdID
{
	USER = 1,
	PASS,
	USERADD,
	USERDEL,

	GET,
	PUT,
	LS,
	LLS, // local ls
	CD,
	LCD,
	RM,
	LRM,
	PWD,
	LPWD,
	MKDIR,
	LMKDIR,
	QUIT,
	HELP,
	
	MGET,
	MPUT,
	RGET,
	RPUT,
	RMDIR,

	SHELL,
	LSHELL,

	BINARY,
	ASCII
} CmdID;

typedef enum statID
{
	STAT_OK = 1,
	STAT_BPR, 	// breakpoint resume
	STAT_CFM, 	// confirm
	STAT_MD5, 	// md5sum
	STAT_PGS, 	// progress
	STAT_FAIL, 	// fail
	STAT_ERR, 	// error
	STAT_CTN,	// continue
	STAT_TERM,	// terminate
	STAT_SIZE,	// size
	STAT_WAIT,	// wait
	//STAT_DONE,	// transaction done
	STAT_EOF,	// end of file
	STAT_EOT	// end of transfer
} StatID;

typedef enum dataID
{
	DATA_FILE = 1,
	DATA_TEXT,
	DATA_LIST,
	DATA_NAME,
	DATA_OTHER
} DataID;

/*********************************************************
 ******************* functions ***************************
 *********************************************************/

void Fclose(FILE *fp);
void Fclose(FILE **fp);
FILE * Fdopen(int fd, const char *type);
char * Fgets(char *ptr, int n, FILE *stream);
FILE * Fopen(const char *filename, const char *mode);
void Fputs(const char *ptr, FILE *stream);
void * Malloc(size_t size);



void	Pthread_create(pthread_t *, const pthread_attr_t *,
					   void * (*)(void *), void *);
void	Pthread_join(pthread_t, void **);
void	Pthread_detach(pthread_t);
void	Pthread_kill(pthread_t, int);

void	Pthread_mutexattr_init(pthread_mutexattr_t *);
void	Pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
void	Pthread_mutex_init(pthread_mutex_t *, pthread_mutexattr_t *);
void	Pthread_mutex_lock(pthread_mutex_t *);
void	Pthread_mutex_unlock(pthread_mutex_t *);

void	Pthread_cond_broadcast(pthread_cond_t *);
void	Pthread_cond_signal(pthread_cond_t *);
void	Pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
void	Pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *,
							   const struct timespec *);

void	Pthread_key_create(pthread_key_t *, void (*)(void *));
void	Pthread_setspecific(pthread_key_t, const void *);
void	Pthread_once(pthread_once_t *, void (*)(void));

string size2str(unsigned long filesize);
int getFileNslice(const char *pathname, uint32_t *pnslice_o);
string getFileSizeString(const char *pathname);
string visualmd5sum(const char * pathname);
string md5sum(const char * pathname);
string visualmd5sumNslice(const char * pathname, uint32_t nslice);
string md5sumNslice(const char * pathname, uint32_t nslice);
string md5sum(const char * str, int len);
unsigned long long getFilesize(const char * pathname);
string getFilesize(string pathname);
string encryptPassword(string password);
string getCurrentTime();
unsigned long long getDiskAvailable();

void restore_terminal_settings(void);
void disable_terminal_return(void);

string getInode(const char * pathname);

#endif	/* __TINYFTP_COMMON_H__ */
