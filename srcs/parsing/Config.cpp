/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 15:34:03 by macorso           #+#    #+#             */
/*   Updated: 2025/10/13 12:53:33 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.h"
#include "Logger.h"

Config::~Config()
{
	#if DEBUG
		Logger::log(LIGHTMAGENTA, "Config destruction");
	#endif
}

std::vector<Server>& Config::getServer()
{
	return m_Servers;
}

void Config::parseConfigFile()
{
	if (m_ArgCount != 2)
		throw std::runtime_error(USAGE);
	if (m_FileName.find(".dlq") == std::string::npos)
		throw std::runtime_error(USAGE);
	
	std::ifstream infile(m_FileName.c_str());
	if (!infile.is_open())
		throw std::runtime_error("Couldn't open " + m_FileName);
	
	m_Servers = m_Parser.parse(infile, m_ep);
}

Config::Config(int ac, char **av, char** ep) : m_ArgCount(ac), m_FileName(av[1] ? av[1] : std::string()), m_ep(ep)
{
	#if DEBUG
		Logger::log(LIGHTMAGENTA, "Config creation");
	#endif

	try
	{
		parseConfigFile();
	}
	catch(const std::exception& e)
	{
		Logger::log(RED, "Error: %s", e.what());
	}
	
}

bool Config::verifServer(const Server& server) const
{
	if (server.getHostIp().empty() || server.getPorts().empty() || server.getServerName().empty())
		return false;
	
	const std::vector<size_t>& ports = server.getPorts();
	for (size_t i = 0; i < ports.size(); ++i) {
		for (size_t j = i + 1; j < ports.size(); ++j) {
			if (ports[i] == ports[j]) {
				Logger::log(RED, "Server %s has duplicate port %zu in configuration", server.getServerName().c_str(), ports[i]);
				return false;
			}
		}
	}
	return true;
}

void Config::launchServers() {
	try
	{
		int epfd = epoll_create(10);
		if (epfd < 0) {
			Logger::log(RED, "epoll creation failed");
			return;
		}

		std::vector<int> allSocketFds;
		bool hasValidServer = false;

		for(std::vector<Server>::iterator it = m_Servers.begin(); it != m_Servers.end(); ++it) {
			if (verifServer(*it) == false) {
				Logger::log(RED, "Server %s not correctly setup", it->getServerName().c_str());
				continue;
			}
			
			it->setupSocket();

			const std::vector<int>& serverSockets = it->getSocketFds();
			if (serverSockets.empty()) {
				Logger::log(RED, "Server %s has no valid sockets, skipping", it->getServerName().c_str());
				continue;
			}
			
			hasValidServer = true;
			for (size_t i = 0; i < serverSockets.size(); ++i) {
				struct epoll_event ev;
				ev.events = EPOLLIN;
				ev.data.fd = serverSockets[i];
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, serverSockets[i], &ev) < 0) {
					Logger::log(RED, "epoll_ctl add server failed for fd %d", serverSockets[i]);
					continue;
				}
				allSocketFds.push_back(serverSockets[i]);
			}
		}

		if (!hasValidServer || allSocketFds.empty()) {
			Logger::log(RED, "No valid servers configured. Exiting.");
			close(epfd);
			return;
		}

		Logger::log(GREEN, "Started with %zu servers and %zu total sockets", 
			m_Servers.size(), allSocketFds.size());

		signal(SIGINT, sigint_handler);
		signal(SIGTERM, sigterm_handler);
		signal(SIGPIPE, sigpipe_handler);
		signal(SIGCHLD, sigchld_handler);
		
		while(sig == 0) {
			struct epoll_event ev[MAX_EVENT];
			int ready = epoll_wait(epfd, ev, MAX_EVENT, 1000);
			
			if (ready == -1) {
				if (errno == EINTR)
					continue;
				perror("epoll_wait failed");
				break;
			}

			if (ready > 0) {
				for (int i = 0; i < ready; i++) {
					int fd = ev[i].data.fd;

					for(std::vector<Server>::iterator server_it = m_Servers.begin(); server_it != m_Servers.end(); ++server_it) {
						if (server_it->ownsFd(fd)) {
							server_it->handleEvent(epfd, ev[i], allSocketFds);
							break;
						}
					}
				}
			}

			for(std::vector<Server>::iterator it = m_Servers.begin(); it != m_Servers.end(); ++it) {
				it->checkTimeouts(epfd);
			}
		}
		for (size_t i = 0; i < allSocketFds.size(); ++i)
			close(allSocketFds[i]);
		close(epfd);
	}
	catch(const std::exception& e)
	{
		Logger::log(RED, "Error: %s", e.what());
	}
}