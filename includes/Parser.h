#pragma once

#include <vector>
#include "Server.h"
#include <fstream>
#include <algorithm>
#include <sstream>

#define PORT_MAX 65535


class Parser
{
	private:
		std::vector<Server> m_Servers;
	private:
		size_t getLineNumber(const std::string& data, size_t pos) const;
		bool isDigits(const std::string& input) const;
		bool isIpV4(const std::string& digit) const;
		bool isIp(const std::string& input) const;
		void removeComments(std::string& fileData) const;
		void removeWhiteSpaces(std::string& fileData) const;
		Location parseLocation(const std::string& data, const std::string& path) const;
		size_t findStartBracket(const std::string& to_find, size_t startPos, const std::string& fileData) const;
		size_t findEndBracket(size_t startPos, const std::string& fileData) const;
		Server parseServer(const std::string& data) const;
		void makeServers(const std::string& fileData);
		void (*getCorrespondingMethod(const std::string& str) const)(Request&, Response&);
	public:
		Parser();
		~Parser();
	public:
		std::vector<Server> parse(std::ifstream& infile);
};

std::vector<std::string> split(const std::string& str, const std::string& sep);

