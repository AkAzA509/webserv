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
		inline std::string getPath() const { return m_path; }
		inline std::string getRedirectionPath() const { return m_redirection_path; }
		inline std::vector<std::string> getIndexFiles() const { return m_indexFiles; }
		inline std::string getRoot() const { return m_root; }
		inline std::map<std::string, void (*)(Request&, Response&)> getAllowedMethods() const { return m_allowed_methods; }
		inline bool isAutoIndexOn() const { return m_autoIndex; }
		inline std::vector<std::string> getCgiPath() const { return m_cgi_path; }
		inline std::vector<std::string> getCgiExt() const { return m_cgi_ext; }
		void setPath(const std::string& path) { m_path = path; }
		void setRedirectionPath(const std::string& path) { m_redirection_path = path; }
		void addIndexFile(const std::string& indexFile) { m_indexFiles.push_back(indexFile); }
		void setRoot(const std::string& root) { m_root = root; }
		void addAllowedMethod(const std::string& methodName, void (*method)(Request&, Response&)) { m_allowed_methods[methodName] = method; }
		void setAutoIndexOn(bool on) { m_autoIndex = on; }
		void addCgiPath(const std::string& path) { m_cgi_path.push_back(path); }
		void addCgiExt(const std::string& ext) { m_cgi_ext.push_back(ext); }
		void setRedirectionPath(const std::string& path) { m_redirection_path = path; }
};

class Server
{
	private:
		int m_socketFD[1024];
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
	public:
		std::vector<size_t> getPorts() const;
		size_t getPort(size_t idx) const;
		void addPort(size_t port);
		void removePort(size_t idx);
		inline std::string getServerName() const { return m_serverName; }
		void setServerName(const std::string& name);
		void setHostIp(const std::string& ip);
		inline std::string getHostIp() const { return m_hostIp; }
		inline std::string getRoot() const { return m_root; }
		void setRoot(const std::string& root);
		inline std::vector<std::string> getIndexFiles() const { return m_indexFiles; }
		std::string getIndexFile(size_t idx) const;
		void addIndexFile(const std::string& file);
		void removeIndexFile(size_t idx);
		inline std::map<int, std::string> getErrorPages() const { return m_errorPages; }
		std::string getErrorPage(int page) const;
		void addErrorPage(int page, const std::string& path);
		void addLocation(Location& loc);
		void removeLocation(size_t idx);
		void addSocket(int idx, int socket);
		void removeSocket(int idx);
};

// Utils

void print_error(const std::string& str, int *fd);
std::string loadFile(const std::string& path);

// Signaux

void sigint_handler(int);

// changer le port de std::string a vector ou container pour gerer l'ecoute sur plusieurs port
