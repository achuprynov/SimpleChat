/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#pragma once

namespace simple_chat
{

    class Connection
    {
    private:

            int m_socket;

            static const int BUFFER_SIZE = 255;
            char	m_buffer[BUFFER_SIZE];
            
    public:

            Connection() : m_socket(-1) {}
            virtual ~Connection() {}

            bool SendMessage(const char * msg, int len);

            const char * RecvMessage();

            bool IsConnected();

            void Disconnect(const char * reason = 0);

    protected:

            bool Connect(const char * host, int port);

            bool Listen(const char * host, int port);

            Connection Accept();

    };

}