/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 15:34:03 by macorso           #+#    #+#             */
/*   Updated: 2025/08/02 11:08:46 by ggirault         ###   ########.fr       */
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
	// for (std::vector<Server>::iterator it = m_Servers.begin(); it != m_Servers.end(); ++it)
	// {
	// 	Server s = *it;
	// 	std::cout << s << std::endl;
	// }
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

void Config::launchServers() {
	try
	{
		for(std::vector<Server>::iterator it = m_Servers.begin(); it != m_Servers.end(); ++it) {
			// it->setupSocket();
			// it->waitConnection();
			std::string request =
				"POST / HTTP/1.1\r\n"
				"Host: localhost:8080\r\n"
				"Connection: keep-alive\r\n"
				"Content-Length: 9\r\n"
				"Cache-Control: max-age=0\r\n"
				"sec-ch-ua: \"Not)A;Brand\";v=\"8\", \"Chromium\";v=\"138\", \"Brave\";v=\"138\"\r\n"
				"sec-ch-ua-mobile: ?0\r\n"
				"sec-ch-ua-platform: \"Linux\"\r\n"
				"Origin: http://localhost:8080\r\n"
				"Content-Type: application/x-www-form-urlencoded\r\n"
				"Upgrade-Insecure-Requests: 1\r\n"
				"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36\r\n"
				"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8\r\n"
				"Sec-GPC: 1\r\n"
				"Accept-Language: en-US,en;q=0.9\r\n"
				"Sec-Fetch-Site: same-origin\r\n"
				"Sec-Fetch-Mode: navigate\r\n"
				"Sec-Fetch-User: ?1\r\n"
				"Sec-Fetch-Dest: document\r\n"
				"Referer: http://localhost:8080/\r\n"
				"Accept-Encoding: gzip, deflate, br, zstd\r\n"
				"Cookie: session_id=abc123def456; user_pref=theme_dark; lang=en; last_visit=2025-08-01T10:06:24.880Z\r\n"
				"\r\n"
				"color=red";
			it->parseRequest(request);
		}
	}
	catch(const std::exception& e)
	{
		Logger::log(RED, "Error: %s", e.what());
	}
}