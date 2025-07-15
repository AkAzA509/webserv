/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lebonsushi <lebonsushi@student.42.fr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 19:53:10 by macorso           #+#    #+#             */
/*   Updated: 2025/07/15 22:41:47 by lebonsushi       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.h"
#include "Logger.h"
#include <limits>

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


size_t Parser::findEndBracket(size_t startPos, const std::string& fileData) const
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

size_t Parser::findStartBracket(const std::string& to_find, size_t startPos, const std::string& fileData) const
{
	size_t i = startPos;
	const size_t len = fileData.length();

	while (i < len && isspace(fileData[i]))
		i++;

	if (i >= len)
		return len;

	if (fileData.compare(i, to_find.length(), to_find) != 0)
	{
		std::ostringstream ss;

		ss << "Expected '" << to_find << "' directive";
		throw std::runtime_error(ss.str());
	}

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

bool Parser::isDigits(const std::string& input) const 
{
	if (input.empty())
		return false;
	for (std::string::const_iterator it = input.begin(); it != input.end(); ++it)
	{
		if (!isdigit(*it))
			return false;
	}
	return true;
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

size_t Parser::getLineNumber(const std::string& data, size_t pos) const
{
	return std::count(data.begin(), data.begin() + pos, '\n') + 1;
}

bool Parser::isIpV4(const std::string& digit) const
{
	if (digit.empty() || digit.length() > 3)
		return false;
	if (digit.length() > 1 && digit[0] == '0')
		return false;
	
	for (size_t i = 0; i < digit.length(); i++)
	{
		if (digit[i] < '0' || digit[i] > '9')
			return false;
	}
	const int num = std::atoi(digit.c_str());
	return num >= 0 && num <= 255;
}

bool Parser::isIp(const std::string& input) const
{
	std::string digit;
	std::istringstream ss(input);

	if (std::count(input.begin(), input.end(), '.') == 3)
	{
		for (int i = 0; i < 4; i++)
		{
			if (!std::getline(ss, digit, '.') || !isIpV4(digit))
				return false;
		}
		return true;
	}
	return false;
}

bool satoi(const std::string& str, int& result)
{
	char* endptr;
	errno = 0;
	long tmp = strtol(str.c_str(), &endptr, 10);

	// Check for conversion errors
	if (*endptr != '\0') return false;
	if (errno == ERANGE) return false;
	if (tmp > std::numeric_limits<int>::max() || tmp < std::numeric_limits<int>::min()) return false;

	result = static_cast<int>(tmp);
	return true;
}

void GET(Request& req, Response& resp)
{
	(void)req;
	(void)resp;
}

void PUT(Request& req, Response& resp)
{
	(void)req;
	(void)resp;
}

void DELETE(Request& req, Response& resp)
{
	(void)req;
	(void)resp;
}

void POST(Request& req, Response& resp)
{
	(void)req;
	(void)resp;
}

void ERROR (Request& req, Response& resp)
{
	(void)req;
	(void)resp;
}

void (*Parser::getCorrespondingMethod(const std::string& str) const)(Request&, Response&)
{
	if (str == "GET")
		return GET;
	else if (str == "PUT")
		return PUT;
	else if (str == "DELETE")
		return DELETE;
	else if (str == "POST")
		return POST;
	else
		return ERROR;
}

Location Parser::parseLocation(const std::string& data, const std::string& path) const
{
	Location location(path);

	std::vector<std::string> lines = split(data, "\n");
	
	for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
	{
		std::string line = *it;
		removeWhiteSpaces(line);

		if (line.empty() || line == "}") continue;

		if (line[line.length() - 1] != ';')
			throw std::runtime_error("Location directives must end with semicolon");

		line.erase(line.end() - 1);
		std::vector<std::string> words = split(line, " ");

		size_t s_v = words.size();

		if (words[0] == "root")
		{
			if (s_v != 2) throw std::runtime_error("Location root requires exactly 1 path");
			location.setRoot(words[1]);
		}
		else if (words[0] == "methods")
		{
			if (s_v < 2) throw std::runtime_error("Location methods requires at least one method");
			
			for (std::vector<std::string>::iterator itm = words.begin() + 1; itm != words.end(); ++itm)
			{
				location.addAllowedMethod(*itm, getCorrespondingMethod(*itm));
			}
		}
		else if (words[0] == "index")
		{
			if (s_v < 2) throw std::runtime_error("Location index requires at least one path to index file");

			for (std::vector<std::string>::iterator wit = words.begin() + 1; wit != words.end(); ++wit)
			{
				std::string path = *wit;
				location.addIndexFile(path);
			}
		}
		else if (words[0] == "autoindex")
		{
			if (s_v != 2) throw std::runtime_error("Autoindex requires on/off");
			
			location.setAutoIndexOn(words[1] == "on");
		}
		else if (words[0] == "cgi_path")
		{
			if (s_v < 2) throw std::runtime_error("cgi path requires at least 1 path");

			for (std::vector<std::string>::iterator wit = words.begin() + 1; wit != words.end(); ++wit)
			{
				std::string path = *wit;
				location.addCgiPath(path);
			}
		}
		else if (words[0] == "cgi_ext")
		{
			if (s_v < 2) throw std::runtime_error("cgi extension requires at least 1 extension");

			for (std::vector<std::string>::iterator wit = words.begin() + 1; wit != words.end(); ++wit)
			{
				std::string extension = *wit;
				location.addCgiExt(extension);
			}
		}
		else if (words[0] == "return")
		{
			if (s_v != 2) throw std::runtime_error("return requires only 1 redirection path");

			location.setRedirectionPath(words[1]);
		}
	}
	return location;
}

Server Parser::parseServer(const std::string& data) const
{
	std::vector<std::string> params = split(data + ' ', "\n\t");
	params.erase(params.begin());
	Server server;

	for (std::vector<std::string>::iterator it = params.begin(); it != params.end(); ++it)
	{
		if (it->empty())
			continue;

		std::string line = *it;
		removeWhiteSpaces(line);
		
		char last_char = line[line.length() - 1];
		if (last_char != ';' && last_char != '{' && last_char != '}')
		{
			std::cout << last_char << std::endl;
			throw std::runtime_error("Line must end with ;, {, or }");
		}
		
		if (last_char == '{' || last_char == '}')
			continue;
		
		line.erase(line.end() - 1);
		

		std::vector<std::string> words = split(line, " ");
		size_t v_size = words.size();

		for (size_t i = 0; i < v_size; i++)
		{
			if (words[0] == "listen")
			{
				if (v_size != 2)
					throw std::runtime_error("'listen' directive requires exactly 1 argument");
				if (isDigits(words[1]) == false)
					throw std::runtime_error("Port must be a number");
				
				int port = std::atoi(words[1].c_str());
				if (port < 0 || port > PORT_MAX)
					throw std::runtime_error("Server can only listens port between 0 and 65535");
				server.addPort(port);
			}
			else if (words[0] == "server_name")
			{
				if (v_size != 2)
					throw std::runtime_error("'server_name' requires exactly one name");
				server.setServerName(words[1]);
			}
			else if (words[0] == "host")
			{
				if (v_size != 2)
					throw std::runtime_error("'host' directive requires exactly 1 argument");
				if (!isIp(words[1]))
					throw std::runtime_error("Invalid IP address format");
				server.setHostIp(words[1]);
			}
			else if (words[0] == "root")
			{
				if (v_size != 2)
					throw std::runtime_error("'root' directive requires exactly 1 path");
				
				server.setRoot(words[1]);
			}
			else if (words[0] == "index")
			{
				if (v_size < 2)
					throw std::runtime_error("'index' directive requires at least 1 path to index file");
					
				for (std::vector<std::string>::iterator it = words.begin() + 1; it != words.end(); ++it)
				{
					std::string path = *it;
					server.addIndexFile(path);
				}
			}
			else if (words[0] == "error_page")
			{
				if (v_size != 3)
					throw std::runtime_error("'error_page' directive requires exactly 2 arguments");
				
				int error_page = 0;
				if (!satoi(words[1], error_page))
					throw std::runtime_error("'error_page' first argument can only be a number");

				server.addErrorPage(error_page, words[2]);
			}
			else if (words[0] == "location")
			{
				if (words.back() != "{")
					throw std::runtime_error("Location block must start with '{'");

				if (v_size != 3)
					throw std::runtime_error("Location requires a path after it");

				size_t block_start = data.find("{", it->find(words[0]));
				size_t block_end = findEndBracket(block_start, data);

				std::string loc_content = data.substr(block_start + 1, block_end - block_start - 1);

				Location loc = parseLocation(loc_content, words[1]);
				server.addLocation(loc);
				while (it != params.end() && data.find("}", it->find(words[0])) == std::string::npos)
					it++;
			}
		}
	}
	return server;
}

void Parser::makeServers(const std::string& fileData)
{
	if (fileData.find("server", 0))
		throw std::runtime_error("Didn't find server");
	
	
	size_t pos = 0;
	const size_t len = fileData.length();

	while (pos < len)
	{
		size_t start = findStartBracket("server", pos, fileData);
		size_t end = findEndBracket(start, fileData);
		
		if (start >= end)
			throw std::runtime_error("Invalid server block scope");
		
		m_Servers.push_back(parseServer(fileData.substr(start, end - start)));
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