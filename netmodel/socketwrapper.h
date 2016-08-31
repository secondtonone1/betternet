# ifndef __SOCKET_WRAPPER_H_
#define __SOCKET_WRAPPER_H_

#include "readbuffer.h"
#include "writebuffer.h"

class ModelManager;
class SocketWrapper;

typedef void  (*read_cb) (ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx);
typedef void  (*write_cb)(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx);
typedef void  (*error_cb)(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx);


struct SocketIndex
{
	int read_pos_plus1;
	int write_pos_plus1;
};

class SocketWrapper
{
public:
	
	SocketWrapper(sockfd fd, ModelManager * modelManager, bool isListen);
	~SocketWrapper();
	void setRead(bool enable = false);
	void setWrite(bool enable = false);

	bool isSetRead(void);
	bool isSetWrite(void);

	int ToRead();
	int ToWrite();

	int writeToBuffer(char * msg, int len);

	int readFromBuffer(char * msg, int len);

	void registercb(read_cb  readcallback,  write_cb writecallback, error_cb errorcallback);
	
	SocketIndex * getSocketIndex();

	int getTotalRead();

private:
	SocketWrapper():m_bWrite(false), m_bRead(false),m_nSocketFd(0),m_bufferRead(0),m_bufferWrite(0),m_pModelManager(NULL){}
	
	

private:
	bool m_bRead;
	bool m_bWrite;
	sockfd m_nSocketFd;
	bool m_bIsListen;
	ModelManager * m_pModelManager;
	ReadBuffer m_bufferRead;
	WriteBuffer  m_bufferWrite;
	read_cb m_pReadCBFunc;
	write_cb m_pWriteCBFunc;
	error_cb m_pErrorCBFunc;
	SocketIndex * m_pSocketIndex;
};



#endif
