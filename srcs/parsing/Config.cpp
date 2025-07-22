/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 15:34:03 by macorso           #+#    #+#             */
/*   Updated: 2025/07/23 01:30:31 by macorso          ###   ########.fr       */
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
	
	m_Servers = m_Parser.parse(infile);

	// for (std::vector<Server>::iterator it = m_Servers.begin(); it != m_Servers.end(); ++it)
	// {
	// 	Server s = *it;
	// 	std::cout << s << std::endl;
	// }
}

Config::Config(int ac, char **av) : m_ArgCount(ac), m_FileName(av[1] ? av[1] : std::string())
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

void Config::launchServers() {
	try
	{
		for(std::vector<Server>::iterator it = m_Servers.begin(); it != m_Servers.end(); ++it) {
			// it->setupSocket();
			// it->waitConnection();
			std::string request =
			"GET /tours HTTP/1.1\r\n"
			"Host: localhost\r\n"
			"Content-Length: 40\r\n"
			"Content-Type: application/octet-stream\r\n"
			"\r\n"
			"<128 bytes of binary data from 00 to 7F>";
			// std::string request2 =
			// "PUT /tours HTTP/1.1\r\n"
			// "Host: localhost\r\n"
			// "Content-Length: 40\r\n"
			// "Content-Type: application/octet-stream\r\n"
			// "\r\n"
			// "<128 bytes of binary data from 00 to 7F>";
			// std::string request3 =
			// "HEAD /tours HTTP/1.1\r\n"
			// "Host: localhost\r\n"
			// "Content-Length: 40\r\n"
			// "Content-Type: application/octet-stream\r\n"
			// "\r\n"
			// "<128 bytes of binary data from 00 to 7F>";
			// std::string request4 =
			// "PUT /tours HTTP/1.1\r\n"
			// "Host: localhost\r\n"
			// "Content-Length: 40\r\n"
			// "Content-Type: application/octet-stream\r\n"
			// "\r\n"
			// "<128 bytes of binary data from 00 to 7F>";
			it->parseRequest(request);
			// it->parseRequest(request2);
		}
	}
	catch(const std::exception& e)
	{
		Logger::log(RED, "Error: %s", e.what());
	}
}