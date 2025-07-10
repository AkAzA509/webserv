/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:27:25 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/10 19:06:01 by macorso          ###   ########.fr       */
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

void Server::removePort(int idx)
{
	if (idx < m_port.size())
		m_port.erase(m_port.begin() + idx);
}

std::vector<int> Server::getPorts()
{
	return m_port;
}

int Server::getPort(int idx)
{
	return m_port[idx];
}

void Server::setServerName(const std::string& name)
{
	m_serverName = name;
}
