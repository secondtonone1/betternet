#ifndef _NET_MODEL_DEF_H_
#define  _NET_MODEL_DEF_H_
#ifdef __linux__
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#endif

#ifdef _WIN32

#include <WinSock2.h>
#include <WinSock.h>
#endif


#define EV_READ  0x01

#define EV_WRITE 0x02

#define MAX_SECONDS_IN_MSEC_LONG \
	(((LONG_MAX) - 999) / 1000)

#ifdef _WIN32
typedef SOCKET sockfd;
#endif

#ifdef __linux__
typedef  int  sockfd ;
#endif


#endif