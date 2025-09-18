/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/09/18 15:05:07 by ggirault         ###   ########.fr       */
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

std::vector<std::string> split(const std::string& str, const std::string& sep)
{
	std::vector<std::string> result;
	size_t start = 0;
	size_t end;

	while ((end = str.find(sep, start)) != std::string::npos)
	{
		result.push_back(str.substr(start, end - start));
		start = end + sep.length();
	}
	if (start < end && !str.substr(start).empty())
		result.push_back(str.substr(start)); // add the last part
	return result;

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

std::vector<std::string> splitset(const std::string& str, const std::string& sep)
{
	std::vector<std::string> tokens;
	size_t start = 0, end = 0;
	while ((end = str.find_first_of(sep, start)) != std::string::npos)
	{
		if (end != start)
			tokens.push_back(str.substr(start, end - start));
		start = end + 1;
	}
	if (start < str.length())
		tokens.push_back(str.substr(start));
	return tokens;
}

std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos) return "";
	size_t last = str.find_last_not_of(" \t");
	return str.substr(first, (last - first + 1));
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

std::vector<std::string> splitLines(const std::string& data)
{
	std::vector<std::string> lines;
	std::istringstream ss(data);
	std::string line;
	
	while (std::getline(ss, line)) {
		lines.push_back(line);
	}
	return lines;
}

Directive Parser::parseDirective(const std::string& line, size_t line_number) const
{
	Directive dir;
	dir.line_number = line_number;
	
	std::string clean_line = line;
	if (clean_line[clean_line.length() - 1] == ';') {
		clean_line.erase(clean_line.end() - 1);
	}

	std::vector<std::string> tokens = splitset(clean_line, " \t");
	if (tokens.empty())
	{
		std::ostringstream o;

		o << "Empty directive at line " << line_number + 1;
		throw std::runtime_error(o.str());
	}

	dir.name = tokens[0];
	for (size_t i = 1; i < tokens.size(); i++)
	{
		if (!tokens[i].empty())
			dir.args.push_back(tokens[i]);
	}

	return dir;
}

Location Parser::parseLocationBlock(const std::vector<std::string>& lines, size_t& index, Server& server) const
{
	(void)server;
	Location location;
	std::string loc_line = trim(lines[index]);
	size_t brace_level = 0;
	size_t start_line = index;

	// Extract location path
	size_t path_start = loc_line.find(' ');
	size_t path_end = loc_line.find('{');
	if (path_start == std::string::npos || path_end == std::string::npos)
	{
		std::ostringstream o;

		o << "Invalid location block at line " << index + 1;
		throw std::runtime_error(o.str());
	}
	std::string path = trim(loc_line.substr(path_start, path_end - path_start));
	location.setPath(path);

	for (; index < lines.size(); index++)
	{
		std::string line = trim(lines[index]);
		if (line.empty()) continue;

		for (size_t i = 0; i < line.length(); i++)
		{
			char c = line[i];
			if (c == '{') brace_level++;
			if (c == '}') brace_level--;
		}

		// Skip comments
		if (line[0] == '#') continue;

		if (index == start_line + 1 && line == "{")
			continue;

		// End of location block
		if (brace_level == 0) break;

		if (line.find(';') != std::string::npos)
		{
			Directive dir = parseDirective(line, index);
			if (dir.name == "root")
			{
				location.setRoot(dir.args[0]);
			}
			else if (dir.name == "allow_methods")
			{
				for (std::vector<std::string>::const_iterator it = dir.args.begin(); it != dir.args.end(); ++it)
					location.addAllowedMethod(*it);
			}
			else if (dir.name == "index")
			{
				for (std::vector<std::string>::iterator index = dir.args.begin(); index != dir.args.end(); ++index)
					location.addIndexFile(*index);
			}
			else if (dir.name == "autoindex")
			{
				bool on = (dir.args[0] == "on");
				location.setAutoIndexOn(on);
			}
			else if (dir.name == "cgi_pass")
			{
				for (std::vector<std::string>::iterator path = dir.args.begin(); path != dir.args.end(); ++path)
					location.addCgiPath(*path);
			}
			else if (dir.name == "cgi_ext") 
			{
				for (std::vector<std::string>::iterator ext = dir.args.begin(); ext != dir.args.end(); ++ext)
					location.addCgiExt(*ext);
			}
			else if (dir.name == "return")
				location.setRedirectionPath(dir.args[0]);
			else if (dir.name == "upload_path") {
				if (dir.args.size() != 1) {
					throw std::runtime_error("upload_path requires exactly one argument");
				}
				location.setUploadPath(dir.args[0]);
			}
		}
	}
	return location;
}

int Parser::parsePort(const Directive& dir) const
{
	if (dir.args.size() != 1)
	{
		std::ostringstream o;
		o << "'listen' requires exactly one argument at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}
	if (!isDigits(dir.args[0]))
	{
		std::ostringstream o;
		o << "Invalid port number at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}
	int port = std::atoi(dir.args[0].c_str());
	if (port < 0 || port > PORT_MAX)
	{
		std::ostringstream o;
		o << "Port cannot be less than 0 or above 65535 at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}
	
	return port;
}

std::pair<int, std::string> Parser::parseErrorPage(const Directive& dir) const
{
	if (dir.args.size() != 2)
	{
		std::ostringstream o;
		o << "'error_page' requires exactly two argument at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}

	if (!isDigits(dir.args[0]))
	{
		std::ostringstream o;
		o << "Invalid error page number at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}
	int page = std::atoi(dir.args[0].c_str());
	
	return std::make_pair(page, dir.args[1]);
}

std::string Parser::parseServerName(const Directive& dir) const
{
	if (dir.args.size() != 1)
	{
		std::ostringstream o;
		o << "'server_name' requires exactly one argument at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}

	return dir.args[0];
}

std::string Parser::parseHost(const Directive& dir) const
{
	if (dir.args.size() != 1)
	{
		std::ostringstream o;
		o << "'host' requires exactly one argument at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}

	if (!isIp(dir.args[0]))
	{
		std::ostringstream o;
		o << "'host' argument need to be an ip, Example: 127.0.0.1\nat line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}
	return dir.args[0];
}

bool Parser::parseTimeout(const std::string& arg) const {
	if (arg.empty() || arg.find_first_not_of("0123456789") != std::string::npos)
		return false;

	errno = 0;
	char* end = NULL;
	unsigned long long num = std::strtoull(arg.c_str(), &end, 10);

	if (num < 200)
		return false;

	if (errno == ERANGE || end == arg.c_str() || *end != '\0' || num > static_cast<unsigned long long>(SIZE_MAX))
		return false;

	return true;
}

std::string Parser::parseRoot(const Directive& dir) const
{
	if (dir.args.size() != 1)
	{
		std::ostringstream o;
		o << "'root' requires exactly one argument at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}

	return dir.args[0];
}

size_t Parser::parseClientBodySize(const Directive& dir) const
{
	if (dir.args.size() != 1)
	{
		std::ostringstream o;
		o << "'client_max_body_size' requires exactly one argument at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}

	const std::string& arg = dir.args[0];
	if (arg.empty() || arg.find_first_not_of("0123456789") != std::string::npos)
	{
		std::ostringstream o;
		o << "'client_max_body_size' must be a positive integer at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}

	errno = 0;
	char* end = NULL;
	unsigned long long num = strtoull(arg.c_str(), &end, 10);

	if (errno == ERANGE || end == arg.c_str() || *end != '\0' || num > static_cast<unsigned long long>(SIZE_MAX))
	{
		std::ostringstream o;
		o << "'client_max_body_size' value out of range at line " << dir.line_number + 1;
		throw std::runtime_error(o.str());
	}

	return static_cast<size_t>(num);
}

void Server::addEnv(char **ep) {
	m_ep = ep;
}

Server Parser::parseServer(const std::string& data, char **ep) const
{
	Server server;
	std::vector<std::string> lines = splitLines(data);
	size_t brace_level = 0;
	bool in_server_block = true;
	server.addEnv(ep);

	std::string server_root = "";
	std::vector<std::string> server_index_files;
	std::map<int, std::string> server_error_pages;
	size_t server_client_max_body_size = 0;
	std::string server_host_ip = "";
	std::string server_server_name = "";

	for (size_t i = 0; i < lines.size(); i++)
	{
		std::string line = trim(lines[i]);
		if (line.empty() || line[0] == '#') continue;

		for (size_t i = 0; i < line.length(); i++)
		{
			char c = line[i];
			if (c == '{') brace_level++;
			if (c == '}') brace_level--;
		}

		if (line.find("location") == 0 && brace_level > 1)
		{
			Location location = parseLocationBlock(lines, i, server);
			server.addLocation(location);
			continue;
		}

		if (brace_level == 1) {
			Directive dir = parseDirective(line, i);
			if (dir.name == "listen") {
				server.addPort(parsePort(dir));
			}
			else if (dir.name == "server_name") {
				server_server_name = parseServerName(dir);
				server.setServerName(server_server_name);
			}
			else if (dir.name == "host") {
				server_host_ip = parseHost(dir);
				server.setHostIp(server_host_ip);
			}
			else if (dir.name == "root") {
				server_root = parseRoot(dir);
				server.setRoot(server_root);
				// Logger::log(RED, "Parsed Root: %s\n", server.getRoot().c_str());
			}
			else if (dir.name == "index") {
				server_index_files.clear();
				for (std::vector<std::string>::iterator it = dir.args.begin(); it != dir.args.end(); ++it) {
					server_index_files.push_back(*it);
					server.addIndexFile(*it);
				}
			}
			else if (dir.name == "error_page") {
				std::pair<int, std::string> result = parseErrorPage(dir);
				server_error_pages[result.first] = result.second;
				server.addErrorPage(result.first, result.second);
			}
			else if (dir.name == "client_max_body_size") {
				server_client_max_body_size = parseClientBodySize(dir);
				server.setClientMaxBodySize(server_client_max_body_size);
			}
			else if (dir.name == "upload_path") {
				if (dir.args.size() != 1) {
					throw std::runtime_error("upload_path requires exactly one argument");
				}
				server.setUploadPath(dir.args[0]);
			}
			else if (dir.name == "timeout_time") {
				if (!parseTimeout(dir.args[0]))
					throw std::runtime_error("timeout_time parameters must be a positive integer above 200 ms");
				server.setTimeout(dir.args[0]);
			}
		}

		if (in_server_block && brace_level == 0) {
			break;
		}
	}
	return server;
}

void Parser::makeServers(const std::string& fileData, char **ep)
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
		
		m_Servers.push_back(parseServer(fileData.substr(start, end - start), ep));
		pos = end + 1;
	}
}

std::vector<Server> Parser::parse(std::ifstream& infile, char **ep)
{
	std::stringstream ss;
	ss << infile.rdbuf();

	std::string fileData = ss.str();

	removeComments(fileData);
	removeWhiteSpaces(fileData);
	
	makeServers(fileData, ep);

	return this->m_Servers;
}