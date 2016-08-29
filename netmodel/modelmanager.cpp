#include "modelmanager.h"

extern int
make_socket_nonblocking(sockfd fd);

#ifdef WIN32
extern struct ModelOp win32ops;
#else
extern struct ModelOp epollops;
#endif

ModelManager::ModelManager()
{
	#ifdef WIN32
	m_pModelOp = &win32ops;
	#endif
	#ifdef linux
		m_pModelOp = &epollops;
	#endif
	m_pModelData = m_pModelOp->init(this);
}

ModelManager::~ModelManager()
{
	m_pModelOp = NULL;
	for(std::map<sockfd , SocketWrapper *>::iterator  wrapperIter = m_mapSocketWrappers.begin();
		wrapperIter != m_mapSocketWrappers.end(); wrapperIter++)
	{
		if(wrapperIter->second)
		{
			delete (wrapperIter->second);
			wrapperIter->second = NULL;
		}

		#ifdef _WIN32
				closesocket(wrapperIter->first);
			#endif

			#ifdef __linux__
				close(wrapperIter->first);
			#endif

	
	}

	m_mapSocketWrappers.clear();
}

SocketWrapper * ModelManager::addFdToManager(sockfd fd, bool isListen)
{
	SocketWrapper * socketWrapper = new SocketWrapper(fd,this,isListen);
	m_mapSocketWrappers.insert(std::pair<sockfd, SocketWrapper *>(fd,socketWrapper));
	return socketWrapper;
}
	

void ModelManager::delFdFromManager(sockfd fd)
{
	std::map<sockfd , SocketWrapper *>::iterator findIter = m_mapSocketWrappers.find(fd);
	if(findIter == m_mapSocketWrappers.end())
	{
		std::cout << "del fd failed , fd : " << fd << "not found!!" <<std::endl;
		return;
	}

	if(findIter->second)
	{
		delete (findIter->second);
		findIter->second = NULL;
	}

	m_mapSocketWrappers.erase(findIter);
#ifdef _WIN32
	closesocket(fd);
#endif

#ifdef __linux__
	close(fd);
#endif
}

void ModelManager::enableRead(sockfd fd)
{
	std::map<sockfd , SocketWrapper *>::iterator findIter = m_mapSocketWrappers.find(fd);
	if(findIter == m_mapSocketWrappers.end())
	{
		std::cout << "  fd :  " << fd << "not found!!" <<std::endl;

		#ifdef _WIN32
		closesocket(fd);
		#endif

		#ifdef __linux__
			close(fd);
		#endif

		return;
	}

	bool readFlag = findIter->second->isSetRead();
	//原socket可读，那么跳过设置
	if(readFlag)
	{
		return; 
	}
	//判断旧事件，更新旧事件
	short oldEvent = 0;
	short curEvent = 0;
	curEvent |= EV_READ;
	bool writeFlag = findIter->second->isSetWrite();
	if(writeFlag)
	{
		//原socket可写，那么更新旧事件
		oldEvent |= EV_WRITE;
		curEvent |= EV_WRITE;
	}

	findIter->second->setRead(true);
	SocketIndex * socketIndex = findIter->second->getSocketIndex();
	m_pModelOp->add(this,fd,oldEvent,curEvent,socketIndex);
}


void ModelManager::enableWrite(sockfd fd)
{
	std::map<sockfd , SocketWrapper *>::iterator findIter = m_mapSocketWrappers.find(fd);
	if(findIter == m_mapSocketWrappers.end())
	{
		std::cout << "  fd :  " << fd << "not found!!" <<std::endl;

		#ifdef _WIN32
		closesocket(fd);
		#endif

		#ifdef __linux__
			close(fd);
		#endif

		return;
	}

	bool writeFlag = findIter->second->isSetWrite();
	if(writeFlag)
	{
		//可写跳过
		return; 
	}

	short oldEvent = 0;
	short curEvent = 0;
	curEvent |= EV_WRITE;

	bool readFlag = findIter->second->isSetRead();
	//原socket可读，更新旧事件
	if(readFlag)
	{
		oldEvent |= EV_READ;
		curEvent |= EV_READ;
	}

	findIter->second->setWrite(true); 
	SocketIndex * socketIndex = findIter->second->getSocketIndex();
	m_pModelOp->add(this,fd,oldEvent,curEvent,socketIndex);
}

void ModelManager::disableRead(sockfd fd)
{
	std::map<sockfd , SocketWrapper *>::iterator findIter = m_mapSocketWrappers.find(fd);
	if(findIter == m_mapSocketWrappers.end())
	{
		std::cout << "  fd :  " << fd << "not found!!" <<std::endl;

		#ifdef _WIN32
		closesocket(fd);
		#endif

		#ifdef __linux__
			close(fd);
		#endif

		return;
	}

	bool readFlag = findIter->second->isSetRead();
	if(!readFlag)
	{
		//原socket不可读，跳出
		return; 
	}

	short oldEvent = 0;
	short curEvent = 0;
	//更新旧事件为可读
	oldEvent |= EV_READ;

	bool writeFlag = findIter->second->isSetWrite();
	if(writeFlag)
	{
		oldEvent |= EV_WRITE;	
		curEvent |= EV_WRITE;
	}

	findIter->second->setRead(false);
	SocketIndex * socketIndex = findIter->second->getSocketIndex();
	m_pModelOp->del(this,fd,oldEvent,curEvent,socketIndex);

}
	
void ModelManager::disableWrite(sockfd fd)
{
	std::map<sockfd , SocketWrapper *>::iterator findIter = m_mapSocketWrappers.find(fd);
	if(findIter == m_mapSocketWrappers.end())
	{
		std::cout << "  fd :  " << fd << "not found!!" <<std::endl;

		#ifdef _WIN32
		closesocket(fd);
		#endif

		#ifdef __linux__
			close(fd);
		#endif

		return;
	}

	bool writeFlag = findIter->second->isSetWrite();
	if(!writeFlag)
	{
		return; 
	}

	short oldEvent = 0;
	short curEvent = 0;
	oldEvent |= EV_WRITE;

	bool readFlag = findIter->second->isSetRead();
	if(readFlag)
	{
		oldEvent |= EV_READ;
		curEvent |= EV_READ;
	}

	findIter->second->setWrite(false);
	SocketIndex * socketIndex = findIter->second->getSocketIndex();
	m_pModelOp->del(this,fd,oldEvent,curEvent,socketIndex);
}

void * ModelManager::getModelData(void)
{
	return m_pModelData;
}

void ModelManager::insertActiveList(sockfd fd, short eventtype)
{
	if(eventtype&EV_READ)
	{
		m_nReadList.push_back(fd);
	}

	if(eventtype&EV_WRITE)
	{
		m_nWriteList.push_back(fd);
	}
}

void ModelManager::dispatch(int MilliSec)
{
	struct timeval tv;
	int res;

	tv.tv_sec = 0;
	tv.tv_usec = MilliSec*1000;

	res = m_pModelOp->dispatch(this, NULL);

	for (std::list<sockfd>::iterator iter=m_nReadList.begin();
		iter!=m_nReadList.end(); iter++ )
	{
		std::map<sockfd , SocketWrapper *>::iterator  findIter = m_mapSocketWrappers.find(*iter);
		if(findIter == m_mapSocketWrappers.end() )
		{
			continue;
		}

		if(findIter->second->ToRead() == -1)
		{
			disableRead(findIter->first);
			disableWrite(findIter->first);
			delFdFromManager(findIter->first);
		}

	}

	for(std::list<sockfd>::iterator iter=m_nWriteList.begin();
		iter!=m_nWriteList.end(); iter++)
	{
		std::map<sockfd , SocketWrapper *>::iterator  findIter = m_mapSocketWrappers.find(*iter);
		if(findIter == m_mapSocketWrappers.end() )
		{
			continue;
		}

		if(findIter->second->ToWrite() == -1)
		{
			disableRead(findIter->first);
			disableWrite(findIter->first);
			delFdFromManager(findIter->first);
		}

	}


	m_nReadList.clear();
	m_nWriteList.clear();	
}

SocketIndex * ModelManager::getSocketIndex(sockfd fd)
{
	std::map<sockfd , SocketWrapper *>::iterator findIter = m_mapSocketWrappers.find(fd);
	if(findIter == m_mapSocketWrappers.end())
	{
		std::cout << "  fd :  " << fd << "not found!!" <<std::endl;

		#ifdef _WIN32
		closesocket(fd);
		#endif

		#ifdef __linux__
			close(fd);
		#endif

		return NULL;
	}
	else
	{
		return findIter->second->getSocketIndex();
	}
}
