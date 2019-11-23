/* 
 * Copyright (C) Alexander Chuprynov <achuprynov@gmail.com>
 * This file is part of solution of test task described in readme.txt.
 */
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "server.h"
#include "client.h"

using namespace simple_chat;

void PrintUsing(char * app)
{
	std::cout << "Run this program with follow options" << std::endl;
	std::cout << app << " server|client host port nickname" << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc < 5)
	{
		PrintUsing(argv[0]);
		return 1;
	}

	char * type		= argv[1];
	char * host		= argv[2];
	int    port		= atoi(argv[3]);
	char * nickname	= argv[4];

	if (strcmp(type, "server") == 0)
	{
		Server server;
		if (server.Start(host, port, nickname))
                {
			server.WaitQuit();
                }
	}
	else if (strcmp(type, "client") == 0)
	{
		Client client;
		if (client.Start(host, port, nickname))
                {
			client.WaitQuit();
                }
	}
	else
	{
		PrintUsing(argv[0]);
		return 1;
	}

	return 0;
}
