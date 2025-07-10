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

#define DEFAULT_CLIENT_MAX_BODY_SIZE 1_000_000  

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
		std::vector<int> m_port;
		std::string m_serverName;
		std::string m_root;
		std::vector<std::string> m_indexFiles;
		std::map<int, std::string> m_errorPages;
		std::vector<Location> m_locations;
	public:
		Server();
		~Server();
		void setupSocket();
		void waitConnection();
		void addPort(int port);
		void removePort(int idx);
		std::vector<int> getPorts();
		int getPort(int idx);
		inline std::string getServerName() { return m_serverName; }
		void setServerName(const std::string& name);
};

// Utils

void print_error(const std::string& str, int *fd);
std::string loadFile(const std::string& path);

// Signaux

void sigint_handler(int);

// changer le port de std::string a vector ou container pour gerer l'ecoute sur plusieurs port
