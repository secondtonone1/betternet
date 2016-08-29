#include "writebuffer.h"

WriteBuffer::WriteBuffer(sockfd fd):SocketBuffer(fd)
 {
	m_pCurSend = NULL;
 }

WriteBuffer::~WriteBuffer()
{
	if(m_pCurSend)
		{
			free(m_pCurSend);
			m_pCurSend = NULL;
		}	

}

WriteBuffer::WriteBuffer(const WriteBuffer & writebuffer):SocketBuffer(writebuffer)
{
	if(m_pCurSend)
		{
			free(m_pCurSend);
			m_pCurSend = NULL;
		}	

	m_pCurSend = SocketBuffer::copyNode(writebuffer.m_pCurSend);
}

WriteBuffer &  WriteBuffer::operator = (const WriteBuffer & socketBuffer)
{
	if(this == &socketBuffer)
	{
		return * this;
	}
	
	m_nSocket = socketBuffer.m_nSocket;
	
	if(m_pCurSend)
		{
			free(m_pCurSend);
			m_pCurSend = NULL;
		}	

	m_pCurSend = SocketBuffer::copyNode(socketBuffer.m_pCurSend);
	clearDataList();

	Node * preNode = NULL;
	Node * nextNode = NULL;
	m_listData.m_pTail = NULL;
	m_listData.m_pHead = NULL;

	for(Node * node = socketBuffer.m_listData.m_pHead; node != NULL;  node = node->mNext)
	{
		Node * newnode = mallocNode();
		newnode->mLen = node->mLen;
		newnode->mOff = node->mOff;
		memcpy(newnode+sizeof(Node), node + sizeof(Node), node->mLen);
		newnode->mPrev = preNode;
		if(preNode)
		{
			preNode->mNext = newnode;
		}
		else
		{
			m_listData.m_pHead = newnode;
		}
		preNode = newnode;
	}

	m_listData.m_pTail = preNode;
	
	

	return *this;
}


int WriteBuffer::sendto(Node * node)
{
		int sendlen = send(m_nSocket, (char*)node+sizeof(Node) + node->mOff, 
			node->mLen, 0);

		if(sendlen < 0)
		{
			int Error = getErrno();
			#ifdef _WIN32
				if(Error == WSAEWOULDBLOCK)
				{
					cout << "socket send need again , socket send full !!" <<endl;
						return 0;
				}

				if(Error == WSAECONNRESET)
				{
					cout << "the other end has closed, errno msg is connreset!!! "<<endl;
						return -1;
				}

				cout << "socket send failed!!!, errno is: %d"<<Error << endl;
				return -1;
			#endif

			#ifdef __linux__
				if( (Error == EWOULDBLOCK) || (Error == EAGAIN))
				{
					cout << "socket send need again , socket send empty!!" <<endl;
					return 0;
				}

				if(Error == ECONNRESET)
				{
					cout << "the other end has closed, errno msg is connreset!!! "<<endl;
						return -1;
				}

				cout << "socket send failed!!!, errno is: %d"<<Error << endl;
				return -1;
			#endif
		}

		if(sendlen == 0)
		{
			cout << "tcp connection closed"<<endl;
			return -1;
		}
		
		node->mOff += sendlen;
		node->mLen -= sendlen;

		return sendlen;
	}




	Node * WriteBuffer::getSendNode()
	{
		if(!m_pCurSend)
		{
			m_pCurSend = popFirstNode();
		}

		return m_pCurSend;
	}



	void WriteBuffer::clearSendNode()
	{
		if(m_pCurSend)
		{
			free(m_pCurSend);
			m_pCurSend = NULL;
		}
		
	}


	int WriteBuffer::pushData(char * msg, int len)
	{
	
		int offset = 0;
		while(len > INITIALSIZE)
		{
			Node * node = mallocNode();
			memcpy((char *)node+ node->mOff + sizeof(Node), msg + offset, INITIALSIZE);
			offset += INITIALSIZE;
			len -= INITIALSIZE;
			insertNodeTail(node);
			
		}

		if(len)
		{
			Node * node = mallocNode(len);
			memcpy((char *)node+ node->mOff + sizeof(Node), msg + offset, len);
			offset += len;
			len -= len;
			insertNodeTail(node);
			
		}

		return offset;
	}

	