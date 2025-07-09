/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 19:53:10 by macorso           #+#    #+#             */
/*   Updated: 2025/07/09 20:01:50 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.h"

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

size_t Parser::findServerEnd(size_t endPos, const std::string& fileData) const
{
	size_t	i;
	size_t	scope;

	std::cout << fileData << std::endl;
	
	scope = 0;
	for (i = endPos + 1; fileData[i]; i++)
	{
		std::cout << i << std::endl;
		if (i > 580)
			std::cout << fileData[i] << std::endl;
		if (fileData[i] == '{')
			scope++;
		if (fileData[i] == '}')
		{
			if (!scope)
				return (i);
			scope--;
		}
	}
	return endPos;
}

size_t Parser::findServerStart(size_t startPos, const std::string& fileData) const
{
	size_t i = startPos;

	while (fileData[i])
	{
		if (fileData[i] == 's')
			break;
		if (!isspace(fileData[i]))
			throw std::runtime_error("Out of scope a");
			// throw std::runtime_error("Out of scope Character");
	}
	if (!fileData[i])
		return startPos;
	if (fileData.compare(i, 6, "server") != 0)
		throw std::runtime_error("Out of scope b");
	i += 6;
	while (fileData[i] && isspace(fileData[i]))
		i++;
	if (fileData[i] == '{')
		return i;
	else 
		throw std::runtime_error("Out of scope c");
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

std::vector<Server> Parser::makeServers(const std::string& fileData) const
{
	std::vector<Server> servers;

	// std::cout << fileData << std::endl;

	if (fileData.find("server", 0))
		throw std::runtime_error("Didn't find server");
	
	size_t start = 0;
	size_t end = 1;

	while (start != end && start < fileData.length())
	{
		start = findServerStart(start, fileData);
		end = findServerEnd(end, fileData);

		if (start == end)
			throw std::runtime_error("Scope of a keyword");
		start = end + 1;
		// std::cout << fileData.substr(start, end - start) << std::endl;
		// servers.push_back(parseServer(fileData.substr(start, end - start)));
	}
	return servers;
}

std::vector<Server> Parser::parse(std::ifstream& infile)
{
	
	std::vector<Server> servers;
	std::stringstream ss;

	ss << infile.rdbuf();

	std::string fileData = ss.str();

	removeComments(fileData);
	removeWhiteSpaces(fileData);
	
	servers = makeServers(fileData);
	#if DEBUG
		// for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
		// {
		// 	Logger::log(BLUE, )
		// }
	#endif

	return servers;
}