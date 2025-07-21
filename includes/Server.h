#pragma once

#include <iostream>
#include <map>
#include <string>
#include "Request.h"
#include "Response.h"
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <sstream>

extern volatile sig_atomic_t sig;

#define MAX_EVENT 100

#define DEFAULT_CLIENT_MAX_BODY_SIZE 1_000_000  

#define HEADER "HTTP/1.1 200 OK\r\n"
#define ERROR_404 "HTTP/1.1 404 Not Found\r\n"
#define ERROR_400 "HTTP/1.1 400 Bad request\r\n"

#define CONTENT_TYPE "Content-Type: "
#define CONTENT_LENGHT "Content-Length: "
#define RETURN "\r\n"
#define CONNECTION_CLOSE "Connection: close\r\n"

struct Directive
{
	std::string name;
	std::vector<std::string> args;
	size_t line_number;
};

template <typename Iterator>
Iterator snext(Iterator it, typename std::iterator_traits<Iterator>::difference_type n = 1)
{
	while (n-- > 0)
		++it;
	return it;
}


class Location
{
	private:
		std::string m_path;
		std::string m_redirection_path;
		std::vector<std::string> m_indexFiles;
		std::string m_root;
		std::map<std::string, void (*)(Request&, Response&)> m_allowed_methods;
		bool m_autoIndex;
		std::vector<std::string> m_cgi_path;
		std::vector<std::string> m_cgi_ext;
	public:
		Location();
		Location(const std::string& path);
		~Location();
		std::string getPath() const { return m_path; }
		std::string getRedirectionPath() const { return m_redirection_path; }
		std::vector<std::string> getIndexFiles() const { return m_indexFiles; }
		std::string getRoot() const { return m_root; }
		std::map<std::string, void (*)(Request&, Response&)> getAllowedMethods() const { return m_allowed_methods; }
		bool isAutoIndexOn() const { return m_autoIndex; }
		std::vector<std::string> getCgiPath() const { return m_cgi_path; }
		std::vector<std::string> getCgiExt() const { return m_cgi_ext; }
		void setPath(const std::string& path) { m_path = path; }
		void setRedirectionPath(const std::string& path) { m_redirection_path = path; }
		void addIndexFile(const std::string& indexFile) { m_indexFiles.push_back(indexFile); }
		void setRoot(const std::string& root) { m_root = root; }
		void addAllowedMethod(const std::string& methodName, void (*method)(Request&, Response&)) { m_allowed_methods[methodName] = method; }
		void setAutoIndexOn(bool on) { m_autoIndex = on; }
		void addCgiPath(const std::string& path) { m_cgi_path.push_back(path); }
		void addCgiExt(const std::string& ext) { m_cgi_ext.push_back(ext); }
};

class Server
{
	private:
		std::vector<int> m_socketFd;
		std::vector<size_t> m_port;
		std::string m_serverName;
		std::string m_hostIp;
		std::string m_root;
		std::vector<std::string> m_indexFiles;
		std::map<int, std::string> m_errorPages;
		std::vector<Location> m_locations;
	public:
		Server();
		~Server();
		void setupSocket();
		void waitConnection();
		void recvClient(int epfd, std::vector<int> socketFd, struct epoll_event ev, std::string& request);
		bool requestComplete(std::string& request);
		void parseRequest(std::string& request);
		void acceptClient(int ready, std::vector<int> socketFd, struct epoll_event *ev, int epfd);
	public:
		std::vector<size_t> getPorts() const;
		size_t getPort(size_t idx) const;
		void addPort(size_t port);
		void removePort(size_t idx);
		std::string getServerName() const { return m_serverName; }
		void setServerName(const std::string& name);
		void setHostIp(const std::string& ip);
		std::vector<Location> getLocations() const { return m_locations; }
		std::string getHostIp() const { return m_hostIp; }
		std::string getRoot() const { return m_root; }
		void setRoot(const std::string& root);
		std::vector<std::string> getIndexFiles() const { return m_indexFiles; }
		std::string getIndexFile(size_t idx) const;
		void addIndexFile(const std::string& file);
		void removeIndexFile(size_t idx);
		const std::map<int, std::string>& getErrorPages() const { return m_errorPages; }
		std::string getErrorPage(int page) const;
		void addErrorPage(int page, const std::string& path);
		void addLocation(Location& loc);
		void removeLocation(size_t idx);
		void removeSocket(int idx);
};

std::ostream& operator<<(std::ostream& o, const Location& loc);
std::ostream& operator<<(std::ostream& o, const Server& server);

// Utils

void print_error(const std::string& str, std::vector<int> fd);
std::string loadFile(const std::string& path);

// Signaux

void sigint_handler(int);

// Request
