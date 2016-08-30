#include "basethread.h"
#include "assert.h"


#include <iostream>
using namespace std;

#if defined _WIN32
#include <process.h>

unsigned _stdcall threadFunc (void * param)
{
	
	Sleep(1000);
	assert(param);
	BaseThread * baseThread = static_cast<BaseThread *>(param);
	assert(baseThread);
	baseThread->threadWorkFunc();
	
	//cout << "hello world!" <<endl;
#if defined _WIN32
	_endthreadex(0);
#endif
	return 0;
}

#endif

#if defined __linux__
void * threadFunc(void * param)
{
	//
	cout <<"begin thread Func" <<endl;	
	//sleep(1000);
	cout << param <<endl;
	assert(param);
	BaseThread * baseThread = static_cast<BaseThread *>(param);
	cout << baseThread <<endl;
	assert(baseThread);
	cout << "ready to call networkfunc"<<endl;
	baseThread->threadWorkFunc();
	cout <<"success thread Func" <<endl;
	return NULL;
}

#endif



void BaseThread::startup(unsigned int stackSize)
{
	assert(m_nId == 0);
	#if defined _WIN32
	m_hThread =(HANDLE) _beginthreadex(NULL,0,threadFunc, this, 0, &m_nId);
	//cout << this <<endl;
	 ::SetThreadPriority(::GetCurrentThread(), 2);
	//让线程跑起来后再退出函数
	// Sleep(1000);
	
	#endif

	#ifdef __linux__
	 pthread_attr_t attr;
	 if (pthread_attr_init(&attr) != 0)
	 {
		 cout <<"Failed init thread attr" <<endl;
		 return ;
	 }

	 if (pthread_attr_setstacksize(&attr, stackSize) != 0)
	 {
		cout << "Failed to set stack size." <<endl;
		 return ;
	 }

	 if (pthread_create(&m_nId, &attr, threadFunc, this) != 0)
	 {
		 cout <<"pthread create failed!!!"<<endl;
		 return ;
	 }

	 if (pthread_attr_destroy(&attr) != 0) 
	 {
		 cout << "Failed to destroy thread attr." <<endl;
		 return ;
	 }

	#endif

}

BaseThread::~BaseThread()
{
	m_nShutDown = 1;
	join();
}

void BaseThread::join()
{
	if (m_nId == 0)
	{
		return;
	}

	#if defined _WIN32
	 DWORD exitCode;
	 while(1)
	 {
		if(GetExitCodeThread(m_hThread, &exitCode) != 0)
		{
			if(exitCode != STILL_ACTIVE)
			{
				break;

			}
			else
			{
				// wait之前， 需要唤起线程， 防止线程处于挂起状态导致死等
				ResumeThread(m_hThread);
				WaitForSingleObject(m_hThread, INFINITE);
			}
		}
		else
		{
			break;
		}
	 }
	
	 CloseHandle(m_hThread);
	#endif

	#ifdef __linux__
	 void* stat;
	 if (m_nId != pthread_self())
	 {
		 int r = pthread_join(m_nId, &stat);
		 if (r != 0)
		 {
			 cout << "Failed to call pthread_join" <<endl;
			 assert(0);
		 }
	 }

	#endif
	 m_nId = 0;
}


void BaseThread::testFunc()
{
	this->threadWorkFunc();
}
