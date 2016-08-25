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
	int (*add)(void *, int fd, short old, short events, void *fdinfo);
	int (*del)(void *, int fd, short old, short events, void *fdinfo);
	int (*dispatch)(void *, struct timeval *);
	void (*dealloc)(void *);
};

class ModelManager
{
public:
	ModelManager();
	~ModelManager();
	//将文件描述符添加到socketwrapper的map中
	SocketWrapper * addFdToManager(int fd, bool isListen = false);
	//将文件描述符从map中移除
	void delFdFromManager(int fd);
	void enableRead(int fd);
	void enableWrite(int fd);
	void disableRead(int fd);
	void disableWrite(int fd);
	void * getModelData(void);
	void insertActiveList(int fd, short eventtype);
	void dispatch(int MilliSec);
private:
	std::map<int , SocketWrapper *> m_mapSocketWrappers;
	ModelOp * m_pModelOp;
	void * m_pModelData;
	std::list<int> m_nReadList;
	std::list<int> m_nWriteList;
	
};

#endif
