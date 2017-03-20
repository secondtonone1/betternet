#ifndef __LYNX_PLATFORM_LIB_CONDITION_H__
#define __LYNX_PLATFORM_LIB_CONDITION_H__

#ifndef _WIN32
# include <pthread.h>
#endif

#include <assert.h>



    class Mutex;   
    class  Condition
    {
    public:
        Condition();
        virtual ~Condition();

        void wait (Mutex& mtx);
        bool wait (Mutex& mutex, unsigned int ms);

        void wait (Mutex* mutex);
        bool wait (Mutex* mutex, unsigned int ms);
        void signal();
        void broadcast();

    private:
        Condition (const Condition&);
        Condition& operator=(const Condition&);

#ifndef _WIN32
        mutable  pthread_cond_t mId;
#else
        void enterWait ();

        void* mGate;
        void* mQueue;
        void* mMutex;
		//因超时或者虚假唤醒导致错过的信号量
        unsigned mGone; 
		//因为调用wait，可用信号量减少，阻塞信号量增加
        unsigned long mBlocked;
		//因为调用siganl，可用信号增加，阻塞信号量减少
        unsigned mWaiting;
#endif
   };
   


#endif // __LYNX_PLATFORM_LIB_CONDITION_H__



