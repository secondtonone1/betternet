#ifndef  _READ_BUFFER_H__
#define _READ_BUFFER_H__

#include "socketbuffer.h"
#include "netmodeldef.h"

class SocketWrapper;
class ReadBuffer: public SocketBuffer
{
	friend class SocketWrapper;
 private:  
	 ReadBuffer(sockfd fd);
	~ReadBuffer();
	ReadBuffer(const ReadBuffer & readBuffer);
	ReadBuffer &  operator = (const ReadBuffer & socketBuffer);
	
	//接受函数，将新来的信息放入node里，每个node最大长度固定
	//长度不够开辟新的node继续接受信息
	//在接受功能的node里，moff该node已经接收的数据大小
	int receive(Node * node);
	
	int popData(char * buf, int len);

	int m_nTotal;
};



#endif
