#include "socketbuffer.h"
#include <assert.h>

//SocketBuffer::SocketBuffer()
//{
//	m_nSocket = 0;

	
//}

SocketBuffer::SocketBuffer(sockfd fd)
{
	m_nSocket = fd;
	
}

SocketBuffer::~SocketBuffer()
{
	
	
	m_nSocket = 0;

	clearDataList();

}


SocketBuffer::SocketBuffer(const SocketBuffer & socketBuffer)
{
	
	m_nSocket = socketBuffer.m_nSocket;
	
	clearDataList();

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


	