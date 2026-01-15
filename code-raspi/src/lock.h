#ifndef LOCK_H
#define LOCK_H

#include <pthread.h>

struct Lock
{
    pthread_mutex_t *mut;
    Lock(pthread_mutex_t *mut)
        : mut(mut)
    {
        pthread_mutex_lock(mut);
    }
    ~Lock()
    {
        pthread_mutex_unlock(mut);
    }
};

#endif
