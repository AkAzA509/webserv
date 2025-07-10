#pragma once

#include <vector>
#include "Server.h"
#include <fstream>
#include <sstream>

typedef enum {
    SERVER,
    DIRECTIVE,
    VALUE,
    LBRACE,
    RBRACE,
    SEMICOLON
} TokenType;

struct Token
{
	TokenType type;
	std::string value;

	Token(TokenType t, const std::string& v) : type(t), value(v) {}
};

class Parser
{
	private:
		std::vector<Server> m_Servers;
	private:
		void removeComments(std::string& fileData) const;
		void removeWhiteSpaces(std::string& fileData) const;
		size_t findServerStart(size_t startPos, const std::string& fileData) const;
		size_t findServerEnd(size_t endPos, const std::string& fileData) const;
		Server parseServer(const std::string& data) const;
		void makeServers(const std::string& fileData);
	public:
		Parser();
		~Parser();
	public:
		std::vector<Server> parse(std::ifstream& infile);
};

std::vector<std::string> split(const std::string& str, const std::string& sep);