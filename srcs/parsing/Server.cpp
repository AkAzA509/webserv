/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:27:25 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/11 22:22:15 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"

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

void Server::addPort(int port)
{
	m_port.push_back(port);
}

void Server::removePort(size_t idx)
{
	if (idx < m_port.size())
		m_port.erase(m_port.begin() + idx);
}

std::vector<int> Server::getPorts() const
{
	return m_port;
}

int Server::getPort(size_t idx) const
{
	return m_port[idx];
}

void Server::setServerName(const std::string& name) { m_serverName = name; }

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