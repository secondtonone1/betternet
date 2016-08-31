#include "readbuffer.h"

 ReadBuffer::ReadBuffer(sockfd fd):SocketBuffer(fd),m_nTotal(0)
 {
 
 }
	
 ReadBuffer::~ReadBuffer()
{
	
}
	
 ReadBuffer::ReadBuffer(const ReadBuffer & readBuffer):SocketBuffer(readBuffer),m_nTotal(readBuffer.m_nTotal)
{
	
}
	
 ReadBuffer &  ReadBuffer::operator = (const ReadBuffer & socketBuffer)
 {
	if(this == &socketBuffer)
	{
		return * this;
	}
	
	m_nSocket = socketBuffer.m_nSocket;
	
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
	
	
	m_nTotal = socketBuffer.m_nTotal;
	return *this;
 }
	
	int ReadBuffer::receive(Node * node)
	{
		
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
		m_nTotal += recvlen;
		return recvlen;
		
	}

	int ReadBuffer::popData(char * buf, int len)
	{
	

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

		m_nTotal -= offset;
		return offset;
	}