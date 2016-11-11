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
#include <fcntl.h>
#endif

#include<iostream>
#include "../netmodel/modelmanager.h"
#include "../msghandler/msghandler.h"
using namespace std;
int count = 0;
std::map<sockfd, MsgHandler*> m_msgHandlerMap;
int idlefd = 0;

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
	
	size_t addrlen = sizeof(sockaddr_in);
	
	while(1)
	{
		memset(&serveraddr, 0, sizeof(sockaddr_in));
		sockfd acceptres = accept(fd, (sockaddr *)&serveraddr, (socklen_t *)&addrlen);
		if(acceptres == -1)
		{
			  int Error = getErrno();
				#ifdef _WIN32
				if(Error == WSAEWOULDBLOCK)
				{
					cout << "total connection has accepted !!" <<endl;
						return ;
				}

				if(Error == WSAECONNRESET)
				{
					cout << "the other end has closed, errno msg is connreset!!! "<<endl;
						return ;
				}

				cout << "socket accept failed!!!, errno is: %d"<<Error << endl;
				return ;
			#endif

			#ifdef __linux__
				if( (Error == EWOULDBLOCK) || (Error == EAGAIN))
				{
					cout << "total connection has accepted !!" <<endl;
					return ;
				}

				if(Error == ECONNRESET)
				{
					cout << "the other end has closed, errno msg is connreset!!! "<<endl;
						return ;
				}
				
				if(Error == EMFILE)

				    {
                                            
					cout << "too many files "<<endl;
					close(idlefd);
                                        idlefd = accept(fd, NULL, NULL);
					count++;
					cout << "after close" <<endl;
					cout << idlefd <<endl;
					while(idlefd > 0)
					{
						close(idlefd);
						idlefd = accept(fd, NULL, NULL);
						cout << "new accept"<<endl;
						cout << idlefd <<endl;
						count ++;
					}

					
					if(idlefd == -1)
					{
						Error = getErrno();
						cout << Error <<endl;
						if(Error == EWOULDBLOCK||Error == EAGAIN )
						{
							cout << "after clear more files, total files accept"<<endl;
						}
					}
					close(idlefd);
					
                                        
                                       idlefd = open("/dev/null", O_RDONLY | O_CLOEXEC);
					cout << "new connections count:" << count <<endl;
					return;
                                        
                                    }


				cout << "socket accept failed!!!, errno is: %d"<<Error << endl;
				return ;
			#endif

		}
	
	
	count ++;
	cout << "new connections count : " << count <<endl;
	SocketWrapper * tcpWrapper = managerPoint->addFdToManager(acceptres);
	tcpWrapper->registercb(tcpReadCB, tcpWriteCB, tcpErrorCB);
	managerPoint->enableRead(acceptres);
	cout << "new connection arrived: "<< inet_ntoa(serveraddr.sin_addr) << endl;
	MsgHandler * msgHandler = new MsgHandler(); 
	msgHandler->setSocketWrapper(tcpWrapper);
	m_msgHandlerMap.insert(std::pair<sockfd, MsgHandler *>(acceptres,msgHandler));
 	
	}
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
	 //¿¿¿¿¿¿¿¿¿
         idlefd = open("/dev/null",O_RDONLY|O_CLOEXEC);


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

