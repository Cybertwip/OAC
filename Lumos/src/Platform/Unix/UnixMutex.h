#pragma once

#include "Core/Mutex.h"

#include <pthread.h>

namespace Lumos
{
    class UnixMutex : public Mutex 
    {
    public:
        virtual void Lock() override;
        virtual void Unlock() override;
        virtual bool TryLock() override;

        UnixMutex(bool p_recursive);
        ~UnixMutex();

    private:
            pthread_mutexattr_t attr;
            pthread_mutex_t mutex;
    };
}
