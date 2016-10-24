#include "Mutex.h"


#if defined (_WIN32)
# include <windows.h>
#endif 
#include <assert.h>


// Mutex lock

Mutex::Mutex()
{
#ifndef WIN32
    int ret = pthread_mutex_init(&mId, NULL);
    ASSERT(ret == 0);
#else
    // Note:  Windows Critical sections are recursive in nature and perhaps
    //        this implementation calls for a non-recursive implementation
    //        (since there also exists a RecursiveMutex class).  The effort
    //        to make this non-recursive just doesn't make sense though. (SLG)
    mId = new CRITICAL_SECTION();
    assert(mId);
    InitializeCriticalSection((CRITICAL_SECTION *)mId);
#endif
}

Mutex::~Mutex()
{
#ifndef WIN32
    int ret = pthread_mutex_destroy(&mId);
    ASSERT(ret == 0);
#else
    DeleteCriticalSection((CRITICAL_SECTION *)mId);
    delete mId;
#endif
}

void 
Mutex::lock()
{
#ifndef WIN32
    int ret = pthread_mutex_lock(&mId);
    ASSERT(ret == 0);
#else
    EnterCriticalSection((CRITICAL_SECTION *)mId);
#endif
}

// Spin lock

void 
Mutex::unlock()
{
#ifndef WIN32
    int ret = pthread_mutex_unlock(&mId);
    ASSERT(ret == 0);
#else
    LeaveCriticalSection((CRITICAL_SECTION *)mId);
#endif
}

SpinMutex::SpinMutex()
{
#ifndef WIN32
#if !defined(__APPLE__)
    int ret = pthread_mutex_init(&mId, (const pthread_mutexattr_t*)PTHREAD_PROCESS_PRIVATE);
    ASSERT(ret == 0);
#else
    int ret = pthread_mutex_init(&mId, (const pthread_mutexattr_t*)NULL);
    ASSERT(ret == 0);
#endif
#else
    mId = new CRITICAL_SECTION();
    assert(mId);
    InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION *)mId, 4000);
#endif
}

SpinMutex::~SpinMutex()
{
#ifndef WIN32
    int ret = pthread_mutex_destroy(&mId);
    ASSERT(ret == 0);
#else
    DeleteCriticalSection((CRITICAL_SECTION *)mId);
    delete mId;
#endif
}

void 
SpinMutex::lock()
{
#ifndef WIN32
    int ret = pthread_mutex_lock(&mId);
    ASSERT(ret == 0);
#else
    EnterCriticalSection((CRITICAL_SECTION *)mId);
#endif
}
		
void 
SpinMutex::unlock()
{
#ifndef WIN32
    int ret = pthread_mutex_unlock(&mId);
    ASSERT(ret == 0);
#else
    LeaveCriticalSection((CRITICAL_SECTION *)mId);
#endif
}

// read and write lock.

ReadWriteMutex::ReadWriteMutex()
#ifdef WIN32
    :mReaderCount(0), mWriterHasLock(false), mPendingWriterCount(0)
#endif
{
#ifndef WIN32
    pthread_rwlock_init(&mId, 0);
#endif
}

ReadWriteMutex::~ReadWriteMutex()
{
#ifndef WIN32
    pthread_rwlock_destroy(&mId);
#endif
}

void 
ReadWriteMutex::unlock()
{
#ifdef WIN32
    lock lock(mMutex);

    // Unlocking a write lock.
    if (mWriterHasLock)
    {
        assert(mReaderCount == 0);
        mWriterHasLock = false;

        // Pending writers have priority. Could potentially starve readers.
        if (mPendingWriterCount > 0)
        {
            mPendingWriteCondition.signal();
        }

        // No writer, no pending writers, so all the readers can go.
        else
        {
            mReadCondition.broadcast();
        }
    }

    // Unlocking a read lock.
    else
    {
        assert(mReaderCount > 0);
        mReaderCount--;

        if (mReaderCount == 0 && mPendingWriterCount > 0)
        {
            mPendingWriteCondition.signal();
        }
    }
#else
    pthread_rwlock_unlock(&mId);
#endif
}

void 
ReadWriteMutex::readLock()
{
#ifdef WIN32
    lock lock(mMutex);
   
    while (mWriterHasLock || mPendingWriterCount > 0)
    {
        mReadCondition.wait(mMutex);
    }
   
    mReaderCount++;
#else
    pthread_rwlock_rdlock(&mId);
#endif
}

void 
ReadWriteMutex::writeLock()
{
#ifdef WIN32
    lock lock(mMutex);
  
    mPendingWriterCount++;
    while (mWriterHasLock || mReaderCount > 0)
    {
        mPendingWriteCondition.wait(mMutex);
    }
   
    mPendingWriterCount--;
    mWriterHasLock = true;
#else
    pthread_rwlock_wrlock(&mId);
#endif
}

#if 0
unsigned int 
ReadWriteMutex::readerCount() const
{
   return (mReaderCount);   
}

unsigned int 
ReadWriteMutex::pendingWriterCount() const
{
   return (mPendingWriterCount);
}
#endif


