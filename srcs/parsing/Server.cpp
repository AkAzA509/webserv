/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ChloeMontaigut <ChloeMontaigut@student.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:27:25 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/29 14:04:20 by ChloeMontai      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.h"
#include <numeric>
#include "Logger.h"

Server::Server() : m_Client_max_body_size(std::numeric_limits<int>::max()) {}
Server::~Server() {}

void Server::addPort(size_t port)
{
	m_port.push_back(port);
}

void Server::removePort(size_t idx)
{
	if (idx < m_port.size())
		m_port.erase(m_port.begin() + idx);
}

std::vector<size_t> Server::getPorts() const
{
	return m_port;
}

size_t Server::getPort(size_t idx) const
{
	return m_port[idx];
}

void Server::setServerName(const std::string& name) { m_serverName = name; }

void Server::setHostIp(const std::string& ip)
{
	m_hostIp = ip;
}

void Server::setRoot(const std::string& root) { m_root = root; }

std::string Server::getIndexFile(size_t idx) const
{
	if (idx < m_indexFiles.size())
		return m_indexFiles[idx];
	return std::string();
}

void Server::addIndexFile(const std::string& file)
{
	m_indexFiles.push_back(file);
}

void Server::removeIndexFile(size_t idx)
{
	if (idx < m_indexFiles.size())
		m_indexFiles.erase(m_indexFiles.begin() + idx);
}

std::string Server::getErrorPage(int page) const
{
	std::map<int, std::string>::const_iterator it;

	if ((it = m_errorPages.find(page)) != m_errorPages.end())
		return it->second;
	return std::string();
}

void Server::addErrorPage(int page, const std::string& path)
{
	m_errorPages[page] = path;
}

void Server::addLocation(Location& loc)
{
	m_locations.push_back(loc);
}

void Server::removeLocation(size_t idx)
{
	if (idx < m_locations.size())
		m_locations.erase(m_locations.begin() + idx);
}

void Server::removeSocket(int idx)
{
	if (idx <= (int)m_socketFd.size())
	{
		close(m_socketFd[idx]);
		m_socketFd.erase(m_socketFd.begin() + idx);
	}
}

std::vector<std::string> splitRequest(const std::string& str) {
	std::vector<std::string> result;
	std::string line;
	std::istringstream stream(str);
	
	while (std::getline(stream, line)) {
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.end() - 1);
		result.push_back(line);
	}

	if (!str.empty() && (str[str.length() - 1] == '\n' || str[str.length() - 1] == '\r'))
		result.push_back("");

	return result;
}

std::ostream& operator<<(std::ostream& os, const Server& server)
{
	os << "Server {\n";
	os << "  Server Name: " << server.getServerName() << "\n";
	os << "  Host IP: " << server.getHostIp() << "\n";
	os << "  Root: " << server.getRoot() << "\n";

	os << "  Ports: [";
	for (size_t i = 0; i < server.getPorts().size(); ++i) {
		os << server.getPorts()[i];
		if (i != server.getPorts().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  Index Files: [";
	for (size_t i = 0; i < server.getIndexFiles().size(); ++i) {
		os << server.getIndexFiles()[i];
		if (i != server.getIndexFiles().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  Error Pages: {\n";
	for (std::map<int, std::string>::const_iterator it = server.getErrorPages().begin(); it != server.getErrorPages().end(); ++it) {
		os << "    " << it->first << ": " << it->second << "\n";
	}
	os << "  }\n";

	os << "  Locations:\n";
	const std::vector<Location>& locs = server.getLocations();
	for (size_t i = 0; i < locs.size(); ++i) {
		os << "    [" << i << "] " << locs[i] << "\n";
	}

	os << "}";
	return os;
}

Location::Location() : m_autoIndex(false) 
{
	#if DEBUG
		Logger::log(CYAN, "Location CONSTRUCTION");
	#endif
}

Location::~Location()
{
	#if DEBUG
		Logger::log(CYAN, "Location DESTRUCTION");
	#endif
}

Location::Location(const std::string& path) : m_path(path)
{
	std::cout << path << std::endl;
	#if DEBUG
		Logger::log(CYAN, "LOCATION DESTRUCTION");
	#endif
}


std::ostream& operator<<(std::ostream& os, const Location& loc)
{
	os << "Location {\n";
	os << "  Path: " << loc.getPath() << "\n";
	os << "  Redirection Path: " << loc.getRedirectionPath() << "\n";
	os << "  Root: " << loc.getRoot() << "\n";
	os << "  AutoIndex: " << (loc.isAutoIndexOn() ? "On" : "Off") << "\n";

	os << "  Index Files: [";
	for (size_t i = 0; i < loc.getIndexFiles().size(); ++i) {
		os << loc.getIndexFiles()[i];
		if (i != loc.getIndexFiles().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  CGI Paths: [";
	for (size_t i = 0; i < loc.getCgiPath().size(); ++i) {
		os << loc.getCgiPath()[i];
		if (i != loc.getCgiPath().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  CGI Extensions: [";
	for (size_t i = 0; i < loc.getCgiExt().size(); ++i) {
		os << loc.getCgiExt()[i];
		if (i != loc.getCgiExt().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  Allowed Methods: [";
	const std::map<std::string, void (*)(Request&, Response&)>& methods = loc.getAllowedMethods();
	for (std::map<std::string, void (*)(Request&, Response&)>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
		os << it->first;
		if (snext(it) != methods.end()) os << ", ";
	}
	os << "]\n";

	os << "}";
	return os;
}