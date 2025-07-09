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
		void removeComments(std::string& fileData) const;
		void removeWhiteSpaces(std::string& fileData) const;
		size_t findServerStart(size_t startPos, const std::string& fileData) const;
		size_t findServerEnd(size_t endPos, const std::string& fileData) const;
		std::vector<Server> makeServers(const std::string& fileData) const;
	public:
		Parser();
		~Parser();
	public:
		std::vector<Server> parse(std::ifstream& infile);
};