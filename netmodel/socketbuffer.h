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
friend class SocketWrapper;
private:
	SocketBuffer(int fd);
	~SocketBuffer();
	
	int receive(Node * node);
	int sendto( Node * node);
	void  setReadFlag(bool open);
	void  setWriteFlag(bool open);
	

	Node* mallocNode(int len = (INITIALSIZE+1));
	void deallocNode(Node * node);
	Node* copyNode(Node * node);
	void insertNodeTail(Node * node);
	void insertNodeHead(Node * node);

	Node * popFirstNode();

	Node * getSendNode();

	void clearSendNode();

	int pushData(char * buf, int len);

	int popData(char * buf, int len);

private:
	SocketBuffer();
	SocketBuffer(const SocketBuffer & socketBuffer);
	SocketBuffer &  operator = (const SocketBuffer & socketBuffer);

	
	

	void clearDataList();

	int m_nSocket;
	bool m_bEnableRead;
	bool m_bEnableWrite;
	
	NodeList m_listData;
	Node * m_pCurSend;
};


#endif
