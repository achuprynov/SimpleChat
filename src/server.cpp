/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#include <iostream>
#include "server.h"

namespace simple_chat
{
    void MessageQueue::PushMessageAndSignal(IncomingMessage * msg)
    {
            Lock l(m_mutex);

            m_messages.push(msg);

            m_event.Signal();
    }
    IncomingMessage * MessageQueue::PopMessageOrWait()
    {
            Lock l(m_mutex);

            if (m_messages.empty())
            {
                    m_event.Wait();
            }

            IncomingMessage * msg = m_messages.front();
            m_messages.pop();

            return msg;
    }

    bool ConnectedClients::AddClient(ClientConnection & client)
    {
            Lock l(m_mutex);

            std::map<std::string, Connection>::iterator it = m_connections.find(client.nickname);
            if (it != m_connections.end())
            {
                    return false;
            }
            else
            {
                    m_connections[client.nickname] = client.connection;
                    return true;
            }
    }

    void ConnectedClients::DelClient(std::string & nickname)
    {
            Lock l(m_mutex);
            m_connections.erase(nickname);
    }
    void ConnectedClients::SendMessageToAllClients(std::string & message, std::string except_nickname)
    {
        Lock l(m_mutex);
        std::map<std::string, Connection>::iterator it = m_connections.begin();
        while (it != m_connections.end())
        {
                std::string nickname	= (*it).first;
                Connection  & conn	= (*it).second;

                if (except_nickname.compare(nickname) != 0)
                {
                        conn.SendMessage(message.c_str(), message.length());
                }

                it++;
        }
    }    

    bool Server::Start(const char * host, int port, const char * nickname)
    {
            m_nickname = nickname;

            if (Listen(host, port) == false)
                    return false;

            // run Acceptor
            if (m_acceptor.StartAttached(Acceptor, this) == false)
            {
                    Disconnect("ERROR Acceptor not started");
                    return false;
            }

            // run Broadcaster
            if (m_broadcaster.StartAttached(Broadcaster, this) == false)
            {
                    Disconnect("ERROR Broadcaster not started");
                    m_acceptor.Join();
                    return false;
            }

            return true;
    }

    void Server::WaitQuit()
    {
            std::cout << "Type \"quit\" for close." << std::endl;

            // wait quit command
            while (IsConnected())
            {
                    std::string cmd;
                    std::cin >> cmd;

                    if (cmd.compare("quit") == 0)
                    {
                            // close all existing connection
                            Disconnect();

                            // empty message is flag for stop Broadcaster
                            PushMessageAndSignal(new IncomingMessage("", ""));
                            break;
                    }
            }

            // wait threads termination
            m_acceptor.Join();
            m_broadcaster.Join();
    }

    void * Server::Acceptor(void * obj)
    {
            Server * server = (Server*)obj;
            if (!server)
                return 0;

            while (1)
            {
                    Connection connection = server->Accept();
                    if (connection.IsConnected() == false)
                            break;

                    ClientConnection * client = new ClientConnection(server, connection);
                    if (Thread::StartDetached(Receiver, client) == false)
                    {
                            delete client;
                            connection.Disconnect();
                    }
            }

            return 0;
    }

    void * Server::Receiver(void * obj)
    {
            ClientConnection * client = (ClientConnection*)obj;
            
            Server           & server     = *(client->server);
            Connection       & connection = client->connection;
            
            static const char * user_exist = "User with this nickname already exist! Please use another name!";

            while (connection.IsConnected())
            {
                    // read message from tcp connection
                    const char * message = connection.RecvMessage();
                    if (message == 0)
                            break;

                    if (client->nickname.empty())
                    {
                            client->nickname = message;
                            
                            if (server.AddClient(*client) == false)
                            {
                                    connection.SendMessage(user_exist, sizeof(user_exist));
                                    connection.Disconnect("Attempt connection with existing nickname!");
                                    break;
                            }

                            // send server event
                            server.PushMessageAndSignal(new IncomingMessage(client->nickname.c_str(), "I am online"));
                    }
                    else
                    {
                            // add new message
                            server.PushMessageAndSignal(new IncomingMessage(client->nickname.c_str(), message));
                    }
            }

            
            if (client->nickname.empty() == false)
            {
                    server.DelClient(client->nickname);

                    // send server event
                    server.PushMessageAndSignal(new IncomingMessage(client->nickname.c_str(), "I am offline"));
            }

            delete client;

            return 0;
    }

    void * Server::Broadcaster(void * obj)
    {
            Server * server = (Server*)obj;

            while (true)
            {
                    // select new messages
                    IncomingMessage * message = server->PopMessageOrWait();
                    if (!message)
                    {
                            break;
                    }

                    // check for stop
                    if (message->nickname.empty() || message->message.empty())
                    {
                            delete message;
                            break;
                    }

                    // create message
                    std::string formatted_message = message->nickname + "> " + message->message;

                    // output to server console
                    std::cout << formatted_message << std::endl;

                    server->SendMessageToAllClients(formatted_message, message->nickname);

                    delete message;
            }

            return 0;
    }

}
