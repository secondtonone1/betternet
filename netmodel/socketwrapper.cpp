#include "socketwrapper.h"
#include "modelmanager.h"
int readtimes = 0;
extern int
make_socket_nonblocking(sockfd fd);
static void defaultReadCB(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx)
{

}
	
static void defaultWriteCB(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx)
{

}
	
static void defaultErrorCB(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx)
{

}

SocketWrapper::SocketWrapper(sockfd fd, ModelManager * modelManager, bool isListen):m_bRead(false), m_bWrite(false), m_nSocketFd(fd),m_bufferRead(fd),m_bufferWrite(fd),
		m_pModelManager(modelManager),m_bIsListen(isListen)
	{
		make_socket_nonblocking(fd);
		
		m_pErrorCBFunc = &defaultErrorCB;
		m_pReadCBFunc = &defaultReadCB;
		m_pWriteCBFunc = &defaultWriteCB;
		m_pSocketIndex = (SocketIndex *)malloc(sizeof(struct SocketIndex));
		m_pSocketIndex->read_pos_plus1 = -1;
		m_pSocketIndex->write_pos_plus1 = -1;
	
	}

SocketWrapper::	~SocketWrapper(){m_bRead = false; m_bWrite = false; 

	if(m_pSocketIndex)
	{
		free(m_pSocketIndex);
		m_pSocketIndex = NULL;
	}
}


void SocketWrapper::setRead(bool enable )
{
	m_bRead = enable;
}
	
void SocketWrapper::setWrite(bool enable )
{
	m_bWrite = enable;
}

bool SocketWrapper::isSetRead(void)
{
	return m_bRead;
}

bool SocketWrapper::isSetWrite(void)
{
	return m_bWrite;
}

int SocketWrapper::ToRead()
{
	
	if(m_bIsListen)
	{
		(*m_pReadCBFunc)(m_pModelManager, this,  m_nSocketFd, NULL);
		return 0;
	}

	while(1)
	{
		readtimes++;
		cout << "readtimes is: "<< readtimes <<endl;
		cout << "begin read!!!" <<endl;
		Node * node = m_bufferRead.mallocNode();
		int recvRes = m_bufferRead.receive(node);
		if(recvRes == -1)
		{
			//触发错误回调函数
			m_bufferRead.deallocNode(node);
			m_pModelManager->disableRead(m_nSocketFd);
			(*m_pErrorCBFunc)(m_pModelManager, this,  m_nSocketFd, NULL);
			return -1;
		}

		if(recvRes == 0)
		{
			//触发正确回调函数
			m_bufferRead.deallocNode(node);
			cout << "read block"<<endl;
			(*m_pReadCBFunc)(m_pModelManager, this,  m_nSocketFd, NULL);
			break;
		}

		if(recvRes == INITIALSIZE)
		{
			//将节点插入链表中
			m_bufferRead.insertNodeTail(node);
			cout <<"read size :"<<INITIALSIZE <<endl;
			cout << "receive data is: "<<(char * )node + sizeof(Node) <<endl;
			//继续读取数据
			continue;
		}
		else
		{
			//将节点插入链表中
			m_bufferRead.insertNodeTail(node);
			cout <<"read size :"<<recvRes <<endl;
			cout << "receive data is: "<<(char * )node + sizeof(Node) <<endl;
			//触发正确回调函数
			(*m_pReadCBFunc)(m_pModelManager, this,  m_nSocketFd, NULL);
			continue;
		}
	}
	
	return 0;
}
	
int SocketWrapper::ToWrite()
{
	while(1)
	{
		Node * node = m_bufferWrite.getSendNode();
		
		if(!node)
		{
			cout << "no data to send !!!" <<endl;
			m_pModelManager->disableWrite(m_nSocketFd);
			return 0;
		}

		int remain = node->mLen;
		cout << "remain len is: " << remain <<endl;
		int sendRes = m_bufferWrite.sendto(node);
		if(sendRes == -1)
		{
			m_pModelManager->disableWrite(m_nSocketFd);
			//触发错误回调函数
			(*m_pErrorCBFunc)(m_pModelManager, this,  m_nSocketFd, NULL);
			return -1;
		}

		if(sendRes == 0)
		{
			//返回EWOULDBLOCK，监听写事件
			m_pModelManager->enableWrite(m_nSocketFd);
			//触发正确回调函数
			cout << "write block!!!"<<endl;
			(*m_pWriteCBFunc)(m_pModelManager, this,  m_nSocketFd, NULL);
			
			break;
		}
		
		char senddata[1024] = {0};
		memcpy(senddata, (char*)node + sizeof(Node) + node->mOff-sendRes, sendRes);
		cout <<"sendRes is: " << sendRes <<endl;
		cout << "senddata is: "<<  senddata <<endl;

		if(sendRes < remain)
		{
			//发送返回值小于期望发送的数值，那么监听写事件
			m_pModelManager->enableWrite(m_nSocketFd);
			cout << "possible ?"<<endl;
			continue;
		}
		else
		{
			//触发正确回调函数
			m_bufferWrite.clearSendNode();
			(*m_pWriteCBFunc)(m_pModelManager, this,  m_nSocketFd, NULL);
			continue;
		}
	}

	return 0;
}

int SocketWrapper::writeToBuffer(char * msg, int len)
{
	 int pushlen = m_bufferWrite.pushData(msg, len);
	 
	 if(pushlen)
	 {
		ToWrite();
	 }

	return pushlen;
}

int SocketWrapper::readFromBuffer(char * msg, int len)
{
	return m_bufferRead.popData(msg,len);
}

int SocketWrapper::getTotalRead()
{
	return m_bufferRead.m_nTotal;
}

void SocketWrapper::registercb(read_cb  readcallback,  write_cb writecallback, error_cb errorcallback)
{
	if(readcallback)
	{
		m_pReadCBFunc = readcallback;
	}

	if(writecallback)
	{
		m_pWriteCBFunc = writecallback;
	}

	if(errorcallback)
	{
		m_pErrorCBFunc = errorcallback;
	}
}

SocketIndex * SocketWrapper::getSocketIndex()
{
	return m_pSocketIndex;
}