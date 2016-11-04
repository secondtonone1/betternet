#include <map>

#ifdef _WIN32
#include<ws2tcpip.h>
#endif

#ifdef __linux__
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#endif

#include<iostream>
#include "../netmodel/modelmanager.h"
#include "../networklib/msghandler.h"
using namespace std;

std::map<sockfd, MsgHandler*> m_msgHandlerMap;


static void tcpReadCB(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx)
{
	/*cout << "read success" <<endl;
	char msg [] = "i love you 1314!!!";
	cout <<"msg size is: "<< sizeof(msg) <<endl;
	wrapper->writeToBuffer(msg, sizeof(msg));*/
	
	std::map<sockfd, MsgHandler*>::iterator findIter = m_msgHandlerMap.find(fd);
	if(findIter != m_msgHandlerMap.end())
	{
		bool result = findIter->second->unserilizeMsg();
		if(!result)
		{
			wrapper->setDelFlag(true);
			delete(findIter->second);
			findIter->second = NULL;
			m_msgHandlerMap.erase(fd);
		}
	}
}
	
static void tcpWriteCB(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx)
{
	cout << "write success" <<endl;
}
	
static void tcpErrorCB(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx)
{
	std::map<sockfd, MsgHandler*>::iterator findIter = m_msgHandlerMap.find(fd);
	if(findIter != m_msgHandlerMap.end())
	{
		delete(findIter->second);
		findIter->second = NULL;
		m_msgHandlerMap.erase(fd);
	}
	cout << "error¡¡£¡£¡£¡" << endl;
}

void listenerReadCb(ModelManager * managerPoint,  SocketWrapper * wrapper, sockfd fd, void * ctx)
{
	cout << "begin accept!!!"<<endl;
	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(sockaddr_in));
	size_t addrlen = sizeof(sockaddr_in);
	sockfd acceptres = accept(fd, (sockaddr *)&serveraddr, (socklen_t *)&addrlen);
	if(acceptres == -1)
	{
		cout << "accept failed !" <<endl;
		int Error = getErrno();
		cout << "errorno is : "<< Error <<endl;
		return ;
	}
	
	SocketWrapper * tcpWrapper = managerPoint->addFdToManager(acceptres);
	tcpWrapper->registercb(tcpReadCB, tcpWriteCB, tcpErrorCB);
	managerPoint->enableRead(acceptres);
	cout << "new connection arrived: "<< inet_ntoa(serveraddr.sin_addr) << endl;
	MsgHandler * msgHandler = new MsgHandler(); 
	msgHandler->setSocketWrapper(tcpWrapper);
	m_msgHandlerMap.insert(std::pair<sockfd, MsgHandler *>(acceptres,msgHandler));
 	
}


int main(int argc, char* argv[])
{
	#ifdef WIN32
		WSADATA wsa_data;
		WSAStartup(0x0201, &wsa_data);
	#endif

	sockfd m_nListenfd =  socket(AF_INET, SOCK_STREAM, 0);

	if(m_nListenfd == -1)
	{
		return false;
	}

	sockaddr_in listenAddr;
	memset(&listenAddr, 0, sizeof(listenAddr) );

//	listenAddr.sin_addr.s_addr = inet_addr("192.168.1.99");
	listenAddr.sin_addr.s_addr = INADDR_ANY;
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = htons(9995);
	//evutil_make_socket_nonblocking(m_nListenfd);
	int one = 1;  
	setsockopt(m_nListenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one)); 

	int bindres = bind(m_nListenfd, (sockaddr *)&listenAddr, sizeof(listenAddr));
	if(bindres == -1)
	{
		cout << "bind false!!!"<<endl;
		return false;
	}

	listen(m_nListenfd, 128);
	ModelManager * manager = new ModelManager();
	SocketWrapper * listenerWrapper = manager->addFdToManager(m_nListenfd,true);
	listenerWrapper->registercb(listenerReadCb, NULL, NULL);
	manager->enableRead(m_nListenfd);

	
	while(1)
	{
		manager->dispatch(10);
	}
	getchar();
	return 0;
}

