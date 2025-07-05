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

std::vector<Token> Parser::tokenise(const std::ifstream& infile)
{
	(void)infile;
	std::vector<Token> tokens;
	return tokens;
}

std::vector<Server> Parser::parse(std::ifstream& infile)
{
	std::vector<Server> servers;
	std::vector<Token> tokens;

	tokens = tokenise(infile);

	std::string line;
	while (std::getline(infile, line))
	{
		size_t pos;
		if ((pos = line.find("#")) != std::string::npos)
			line.erase(pos);
	}
	return servers;
}