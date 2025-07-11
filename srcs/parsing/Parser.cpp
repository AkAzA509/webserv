/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 19:53:10 by macorso           #+#    #+#             */
/*   Updated: 2025/07/11 13:17:28 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.h"
#include "Logger.h"

Parser::Parser()
{
	#if DEBUG
		std::cout << "Parser creation" << std::endl;
	#endif
}

Parser::~Parser()
{
	#if DEBUG
		std::cout << "Parser destruction" << std::endl;
	#endif
}

size_t Parser::findServerEnd(size_t startPos, const std::string& fileData) const
{
	size_t scope = 0;
	const size_t len = fileData.length();
	
	for (size_t i = startPos + 1; i < len; i++)
	{
		if (fileData[i] == '{')
			scope++;
		else if (fileData[i] == '}')
		{
			if (scope == 0)
				return i;
			scope--;
		}
	}
	throw std::runtime_error("Unclosed server block");
}

size_t Parser::findServerStart(size_t startPos, const std::string& fileData) const
{
	size_t i = startPos;
	const size_t len = fileData.length();

	while (i < len && isspace(fileData[i]))
		i++;

	if (i >= len)
		return len;

	if (fileData.compare(i, 6, "server") != 0)
		throw std::runtime_error("Expected 'server' directive");

	i += 6;

	while (i < len && isspace(fileData[i]))
		i++;

	if (i >= len || fileData[i] != '{')
		throw std::runtime_error("Expected '{' after server directive");

	return i;
}

void Parser::removeComments(std::string& fileData) const
{
	size_t pos;

	pos = fileData.find('#');
	while (pos != std::string::npos)
	{
		size_t pos_end;
		pos_end = fileData.find('\n', pos);
		fileData.erase(pos, pos_end - pos);
		pos = fileData.find('#');
	}
}

void Parser::removeWhiteSpaces(std::string& str) const
{
	size_t	i = 0;

	while (str[i] && isspace(str[i]))
		i++;
	str = str.substr(i);
	i = str.length() - 1;
	while (i > 0 && isspace(str[i]))
		i--;
	str = str.substr(0, i + 1);
}

std::vector<std::string> split(const std::string& str, const std::string& sep)
{
	std::vector<std::string> res;
	
	if (sep.empty())
	{
		if (!str.empty())
			res.push_back(str);
		return res;
	}
	
	size_t start = 0;

	for (size_t end = str.find(sep, start); end != std::string::npos; end = str.find(sep, start))
	{
		if (start < end)
			res.push_back(str.substr(start, end - start));
		start = end + sep.length();
	}

	if (start < str.length())
		res.push_back(str.substr(start));
	return res;
}

Server Parser::parseServer(const std::string& data) const
{
	std::cout << data << std::endl;
	std::vector<std::string> param = split(data + ' ', "\n\t");

	
	return Server();
}

void Parser::makeServers(const std::string& fileData)
{
	if (fileData.find("server", 0))
		throw std::runtime_error("Didn't find server");
	
	
	size_t pos = 0;
	const size_t len = fileData.length();

	while (pos < len)
	{
		size_t start = findServerStart(pos, fileData);
		size_t end = findServerEnd(start, fileData);
		
		if (start >= end)
			throw std::runtime_error("Invalid server block scope");
		
		m_Servers.push_back(parseServer(fileData.substr(start, end - start + 1)));
		pos = end + 1;
	}
}

std::vector<Server> Parser::parse(std::ifstream& infile)
{
	std::stringstream ss;
	ss << infile.rdbuf();

	std::string fileData = ss.str();

	removeComments(fileData);
	removeWhiteSpaces(fileData);
	
	makeServers(fileData);
	#if DEBUG
		// for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
		// {
		// 	Logger::log(BLUE, )
		// }
	#endif

	return this->m_Servers;
}