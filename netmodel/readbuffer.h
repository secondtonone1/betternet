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
	ReadBuffer &  ReadBuffer::operator = (const ReadBuffer & socketBuffer);
	
	int receive(Node * node);
	
	int popData(char * buf, int len);
};



#endif