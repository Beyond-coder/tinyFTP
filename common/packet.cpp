#include 	"packet.h"

Packet::Packet(PI * ppi)
{ 
	this->pstype = HPACKET;
	ps = (PacketStruct*) Malloc(PACKSIZE);
	prePs = (PacketStruct*) Malloc(PACKSIZE);
	ps->sesid = 0; 
	this->ppi =  ppi;
}

// Packet::Packet(PacketStoreType pstype)
// { 
// 	init(pstype); 
// }
// void Packet::init(PacketStoreType pstype)
// { 
// 	this->pstype = pstype;
// 	ps = (PacketStruct*) Malloc(PACKSIZE); 
// }
void Packet::fill(uint16_t tagid, uint16_t cmdid, uint16_t statid, uint16_t dataid, uint32_t nslice, uint32_t sindex, uint16_t bsize, const char * body)
{ 
	ps->tagid = tagid;

	ps->cmdid = cmdid;
	ps->statid = statid;
	ps->dataid = dataid;

	ps->nslice = nslice;
	ps->sindex = sindex;
	ps->bsize = bsize;

	if (bsize > PBODYCAP)
	{
		Error::msg("\033[31mPacket::fill error: bsize=%d, bsize > PBODYCAP\033[0m", bsize);
		return;
	}
	if(body != NULL && bsize != 0){
		memcpy(ps->body, body, bsize);
	} 
}

void Packet::fillCmd(uint16_t cmdid, uint16_t bsize, const char * body)
{ 
	fill(TAG_CMD, cmdid, 0, 0, 0, 0, bsize, body);  
}

void Packet::fillStat(uint16_t statid, uint16_t bsize, const char * body)
{ 
	fill(TAG_STAT, 0, statid, 0, 0, 0, bsize, body); 
}

void Packet::fillData(uint16_t dataid, uint32_t nslice, uint32_t sindex, uint16_t bsize, const char * body)
{ 
	fill(TAG_DATA, 0, 0, dataid, nslice, sindex, bsize, body); 
}

void Packet::setSessionID(uint32_t sesid)
{ 
	ps->sesid = sesid; 
	//printf("setSessionID: %u\n", ps->sesid);
}

void Packet::reset(PacketStoreType pstype)
{
	//must keep sesid
	if (this->pstype == NPACKET){
		if (pstype == HPACKET){
			ps->sesid = ntohl(ps->sesid);
		}
	} else if (this->pstype == HPACKET){
		this->savePacketState();
		if (pstype == NPACKET){
			ps->sesid = htonl(ps->sesid);
		}
	}
	this->pstype = pstype;

	ps->tagid = 0;

	ps->cmdid = 0;
	ps->statid = 0;
	ps->dataid = 0;

	ps->nslice = 0;
	ps->sindex = 0;
	ps->bsize = 0;

	memset(ps->body, 0, PBODYCAP);
}
void Packet::savePacketState()
{
	prePs->sesid = ps->sesid;

	prePs->tagid = ps->tagid;

	prePs->cmdid = ps->cmdid;
	prePs->statid = ps->statid;
	prePs->dataid = ps->dataid;

	prePs->nslice = ps->nslice;
	prePs->sindex = ps->sindex;
	prePs->bsize = ps->bsize;
}

void Packet::zero()
{
	memset(ps, 0, PACKSIZE);
}

void Packet::ntohp()
{
	if (pstype == HPACKET){
		Error::msg("already in HOST byte order\n");
		return;
	}
	// 每一部分都做相应转换即可
	ps->sesid = ntohl(ps->sesid);
	ps->tagid = ntohs(ps->tagid);
	
	ps->cmdid = ntohs(ps->cmdid);
	ps->statid = ntohs(ps->statid);
	ps->dataid = ntohs(ps->dataid);

	ps->nslice = ntohl(ps->nslice);
	ps->sindex = ntohl(ps->sindex);
	ps->bsize = ntohs(ps->bsize);

	this->pstype = HPACKET;
	
}


void Packet::htonp()
{
	if (pstype == NPACKET){
		Error::msg("already in NETWORK byte order\n");
		return;
	}

	ps->sesid = htonl(ps->sesid);
	ps->tagid = htons(ps->tagid);
	
	ps->cmdid = htons(ps->cmdid);
	ps->statid = htons(ps->statid);
	ps->dataid = htons(ps->dataid);

	ps->nslice = htonl(ps->nslice);
	ps->sindex = htonl(ps->sindex);
	ps->bsize = htons(ps->bsize);

	this->pstype = NPACKET;
	
}

void Packet::print()
{
	if (!DEBUG)
		return;
	
	if (pstype == HPACKET)
	{
		printf("\t\t[HOST Packet: %p]\n", ps);
		
	}
	else if (pstype == NPACKET)
	{
		printf("\t\t[NETWORK Packet: %p]\n", ps);
	}
	else {
		Error::msg("unknown PacketStoreType\n");
		return;
	}

	printf("\t\tsesid = %u\n", ps->sesid);
	printf("\t\ttagid = %d\n", ps->tagid);
	printf("\t\tcmdid = %d\n", ps->cmdid);
	printf("\t\tstatid = %d\n", ps->statid);
	printf("\t\tdataid = %d\n", ps->dataid);
	printf("\t\tnslice = %u\n", ps->nslice);
	printf("\t\tsindex = %u\n", ps->sindex);
	printf("\t\tbsize = %d\n", ps->bsize);
	if (pstype == HPACKET)
	{
		printf("\t\tbody = %s\n",  this->getSBody().c_str());
	}
	
	
	fflush(stdout);
}


void Packet::pprint()
{
	printf("\t\t[Previous HOST Packet: %p]\n", prePs);
	printf("\t\tsesid = %u\n", prePs->sesid);
	printf("\t\ttagid = %d\n", prePs->tagid);
	printf("\t\tcmdid = %d\n", prePs->cmdid);
	printf("\t\tstatid = %d\n", prePs->statid);
	printf("\t\tdataid = %d\n", prePs->dataid);
	printf("\t\tnslice = %u\n", prePs->nslice);
	printf("\t\tsindex = %u\n", prePs->sindex);
	printf("\t\tbsize = %d\n", prePs->bsize);
	
	fflush(stdout);
}

void Packet::sendCMD(uint16_t cmdid, string sbody)
{
	// send OK
	// 指定为HOSTpacket
	this->reset(HPACKET);
	this->fillCmd(cmdid, sbody.size(), sbody.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendCMD_GET(const char *body)
{
	// send OK
	this->reset(HPACKET);
	this->fillCmd(GET, strlen(body), body);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendCMD_GET(string sbody)
{
	// send OK
	this->reset(HPACKET);
	this->fillCmd(GET, sbody.size(), sbody.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendCMD_LMKDIR(const char *body)
{
	// send OK
	this->reset(HPACKET);
	this->fillCmd(LMKDIR, strlen(body), body);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendCMD_LMKDIR(string sbody)
{
	// send OK
	this->reset(HPACKET);
	this->fillCmd(LMKDIR, sbody.size(), sbody.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

// void Packet::sendDATA_FILE(uint16_t dataid, uint32_t nslice, uint32_t sindex, uint16_t bsize, char * body)
// {
// 	//this->print();
// 	this->reset(HPACKET);
// 	this->fillData(DATA_FILE, nslice, sindex, bsize, body);
// 	//printf("sendDATA:\n");
// 	//this->print();
// 	this->htonp();
// 	ppi->sendOnePacket(this->ps, PACKSIZE);
// }

void Packet::sendDATA_FILE(uint32_t nslice, uint32_t sindex, uint16_t bsize, const char *body)
{
	this->reset(HPACKET);
	this->fillData(DATA_FILE, nslice, sindex, bsize, body);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

// void Packet::sendDATA_NAME(uint32_t nslice, uint32_t sindex, uint16_t bsize, char *body)
// {
// 	//this->print();
// 	this->reset(HPACKET);
// 	this->fillData(DATA_NAME, nslice, sindex, bsize, body);
// 	//printf("sendDATA:\n");
// 	//this->print();
// 	this->htonp();
// 	ppi->sendOnePacket(this->ps, PACKSIZE);
// }
void Packet::sendDATA_LIST(uint32_t nslice, uint32_t sindex, uint16_t bsize, const char *body)
{
	this->reset(HPACKET);
	this->fillData(DATA_LIST, nslice, sindex, bsize, body);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendDATA_LIST(uint32_t nslice, uint32_t sindex, uint16_t bsize, string body)
{
	this->reset(HPACKET);
	this->fillData(DATA_LIST, nslice, sindex, body.size(), body.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}
void Packet::sendDATA_NAME(uint32_t nslice, uint32_t sindex, uint16_t bsize, const char *body)
{
	this->reset(HPACKET);
	this->fillData(DATA_NAME, nslice, sindex, bsize, body);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendDATA_NAME(uint32_t nslice, uint32_t sindex, uint16_t bsize, string body)
{
	this->reset(HPACKET);
	this->fillData(DATA_NAME, nslice, sindex, body.size(), body.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}
void Packet::sendDATA_TEXT(const char *body)
{
	// send OK
	this->reset(HPACKET);
	this->fillData(DATA_TEXT, 0, 0, strlen(body), body);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendDATA_TEXT(uint16_t bsize, const char *body)
{
	// send OK
	this->reset(HPACKET);
	this->fillData(DATA_TEXT, 0, 0, bsize, body);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendDATA_TEXT(string body)
{
	// send OK
	this->reset(HPACKET);
	this->fillData(DATA_TEXT, 0, 0, body.size(), body.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT(uint16_t statid ,string body)
{
	this->reset(HPACKET);
	this->fillStat(statid, body.size(), body.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_OK()
{
	// send OK
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[32mOK to transfer\033[0m");
	this->fillStat(STAT_OK, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}
void Packet::sendSTAT_OK(const char *msg)
{
	// send OK
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[32m%s\033[0m", msg);
	this->fillStat(STAT_OK, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_OK(string msg)
{
	// send OK
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[32m%s\033[0m", msg.c_str());
	this->fillStat(STAT_OK, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_BPR(string body)
{
	this->reset(HPACKET);
	this->fillStat(STAT_BPR, body.size(), body.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}


void Packet::sendSTAT_MD5(string body)
{
	this->reset(HPACKET);
	this->fillStat(STAT_MD5, body.size(), body.c_str());
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}
void Packet::sendSTAT_PGS(string body)
{
	this->reset(HPACKET);
	this->fillStat(STAT_PGS, body.size(), body.c_str());
	this->htonp();
	ppi->sendOnePacketBlocked(this->ps, PACKSIZE);
}


void Packet::sendSTAT_FAIL(string body)
{
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[31m%s\033[0m", body.c_str());
	this->fillStat(STAT_FAIL, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_CFM(const char *msg)
{
	// send CFM
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "%s", msg);
	this->fillStat(STAT_CFM, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_CFM(string msg)
{
	// send CFM
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "%s", msg.c_str());
	this->fillStat(STAT_CFM, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_ERR()
{
	// send ERR
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[31mError occurred\033[0m");
	this->fillStat(STAT_ERR, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_ERR(const char *msg)
{
	// send ERR
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[31m%s\033[0m", msg);
	this->fillStat(STAT_ERR, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}
void Packet::sendSTAT_ERR(string msg)
{
	// send ERR
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[31m%s\033[0m", msg.c_str());
	this->fillStat(STAT_ERR, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_EOF()
{
	// send EOT
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[32mEnd of File\033[0m");
	this->fillStat(STAT_EOF, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_EOF(string msg)
{
	// send ERR
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[31m%s\033[0m", msg.c_str());
	this->fillStat(STAT_EOF, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_EOT()
{
	// send EOT
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[32mEnd of Tansfer\033[0m");
	this->fillStat(STAT_EOT, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

void Packet::sendSTAT_EOT(string msg)
{
	// send ERR
	this->reset(HPACKET);
	char buf[MAXLINE];
	snprintf(buf, MAXLINE, "\033[32m%s\033[0m", msg.c_str());
	this->fillStat(STAT_EOT, strlen(buf), buf);
	this->htonp();
	ppi->sendOnePacket(this->ps, PACKSIZE);
}

PacketStruct * Packet::getPs()
{ 
	return ps;
}
uint32_t Packet::getSesid()
{ 
	return ps->sesid;
}

uint16_t Packet::getTagid()
{ 
	return ps->tagid;
}
uint16_t Packet::getCmdid()
{ 
	return ps->cmdid;
}

uint16_t Packet::getStatid()
{ 
	return ps->statid;
}

uint16_t Packet::getDataid()
{ 
	return ps->dataid;
}

uint32_t Packet::getNslice()
{ 
	return ps->nslice;
}

uint32_t Packet::getSindex()
{ 
	return ps->sindex;
}

uint16_t Packet::getBsize()
{ 
	return ps->bsize;
}
char * Packet::getBody()
{ 
	return ps->body;
}
std::string Packet::getSBody()
{ 
	char buf[PBODYCAP + 1] = {0};
	strncpy(buf, ps->body, ps->bsize);
	return string(buf);
}




PacketStruct * Packet::getPrePs()
{ 
	return prePs;
}
uint32_t Packet::getPreSesid()
{ 
	return prePs->sesid;
}

uint16_t Packet::getPreTagid()
{ 
	return prePs->tagid;
}
uint16_t Packet::getPreCmdid()
{ 
	return prePs->cmdid;
}

uint16_t Packet::getPreStatid()
{ 
	return prePs->statid;
}

uint16_t Packet::getPreDataid()
{ 
	return prePs->dataid;
}

uint32_t Packet::getPreNslice()
{ 
	return prePs->nslice;
}

string Packet::getPreSNslice()
{ 
	char buf[MAXLINE] = {0};
	snprintf(buf, MAXLINE, "%u", prePs->nslice);
	return string(buf);
}

uint32_t Packet::getPreSindex()
{ 
	return prePs->sindex;
}

string Packet::getPreSSindex()
{ 
	char buf[MAXLINE] = {0};
	snprintf(buf, MAXLINE, "%u", prePs->sindex);
	return string(buf);
}

uint16_t Packet::getPreBsize()
{ 
	return prePs->bsize;
}

Packet::~Packet()
{ 
	free(ps);
	free(prePs);  
}