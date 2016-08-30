#include "clock.h"

//在构造函数里创建锁
Mutex::Mutex()
{
	#if defined _WIN32
		m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	#endif

#ifdef __linux__
		int  r = pthread_mutex_init(&m_hMutex, 0);
		if(r != 0)
		{
			cout <<"Linux Lock init failed!!!"<<endl;
		}
#endif
}

//析构函数里销毁锁
Mutex::~ Mutex()
{
	#if defined _WIN32
		::CloseHandle(m_hMutex);
	#endif

#ifdef __linux__
		int r = pthread_mutex_destroy(&m_hMutex);
		if(r != 0)
		{
			cout << "Linux destroy failed!!!"<<endl;
		}

#endif
}

//互斥锁上锁
void Mutex::lock()  
{
	#if defined _WIN32
	  DWORD d = WaitForSingleObject(m_hMutex, INFINITE);
	#endif

	#ifdef __linux__
		int r = pthread_mutex_lock(&m_hMutex);
		if(r != 0)
		{
			cout << "linux lock failed!!!"<<endl;
		}
	#endif

}

//互斥锁解锁
void Mutex::unlock() 
{
	#if defined _WIN32
	  ::ReleaseMutex(m_hMutex);
	#endif

#ifdef __linux__
	  int ret = pthread_mutex_unlock(&m_hMutex);
	  if(ret != 0)
	  {
		  cout << "Linux unlock failed!!!" <<endl;
	  }
#endif
}
