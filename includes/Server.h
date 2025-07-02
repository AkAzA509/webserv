#pragma once

#include <iostream>
#include <map>
#include <string>
#include "Request.h"
#include "Response.h"
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <fcntl.h>

struct Location
{
	std::string path;
	std::string root;
	std::map<std::string, void (*)(Request&, Response&)> allowed_methods;
	bool autoIndex;
	std::string cgi_pass;
};

class Server
{
	private:
		//int sokFd;
		//std::string website;
	public:
		std::string listenPort;
		std::string serverName;
		std::string root;
		std::vector<std::string> indexFiles;
		std::map<int, std::string> errorPages;
		std::vector<Location> locations;
};