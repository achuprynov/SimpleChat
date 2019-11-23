/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#pragma once

#include <queue>
#include <map>

#include "pthread_tools.h"
#include "connection.h"

namespace simple_chat
{
    
    using namespace pthread_tools;
    
    class Server;
     
    class IncomingMessage
    {
    public:
            std::string nickname;
            std::string message;
            IncomingMessage(const char * nick, const char * msg)
            {
                    nickname = nick;
                    message  = msg;
            }
    };
    
    class MessageQueue
    {
    private:
            std::queue<IncomingMessage *> m_messages;
            Mutex	m_mutex;
            Event	m_event;
            
    public:
            MessageQueue() : m_event(m_mutex) { }
            virtual ~MessageQueue() {}
            
            void PushMessageAndSignal(IncomingMessage * msg);
            IncomingMessage * PopMessageOrWait();

    };

    class ClientConnection
    {
    public:
            Server    * server;
            Connection	connection;
            std::string nickname;

            ClientConnection(Server * srv, Connection conn)
            {
                    server = srv;
                    connection = conn;
            }
    };
    
    class ConnectedClients
    {
    private:
            std::map<std::string, Connection> m_connections;
            Mutex	m_mutex;

    public:
            ConnectedClients() {}
            virtual ~ConnectedClients() {}
        
            bool AddClient(ClientConnection & client);
            void DelClient(std::string & nickname);

            void SendMessageToAllClients(std::string & message, std::string except_nickname);
     };    

    class Server : public Connection, public MessageQueue, public ConnectedClients
    {
    private:

            Thread	m_acceptor;
            Thread	m_broadcaster;

            std::string m_nickname;

    public:

            Server() {}
            virtual ~Server() {}

            bool Start(const char * host, int port, const char * nickname);

            void WaitQuit();

    private:

            static void * Acceptor(void * obj);

            static void * Receiver(void * obj);

            static void * Broadcaster(void * obj);

    };

}