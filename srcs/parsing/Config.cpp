#include "Config.h"

Config::~Config()
{
	#if DEBUG
		std::cout << "Config destruction" << std::endl;
	#endif
}

void Config::parseConfigFile()
{
	if (m_ArgCount != 2)
		throw std::runtime_error(USAGE);
	if (m_FileName.find(".dlq") == std::string::npos)
		throw std::runtime_error(USAGE);
	
	std::ifstream infile(m_FileName.c_str());
	if (!infile.is_open())
		throw std::runtime_error("Couldn't open " + m_FileName);
	
	m_Servers = m_Parser.parse(infile);
}

Config::Config(int ac, char **av) : m_ArgCount(ac), m_FileName(av[1] ? av[1] : std::string())
{
	#if DEBUG
		std::cout << "Config creation" << std::endl;
	#endif

	try
	{
		parseConfigFile();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	
}