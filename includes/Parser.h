#pragma once

#include <vector>
#include "Server.h"
#include <fstream>

enum TokenType
{
	SERVER,
	LISTEN,
	SERVER_NAME,
	INDEX,
	ROOT,
	ERROR_PAGE,
	LOCATION,
	LBRACE,
	RBRACE,
	PATH,
	AUTOINDEX,
	ALLOWED_METHOD,
	CGIPATH
};

struct Token
{
	TokenType type;
	std::string value;
};

class Parser
{
	private:
		std::vector<Token> tokenise(const std::ifstream& infile);
	public:
		Parser();
		~Parser();
	public:
		std::vector<Server> parse(std::ifstream& infile);
};