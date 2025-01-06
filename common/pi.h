#ifndef _TINYFTP_PI_H_
#define _TINYFTP_PI_H_
#include    "common.h"


// 抽象类，用来定义接口
// 协议解释器
class PI
{
public:
	// 收到一个包
	virtual bool recvOnePacket() = 0;
	// 发送一个包
	virtual bool sendOnePacket(PacketStruct * ps, size_t nbytes) = 0;
	// 阻塞发送一个包
	virtual bool sendOnePacketBlocked(PacketStruct * ps, size_t nbytes) = 0;
	// virtual bool recvOnePacket(){ printf("virtual bool recvOnePacket()\n"); return true; }
	// virtual bool sendOnePacket(PacketStruct * ps, size_t nbytes){ printf("virtual bool sendOnePacket(Packet * ps, size_t nbytes)\n"); return true; }
};

#endif /* _TINYFTP_PI_H_ */