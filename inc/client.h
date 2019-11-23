/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#pragma once

#include "pthread_tools.h"
#include "connection.h"

namespace simple_chat
{
    using namespace pthread_tools;
    
    class Client : public Connection
    {
    private:

            Thread	m_receiver;

    public:

            Client() {}
            virtual ~Client() {}

            bool Start(const char * host, int port, const char * nickname);

            void WaitQuit();

    private:

            static void * Receiver(void * obj);

    };

}