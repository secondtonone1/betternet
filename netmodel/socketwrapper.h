# ifndef __SOCKET_WRAPPER_H_
#define __SOCKET_WRAPPER_H_

#include "socketbuffer.h"
class ModelManager;
class SocketWrapper;

typedef void  (*read_cb) (ModelManager * managerPoint,  SocketWrapper * wrapper, int fd, void * ctx);
typedef void  (*write_cb)(ModelManager * managerPoint,  SocketWrapper * wrapper, int fd, void * ctx);
typedef void  (*error_cb)(ModelManager * managerPoint,  SocketWrapper * wrapper, int fd, void * ctx);


class SocketWrapper
{
public:
	
	SocketWrapper(int fd, ModelManager * modelManager, bool isListen);
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
	

private:
	SocketWrapper():m_bWrite(false), m_bRead(false),m_nSocketFd(0),m_bufferRead(0),m_bufferWrite(0),m_pModelManager(NULL){}
	
	

private:
	bool m_bRead;
	bool m_bWrite;
	int m_nSocketFd;
	bool m_bIsListen;
	ModelManager * m_pModelManager;
	SocketBuffer m_bufferRead;
	SocketBuffer  m_bufferWrite;
	read_cb m_pReadCBFunc;
	write_cb m_pWriteCBFunc;
	error_cb m_pErrorCBFunc;
};



#endif
