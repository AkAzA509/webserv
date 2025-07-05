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
#include <signal.h>

extern volatile sig_atomic_t stop;

#define HEADER "HTTP/1.1 200 OK\r\n"
#define ERROR_404 "HTTP/1.1 404 Not Found\r\n"
#define ERROR_400 "HTTP/1.1 400 Bad request\r\n"

#define CONTENT_TYPE "Content-Type: "
#define CONTENT_LENGHT "Content-Length: "
#define RETURN "\r\n"
#define CONNECTION_CLOSE "Connection: close\r\n"

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
		int m_socketFD[1024];
	public:
		Server();
		~Server();
		std::vector<int> port;
		std::string serverName;
		std::string root;
		std::vector<std::string> indexFiles;
		std::map<int, std::string> errorPages;
		std::vector<Location> locations;
		void setupSocket();
		void waitConnection();
};

// Utils

void print_error(const std::string& str, int *fd);
std::string loadFile(const std::string& path);

// Signaux

void sigint_handler(int);

// changer le port de std::string a vector ou container pour gerer l'ecoute sur plusieurs port
