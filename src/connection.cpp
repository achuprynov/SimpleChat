/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#include <unistd.h>
#include <error.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <iostream>

#include "connection.h"

namespace simple_chat
{

    bool Connection::SendMessage(const char * msg, int len)
    {
            if ( IsConnected() == false )
            {
                    return false;
            }

            char * pos = (char *)msg;

            // split message on blocks
            while (len > 0)
            {
                    int blockLen = len;

                    if (blockLen > BUFFER_SIZE)
                            blockLen = BUFFER_SIZE;
                    len -= blockLen;

                    // send size of block
                    while (1)
                    {
                            int written = send(m_socket, &blockLen, sizeof(int), 0);
                            if (written == sizeof(int))
                                    break;

                            if (written < 0)
                            {
                                    Disconnect("ERROR on send");
                                    return false;
                            }
                    }

                    // send block
                    while (blockLen > 0)
                    {
                            int written = send(m_socket, pos, blockLen, 0);
                            if (written < 0)
                            {
                                    Disconnect("ERROR on send");
                                    return false;
                            }
                            pos      += written;
                            blockLen -= written;
                    }
            }

            return true;
    }

    const char * Connection::RecvMessage()
    {
            if ( IsConnected() == false )
            {
                    return 0;
            }

            // receive size of block
            int msgSize = 0;
            int received = recv(m_socket, &msgSize, sizeof(int), 0);
            if (received <= 0)
            {
                    if (received < 0)
                            Disconnect("ERROR on recv");
                    else
                            Disconnect("Connection closed");
                    return 0;
            }

            if (msgSize > BUFFER_SIZE)
            {
                    Disconnect("Error msgSize > BUFFER_SIZE");
                    return 0;
            }

            // receive block
            char * pos = m_buffer;
            while (msgSize)
            {
                    int received = recv(m_socket, pos, msgSize, 0);
                    if (received <= 0)
                    {
                            if (received < 0)
                                    Disconnect("ERROR on recv");
                            else
                                    Disconnect("Connection closed");
                            return 0;
                    }
                    else
                    {
                            msgSize -= received;
                            pos	+= received;
                    }
            }

            *pos = 0;

            return m_buffer;
    }

    bool Connection::IsConnected()
    {
            if (m_socket < 0)
                    return false;
            return true;
    }

    void Connection::Disconnect(const char * reason)
    {
            if ( IsConnected() )
            {
                    if (reason)
                    {
                            std::cerr << reason << std::endl;
                    }

                    shutdown(m_socket, SHUT_RDWR);
                    close(m_socket);
                    m_socket = -1;
            }
    }

    bool Connection::Connect(const char * host, int port)
    {
            struct sockaddr_in addr;
            struct hostent *server;

            m_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (m_socket < 0)
            {
                    Disconnect("ERROR opening socket");
                    return false;
            }

            server = gethostbyname(host);
            if (server == NULL)
            {
                    Disconnect("ERROR no such host");
                    return false;
            }

            bzero((char *) &addr, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            bcopy((char *)server->h_addr, (char *)&addr.sin_addr.s_addr, server->h_length);

            if (connect(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
            {
                    Disconnect("ERROR connecting");
                    return false;
            }

            return true;
    }

    bool Connection::Listen(const char * host, int port)
    {
            struct sockaddr_in addr;

            m_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (m_socket < 0)
            {
                    Disconnect("ERROR opening socket");
                    return false;
            }

            bzero((char *) &addr, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(m_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0)
            {
                    Disconnect("ERROR on binding");
                    return false;
            }

            if (listen(m_socket, 10) < 0)
            {
                    Disconnect("ERROR on listen");
                    return false;
            }

            return true;
    }

    Connection Connection::Accept()
    {
            Connection newClient;

            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);

            newClient.m_socket = accept(m_socket, (struct sockaddr *) &addr, &len);
            if (newClient.m_socket < 0)
            {
                    newClient.Disconnect("ERROR on accept");
            }
            else
            {
                    char clientIpAddress[16];
                    inet_ntop(AF_INET, &(addr.sin_addr), clientIpAddress, sizeof(clientIpAddress));
                    std::cout << "Connection from client " << clientIpAddress << ":" << ntohs(addr.sin_port) << std::endl;
            }
            return newClient;
    }

}