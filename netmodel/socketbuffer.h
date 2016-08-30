#ifndef __SOCKET_BUFFER_H__
#define __SOCKET_BUFFER_H__

#include <stdlib.h>
#include <malloc.h>
#include <iostream>
#include <string.h>

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#endif

#include "netmodeldef.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WinSock.h>
#endif

using namespace std;

#define INITIALSIZE 200

extern int getErrno();

class Node 
{
public:
	Node() : mOff(0), mLen(0) ,mNext(NULL), mPrev(NULL){}
     int mOff; // valid data position for read 
     int mLen; // data length
            //LYNX_DLIST_ENTRY(Node) mEntries; 
     class Node *mNext;     /* next element */                                             \
     class Node *mPrev;

};

class NodeList
{
public:
	NodeList():m_pHead(NULL),m_pTail(NULL){}
	Node * m_pHead;
	Node * m_pTail;
};

class SocketBuffer
{
//friend class SocketWrapper;
protected:
	SocketBuffer(sockfd fd);
	~SocketBuffer();
	

	Node* mallocNode(int len = (INITIALSIZE+1));
	void deallocNode(Node * node);
	Node* copyNode(Node * node);
	void insertNodeTail(Node * node);
	void insertNodeHead(Node * node);
	Node * popFirstNode();

protected:
	//防止子类调用基类无参构造函数
	//SocketBuffer();
	SocketBuffer(const SocketBuffer & socketBuffer);
	

	void clearDataList();

	sockfd m_nSocket;

	
	NodeList m_listData;
	
};


#endif
