#include "netmodeldef.h"
#include <iostream>
#include <fcntl.h>
using namespace std;

int getErrno()
{
	#ifdef __linux__
		return errno; 
	#endif


	#ifdef _WIN32
		return	WSAGetLastError();
	#endif

}

int
make_socket_nonblocking(int fd)
{
#ifdef WIN32
	{
		u_long nonblocking = 1;
		if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
			cout << "fcntl failed, fd is : " << fd; 
			
			return -1;
		}
	}
#else
	{
		int flags;
		if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
			cout << "fcntl failed, fd is : " << fd;
			return -1;
		}
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
			cout << "fcntl failed, fd is : " << fd;
			return -1;
		}
	}
#endif
	return 0;
}
