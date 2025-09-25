#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <poll.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <numeric>
#include <sys/time.h>
#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctime>
#include <set>
#include <cstring>
#include <sstream>
#include <netdb.h>

extern volatile sig_atomic_t sig;

#define MAX_EVENT 100

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

class Request;
class Response;

class Location
{
	private:
		std::string m_path;
		std::string m_uploadPath;
		std::string m_redirection_path;
		std::vector<std::string> m_indexFiles;
		std::string m_root;
		std::vector<std::string> m_allowed_methods;
		bool m_autoIndex;
		std::vector<std::string> m_cgi_pass;
		std::vector<std::string> m_cgi_ext;
	public:
		Location();
		Location(const std::string& path);
		~Location();
		bool isAllowedMethod(const std::string& method) const;
		std::string getPath() const { return m_path; }
		std::string getUploadPath() const { return m_uploadPath; }
		std::string getRedirectionPath() const { return m_redirection_path; }
		const std::vector<std::string>& getIndexFiles() const { return m_indexFiles; }
		std::string getRoot() const { return m_root; }
		const std::vector<std::string>& getAllowedMethods() const { return m_allowed_methods; }
		bool isAutoIndexOn() const { return m_autoIndex; }
		std::vector<std::string> getCgiPath() const { return m_cgi_pass; }
		std::vector<std::string> getCgiExt() const { return m_cgi_ext; }
		void setRedirectionPath(const std::string& path) { m_redirection_path = path; }
		void addIndexFile(const std::string& indexFile) { m_indexFiles.push_back(indexFile); }
		void addAllowedMethod(const std::string& methodName) { m_allowed_methods.push_back(methodName); }
		void setAutoIndexOn(bool on) { m_autoIndex = on; }
		std::vector<std::string> getIndex() const { return m_indexFiles; }
		bool getAutoindex() const { return m_autoIndex; }
		void addCgiPath(const std::string& path) { m_cgi_pass.push_back(path); }
		void addCgiExt(const std::string& ext) { m_cgi_ext.push_back(ext); }
		void setUploadPath(const std::string& path) {
			m_uploadPath = path;
			if (!m_uploadPath.empty() && m_uploadPath[m_uploadPath.length()-1] != '/') {
				m_uploadPath += '/';
			}
		}
		void setPath(const std::string& path) {
			m_path = path;

			if (!m_path.empty() && m_path != "/" && m_path[m_path.length()-1] != '/') {
				m_path += '/';
			}
		}
		void setRoot(const std::string& root) {
			m_root = root;
			if (!m_root.empty() && m_root[m_root.length()-1] != '/') {
				m_root += '/';
			}
		}
};

unsigned long getCurrentTimeMs();

struct ClientState {
	std::string request_buffer;
	bool request_complete;
	unsigned long last_activity;
	ClientState() : request_complete(false), last_activity(getCurrentTimeMs()) {}
};

class Server
{
	private:
		char **m_ep;
		std::vector<int> m_socketFd;
		size_t m_Client_max_body_size;
		std::vector<size_t> m_port;
		std::string m_serverName;
		std::string m_hostIp;
		std::string m_root;
		std::vector<std::string> m_indexFiles;
		std::map<int, std::string> m_errorPages;
		std::string m_uploadPath;
		std::vector<Location> m_locations;
		std::map<int, ClientState> m_clients;
		std::string m_forcedResponse;
		unsigned long m_timeout;
	public:
		Server();
		~Server();
		void setupSocket();
		void waitConnection();
		bool recvClient(int epfd, struct epoll_event ev, int client_fd);
		bool requestComplete(std::string& request);
		void acceptClient(int ready, std::vector<int> socketFd, struct epoll_event *ev, int epfd);
		void sendClient(Response& response, int client_fd, int epfd, struct epoll_event ev);
		void cleanupClient(int epfd, int client_fd, struct epoll_event ev);
		void checkTimeouts(int epfd);
		void setErrorForced(int error_code, int client_fd, int epfd, struct epoll_event ev);

	public:
		std::vector<size_t> getPorts() const;
		void setUploadPath(const std::string& path) {
			m_uploadPath = path;
			if (!m_uploadPath.empty() && m_uploadPath[m_uploadPath.length()-1] != '/') {
				m_uploadPath += '/';
			}
		}
		std::string getUploadPath() const { return m_uploadPath; }
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
		std::string getClientRequest(int client_fd);
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
		inline void setClientMaxBodySize(int size) { m_Client_max_body_size = size; }
		void setClientMaxBodySize(size_t size);
		void addEnv(char **ep);
		void setTimeout(const std::string& time);
	};
	
std::ostream& operator<<(std::ostream& o, const Location& loc);
std::ostream& operator<<(std::ostream& o, const Server& server);

// Utils

std::vector<std::string> splitRequest(const std::string& str);
void print_error(const std::string& str, int fd);
std::string loadFile(const std::string& path);
std::string getFileType(const std::string& path);
std::string normalizePath(const std::string& path);
std::string urlDecode(const std::string& str);
std::vector<std::string> prepareCgiEnv(const Request& req, const std::string& scriptName, const std::string& reqPath);

// Signaux

void sigint_handler(int);
void sigterm_handler(int);
void sigpipe_handler(int);
void sigchld_handler(int);