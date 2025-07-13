/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:27:25 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/13 23:31:21 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.h"
#include "Logger.h"

Server::Server() {
	for (size_t i = 0; i < 1024; i++)
		m_socketFD[i] = -1;
}

Server::~Server() {
	for (size_t i = 0; i < 1024; i++) {
		if (m_socketFD[i] != -1)
			close(m_socketFD[i]);
	}
}

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

void Server::addSocket(int idx, int socket)
{
	if (m_socketFD[idx] != -1)
		close(m_socketFD[idx]);
	m_socketFD[idx] = socket;
}

void Server::removeSocket(int idx)
{
	if (m_socketFD[idx] != -1)
	{
		close(m_socketFD[idx]);
		m_socketFD[idx] = -1;
	}
}

Location::Location()
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
	#if DEBUG
		Logger::log(CYAN, "LOCATION DESTRUCTION");
	#endif
}