/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 15:34:03 by macorso           #+#    #+#             */
/*   Updated: 2025/07/30 16:21:31 by macorso          ###   ########.fr       */
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
			it->setupSocket();
			it->waitConnection();
			// std::string request =
			// 	"POST / HTTP/1.1\r\n"
			// 	"Host: localhost:8080\r\n"
			// 	"Cookie: session_id=abc123; theme=dark; logged_in=true\r\n"
			// 	"Connection: keep-alive\r\n"
			// 	"Content-Length:\r\n"
			// 	"Cache-Control: max-age=0\r\n"
			// 	"sec-ch-ua: \"Not)A;Brand\";v=\"8\", \"Chromium\";v=\"138\", \"Brave\";v=\"138\"\r\n"
			// 	"sec-ch-ua-mobile: ?0\r\n"
			// 	"sec-ch-ua-platform: \"Linux\"\r\n"
			// 	"Origin: http://localhost:8080\r\n"
			// 	"Content-Type: multipart/form-data; boundary=----WebKitFormBoundarynGSd7qu4fkLSGPvS\r\n"
			// 	"Upgrade-Insecure-Requests: 1\r\n"
			// 	"User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/138.0.0.0 Safari/537.36\r\n"
			// 	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8\r\n"
			// 	"Sec-GPC: 1\r\n"
			// 	"Accept-Language: fr-FR,fr;q=0.8\r\n"
			// 	"Sec-Fetch-Site: same-origin\r\n"
			// 	"Sec-Fetch-Mode: navigate\r\n"
			// 	"Sec-Fetch-User: ?1\r\n"
			// 	"Sec-Fetch-Dest: document\r\n"
			// 	"Referer: http://localhost:8080/\r\n"
			// 	"Accept-Encoding: gzip, deflate, br, zstd\r\n"
			// 	"\r\n"
			// 	"------WebKitFormBoundarynGSd7qu4fkLSGPvS\r\n"
			// 	"Content-Disposition: form-data; name=\"file\"; filename=\"IMG_0441.jpeg\"\r\n"
			// 	"Content-Type: image/jpeg\r\n"
			// 	"\r\n"
			// 	"estbtrierogerovebbrogvierogiwebfperngpwofnoeibgrvoelrign;wefnpqeifwlrkgn;wekfnqlekfwlerkgnw\r\n"
			// 	"------WebKitFormBoundarynGSd7qu4fkLSGPvS--\r\n";
			// 	Response res = it->parseRequest(request);
		}
	}
	catch(const std::exception& e)
	{
		Logger::log(RED, "Error: %s", e.what());
	}
}