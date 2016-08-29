#ifndef _MODEL_MANAGER_H_
#define _MODEL_MANAGER_H_
 
#include "socketwrapper.h"
#include <iostream>
#include <map>
#include <list>
#include <set>

#include "netmodeldef.h"

#ifdef __linux__
#include <stdio.h>
#include <sys/time.h>
#endif

#ifdef _WIN32
#include <WinSock2.h>
#endif

struct ModelOp
{
	const char *name;
	void *(*init)(void *);
	int (*add)(void *, sockfd fd, short old, short events, void *fdinfo);
	int (*del)(void *, sockfd fd, short old, short events, void *fdinfo);
	int (*dispatch)(void *, struct timeval *);
	void (*dealloc)(void *);
};

class ModelManager
{
public:
	ModelManager();
	~ModelManager();
	//将文件描述符添加到socketwrapper的map中
	SocketWrapper * addFdToManager(sockfd fd, bool isListen = false);
	//将文件描述符从map中移除
	void delFdFromManager(sockfd fd);
	void enableRead(sockfd fd);
	void enableWrite(sockfd fd);
	void disableRead(sockfd fd);
	void disableWrite(sockfd fd);
	void * getModelData(void);
	void insertActiveList(sockfd fd, short eventtype);
	void dispatch(int MilliSec);
	SocketIndex * getSocketIndex(sockfd fd);
private:
	std::map<sockfd , SocketWrapper *> m_mapSocketWrappers;
	ModelOp * m_pModelOp;
	void * m_pModelData;
	std::list<sockfd> m_nReadList;
	std::list<sockfd> m_nWriteList;
	
};

#endif
