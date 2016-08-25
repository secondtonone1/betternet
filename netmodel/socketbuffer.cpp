#include "socketbuffer.h"
#include <assert.h>

SocketBuffer::SocketBuffer()
{
	m_nSocket = 0;
	m_pCurSend = NULL;	
	m_bEnableRead = false;
	m_bEnableWrite = false;
	
}

SocketBuffer::SocketBuffer(int fd)
{
	m_nSocket = fd;
	m_pCurSend = NULL;
	m_bEnableRead = false;
	m_bEnableWrite = false;

}

SocketBuffer::~SocketBuffer()
{
	
	m_bEnableRead = false;
	m_bEnableWrite = false;
	m_nSocket = 0;
	m_pCurSend = NULL;
	clearDataList();

}


SocketBuffer::SocketBuffer(const SocketBuffer & socketBuffer)
{
	
	m_bEnableRead = socketBuffer.m_bEnableRead;
	m_bEnableWrite = socketBuffer.m_bEnableWrite;
	m_nSocket = socketBuffer.m_nSocket;
	

	clearDataList();

	Node * preNode = NULL;
	Node * nextNode = NULL;
	
	for(Node * node = socketBuffer.m_listData.m_pHead; node != NULL;  node = node->mNext)
	{
		Node * newnode = mallocNode();
		newnode->mLen = node->mLen;
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
	m_pCurSend = NULL;
}

SocketBuffer & SocketBuffer:: operator = (const SocketBuffer & socketBuffer)
{
	if(this == &socketBuffer)
	{
		return * this;
	}

	m_bEnableRead = socketBuffer.m_bEnableRead;
	m_bEnableWrite = socketBuffer.m_bEnableWrite;
	m_nSocket = socketBuffer.m_nSocket;

	clearDataList();

	Node * preNode = NULL;
	Node * nextNode = NULL;
	
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
	
	m_pCurSend = NULL;

	return *this;
}

	void SocketBuffer::deallocNode(Node * node)
	{
			free(node);
			node = NULL;
	}

	Node * SocketBuffer::copyNode(Node * node)
	{
		Node * newnode = (Node *)malloc(INITIALSIZE + sizeof( Node) + 1);
		memcpy(newnode + sizeof( Node),node + sizeof( Node), INITIALSIZE );
		return newnode;
	}
	
	Node* SocketBuffer::mallocNode(int len)
	{
		Node * node = NULL;
		 node = (Node *)malloc(INITIALSIZE + sizeof( Node)+1);
		if(!node)
		{
			cout << "malloc node failed !!!"<<endl;
			return NULL;
		}
		memset(node , 0, INITIALSIZE+1+sizeof(Node));
		node->mLen = len;
		node->mOff = 0;
		node->mNext = NULL;
		node->mPrev = NULL;
		return node;
	}

	void SocketBuffer::clearDataList()
	{

		if(m_pCurSend)
		{
			free(m_pCurSend);
			m_pCurSend = NULL;
		}

		if(!m_listData.m_pHead)
		{
			return;
		}

		Node * node = m_listData.m_pHead;

		while(node)
		{
			Node * nextNode = node->mNext;
			free(node);
			node = nextNode;
		}

		m_listData.m_pHead = NULL;
		m_listData.m_pTail = NULL;
	}


	int SocketBuffer::receive(Node * node)
	{
		if(!m_bEnableRead)
		{
			cout << "read flag is close!!!"<<endl;
			return -1;
		}

		int recvlen = recv(m_nSocket, (char *)node+sizeof(Node)+node->mOff , node->mLen, 0);
		
		if(recvlen < 0)
		{
			int Error = getErrno();
			#ifdef _WIN32
				if(Error == WSAEWOULDBLOCK)
				{
					cout << "socket receive need again , socket receive empty!!" <<endl;
						return 0;
				}
				cout << "socket receive failed!!!, errno is: %d"<<Error << endl;
				return -1;
			#endif

			#ifdef __linux__
				if( (Error == EWOULDBLOCK) || (Error == EAGAIN))
				{
					cout << "socket receive need again , socket receive empty!!" <<endl;
					return 0;
				}
				cout << "socket receive failed!!!, errno is: %d"<<Error << endl;
				return -1;
			#endif
		}

		if(recvlen == 0)
		{
			cout << "tcp connection closed"<<endl;
			return -1;
		}

		node->mOff += recvlen;
		node->mLen -= recvlen;

		return recvlen;
		
	}


	int SocketBuffer::sendto(Node * node)
	{
		if(!m_bEnableWrite)
		{
			cout << "write flag is close!!!"<<endl;
			return -1;
		}
		
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
				cout << "socket send failed!!!, errno is: %d"<<Error << endl;
				return -1;
			#endif

			#ifdef __linux__
				if( (Error == EWOULDBLOCK) || (Error == EAGAIN))
				{
					cout << "socket send need again , socket send empty!!" <<endl;
					return 0;
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

	void  SocketBuffer::setReadFlag(bool open)
	{
		m_bEnableRead = open;
	}
	
	void  SocketBuffer::setWriteFlag(bool open)
	{
		m_bEnableWrite = open;
	}

	

	void SocketBuffer::insertNodeTail(Node * node)
	{
		if(m_listData.m_pTail == m_listData.m_pHead && m_listData.m_pTail == NULL)
		{
			m_listData.m_pTail = node;
			m_listData.m_pHead = node;
			 node->mNext = NULL;
			 node->mPrev = NULL;
			return;
		}		

		m_listData.m_pTail->mNext = node;
		node->mPrev = m_listData.m_pTail;
		node->mNext = NULL;
		m_listData.m_pTail = node;
	}

	int SocketBuffer::pushData(char * msg, int len)
	{
		if(m_bEnableRead)
		{
			return 0;
		}

		int offset = 0;
		while(len > INITIALSIZE)
		{
			Node * node = mallocNode(INITIALSIZE);
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

	int SocketBuffer::popData(char * buf, int len)
	{
		if(m_bEnableWrite)
		{
			return 0;
		}

		int offset  = 0;
		
		Node * node = popFirstNode();
		if(!node)
		{
			return 0;
		}

		while(node && node->mOff <= len)
		{
			memcpy(buf+offset, (char*)node + sizeof(Node) , node->mOff);
			len -= node->mOff;
			offset += node->mOff;
			deallocNode(node);
			node = popFirstNode();
		}
		
		if(len && node)
		{
			memcpy(buf+offset, (char*)node + sizeof(Node) , len);
			offset += len;
			
			Node * newNode = mallocNode();
			newNode->mOff = node->mOff - len;
			newNode->mLen = INITIALSIZE - newNode->mOff ;
			memcpy((char *)newNode + sizeof(Node) , (char*)node + sizeof(Node) + len,  node->mOff - len);
			insertNodeHead(newNode);
			deallocNode(node);

		}

		return offset;
	}

	void SocketBuffer::insertNodeHead(Node * node)
	{
		if(m_listData.m_pTail == m_listData.m_pHead && m_listData.m_pTail == NULL)
		{
			m_listData.m_pTail = node;
			m_listData.m_pHead = node;
			node->mNext = NULL;
			node->mPrev = NULL;
			return;
		}

		m_listData.m_pHead->mPrev = node;
		node->mNext = m_listData.m_pHead;
		node->mPrev = NULL;
		m_listData.m_pHead = node;
	}

	Node * SocketBuffer::popFirstNode()
	{
		if(!m_listData.m_pHead )
		{
			return NULL;
		}

		if(!(m_listData.m_pHead->mNext))
		{
			Node * node = m_listData.m_pHead;
			m_listData.m_pHead = NULL;
			m_listData.m_pTail = NULL;
			return node;
		}

		Node * node = m_listData.m_pHead;
		node ->mNext->mPrev = NULL;
		m_listData.m_pHead = node->mNext;
		return node;
	}

	Node * SocketBuffer::getSendNode()
	{
		if(!m_pCurSend)
		{
			m_pCurSend = popFirstNode();
		}

		return m_pCurSend;
	}



	void SocketBuffer::clearSendNode()
	{
		if(m_pCurSend)
		{
			free(m_pCurSend);
			m_pCurSend = NULL;
		}
		
	}
