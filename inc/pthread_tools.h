/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#pragma once

#include <pthread.h>

namespace pthread_tools
{

    class Mutex
    {
    private:
           pthread_mutex_t m_mutex;
           
           friend class Lock;
           friend class Event;
    public:
            Mutex()
            {
                    pthread_mutex_init(&m_mutex, NULL);
            }
            ~Mutex()
            {
                    pthread_mutex_destroy(&m_mutex);
            }
    protected:
            void Lock()
            {
                    pthread_mutex_lock(&m_mutex);
            }
            void Unlock()
            {
                    pthread_mutex_unlock(&m_mutex);
            }
    };

    class Lock
    {
    private:
            Mutex & m_mutex;
    public:
            Lock(Mutex & mutex) : m_mutex(mutex)
            {
                    m_mutex.Lock();
            }
            ~Lock()
            {
                    m_mutex.Unlock();
            }
    };

    class Event
    {
    private:
            pthread_cond_t	m_condv;
            pthread_mutex_t   & m_mutex;
    public:
            Event(Mutex & mutex) : m_mutex(mutex.m_mutex)
            {
                    pthread_cond_init(&m_condv, 0);
            }
            ~Event()
            {
                    pthread_cond_destroy(&m_condv);
            }
            void Signal()
            {
                    pthread_cond_signal(&m_condv);
            }
            void Wait()
            {
                pthread_cond_wait(&m_condv, &m_mutex);
            }
    };

    typedef void* (*ThreadFunc)(void*);

    class Thread
    {
    private:
            pthread_t	m_thread_id;

    public:

            Thread() : m_thread_id(-1) {}
            ~Thread() {}

            bool StartAttached(ThreadFunc func, void * arg)
            {
                    if (pthread_create(&m_thread_id, NULL, func, arg) == 0)
                    {
                            return true;
                    }
                    return false;
            }

            static bool StartDetached(ThreadFunc func, void * arg)
            {
                    pthread_t	thread_id;
                    pthread_attr_t	thread_attr;

                    if (pthread_attr_init(&thread_attr) == 0)
                    {
                            if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) == 0)
                            {
                                    if (pthread_create(&thread_id, &thread_attr, func, arg) == 0)
                                    {
                                            return true;
                                    }
                            }
                    }

                    return false;
            }

            void Join()
            {
                    if (m_thread_id != -1)
                    {
                            pthread_join(m_thread_id, 0);
                    }
            }
    };

}