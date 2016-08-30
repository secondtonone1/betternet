#ifndef  _WRITE_BUFFER_H__
#define _WRITE_BUFFER_H__

#include "socketbuffer.h"
#include "netmodeldef.h"

class SocketWrapper;
class WriteBuffer: public SocketBuffer
{
	friend class SocketWrapper;
 private:  
	 WriteBuffer(sockfd fd);
	~WriteBuffer();
	WriteBuffer(const WriteBuffer & writebuffer);
	WriteBuffer &  operator = (const WriteBuffer & socketBuffer);
	Node * m_pCurSend;
	int sendto( Node * node);

	Node * getSendNode();

	void clearSendNode();

	int pushData(char * buf, int len);
};

#endif
