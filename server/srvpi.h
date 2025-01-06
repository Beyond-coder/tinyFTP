#ifndef _TINYFTP_SRVPI_H_
#define _TINYFTP_SRVPI_H_

#include    "../common/common.h"
#include    "../common/error.h"
#include    "../common/packet.h"
#include    "../common/socket.h"
#include    "../common/sockstream.h"
#include    "../common/database.h"
#include    "../common/pi.h"
#include    "srvdtp.h"

// Server Protocol Interpreter (SrvPI)
class SrvPI : public PI
{
public:
	SrvPI(string dbFilename, int connfd);
	bool checkBreakpoint();
	bool checkGETBreakpoint();
	bool recvOnePacket();
	bool sendOnePacketBlocked(PacketStruct * ps, size_t nbytes);
	bool sendOnePacket(PacketStruct * ps, size_t nbytes);
	void run();
	void split(std::string src, std::string token, vector<string>& vect);  

	void cmdUSER();
	void cmdPASS();
	void cmdUSERADD();
	void cmdUSERDEL();

	void cmdGET();
	//void cmdGET(string pathname);
	void RGET_recurse(string srvpath, string clipath);
	void RGET_iterate(string srvpath, string clipath);
	void cmdRGET();
	void cmdPUT();
	bool sizecheck(string & sizestr);
	bool md5check(string & md5str, string newpath);
	void cmdPUT(string clipath, string srvpath);
	void cmdRPUT();
	bool cmdLMKDIR(string path);
	void cmdLS();
	void cmdCD();
	void cmdRM();
	void cmdPWD();
	void cmdMKDIR();
	void cmdRMDIR();
	void cmdSHELL();

	
	int getConnfd();
	FILE* setFp(FILE *fp);
	FILE* & getFp();
	Database * getPDB();
	string getClipath();
	string getFilename();
	unsigned long long getFilesize();
	~SrvPI();
	
	


private:

	// 统计包数
	int sessionCommandPacketCount;
	// 包
	Packet packet;
	// 读到的包
	Packet readpacket;
	// 连接
	int connfd;
	SockStream connSockStream;
	//SrvDTP srvDTP;
	Database db;

	string userID; // for simple, userID is equal to session ID
	std::string userRootDir;
	std::string userRCWD; // current working directory relative to userRootDir

	string abspath;
	string filename;
	string filesize;

	string clipath;
	
	FILE* fp;
	// -1: error, -2: CFM, 0: ok
	int combineAndValidatePath(uint16_t cmdid, string userinput, string & msg_o, string & abspath_o);
	int cmdPathProcess(uint16_t cmdid, string newAbsDir, string & msg_o);
	void saveUserState();

	// 深度优先遍历删除文件夹
	void rmdirDFS();
	// 删除文件夹
	void removeDir(const char *path_raw, bool removeSelf);

};

#endif /* _TINYFTP_SRVPI_H_ */