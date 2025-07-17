#pragma once

#include "Request.h"
#include "Response.h"
#include "Parser.h"
#include "Server.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#define USAGE "Usage: ./webserv <configuration file>.dlq"

class Config
{
	private:
		Parser m_Parser;
	private:
		int m_ArgCount;
		std::string m_FileName;
		std::vector<Server> m_Servers;
		void parseConfigFile();
	public:
		Config(int ac, char **av);
		std::vector<Server>& getServer();
		void launchServers();
		~Config();
};
