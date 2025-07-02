#include "Parser.h"

Parser::Parser()
{
	#if DEBUG
		std::cout << "Parser creation" << std::endl;
	#endif
}

std::vector<Server> Parser::parse(std::ifstream& infile)
{
	std::vector<Server> servers;

	std::string line;
	while (std::getline(infile, line))
	{
		size_t pos;
		if ((pos = line.find("#")) != std::string::npos)
			line.erase(pos);
		std::cout << line << std::endl;
	}
	return servers;
}