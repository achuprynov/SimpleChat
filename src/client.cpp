/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#include <string.h>
#include <iostream>

#include "client.h"

namespace simple_chat
{

    bool Client::Start(const char * host, int port, const char * nickname)
    {
            // connect to chat server
            if (Connect(host, port) == false)
                    return false;

            // first send nickname
            if (SendMessage(nickname, strlen(nickname)) == false)
                    return false;

            // run receiver
            if (m_receiver.StartAttached(Receiver, this) == false)
            {
                    Disconnect("ERROR Receiver not started");
                    return false;
            }

            return true;
    }

    void Client::WaitQuit()
    {
            std::cout << "Type \"quit\" for close." << std::endl;

            // wait quit command
            while (IsConnected())
            {
                    std::string msg;
                    std::getline(std::cin, msg);

                    if (msg.compare("quit") == 0)
                    {
                            // disconnect from server
                            Disconnect();
                            break;
                    }

                    SendMessage(msg.c_str(), msg.length());
            }

            // wait receiver stopping
            m_receiver.Join();
    }

    void * Client::Receiver(void * obj)
    {
            Connection conn = *(Connection*)obj;

            while (conn.IsConnected())
            {
                    // read message from tcp connection
                    const char * msg = conn.RecvMessage();
                    if (msg == 0)
                            break;

                    // write to console
                    std::cout << msg << std::endl;
            }

            std::cout << "Connection to server is lost" << std::endl;
            return 0;
    }

}