# ifndef _BASE_THREAD_H_
#define _BASE_THREAD_H_

#ifdef __linux__
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#endif

#ifdef _WIN32
#include <Winsock2.h>
#include <process.h>
#endif



class BaseThread
{
public:
	BaseThread():m_nId(0),m_nShutDown(0){}
	virtual ~ BaseThread();
	void startup(unsigned int stackSize = 1024 * 1024 * 2);
	void join();
	void testFunc();
	virtual void threadWorkFunc(void)=0;
protected:
#ifdef _WIN32
	unsigned int m_nId;
#endif

#ifdef __linux__
	pthread_t m_nId;
#endif
		int m_nShutDown;
private:

#if defined _WIN32
	HANDLE m_hThread;
#endif

	
	;

};

#endif