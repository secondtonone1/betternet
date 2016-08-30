# ifndef _BASELOCK_H_
# define _BASELOCK_H_
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <pthread.h>
#endif

#include <iostream>
using namespace std;
//各种类型的锁的基类 baselock baseclass
class BaseLock
{
public:
	BaseLock(){}
	virtual ~BaseLock(){}
	virtual void lock()  = 0 ;
	virtual void unlock() = 0 ;
};

//互斥锁继承基类 mutexlock  inherit baselock
class Mutex :public BaseLock
{
public:
	Mutex();
	~Mutex();
	virtual void lock()  ;
	virtual void unlock() ;
private:
#if defined _WIN32
	HANDLE m_hMutex;
#endif

#ifdef __linux__
	pthread_mutex_t m_hMutex;
#endif
};


class CLock
{
public:
	CLock(   BaseLock & baseLock):m_cBaseLock(baseLock){
		//构造函数里通过基类锁调用加锁函数(多态)
		m_cBaseLock.lock();
	}
	~CLock(){
		//析构函数先解锁
		m_cBaseLock.unlock();
	}
private:
	//常引用变量，需要在初始化列表初始
	//多态机制
	  BaseLock & m_cBaseLock;
};







#endif 
