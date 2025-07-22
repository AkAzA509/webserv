/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 10:13:39 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/23 01:30:15 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.h"
#include "Parser.h"
#include "Logger.h"



Request::Request(Location loc, std::vector<std::string>& firstRequestLine, std::vector<std::string>& request) : m_loc(loc), m_methode(firstRequestLine[0]), m_url(firstRequestLine[1]), m_version(firstRequestLine[2]), m_foundBody(false) {
	std::cout << m_url << std::endl;
	std::string methode_name[5] = {"GET", "POST", "DELETE", "HEAD", "PUT"};
	void(Request::*fonction[])(std::vector<std::string>&) = {&Request::methodeGet, &Request::methodePost, &Request::methodeDelete, &Request::methodeHead, &Request::methodePut};

	int j = -1;
	for (int i = 0; i < 5; i++) {
		if (methode_name[i].compare(m_methode) == 0)
			j = i;
	}
	
	if (j != -1)
		(this->*fonction[j])(request);
	else {
		Logger::log(RED, "methode %s not autorized error ...", m_methode.c_str());
		//send_error_400;
	}
}

Request::Request(const Request &copy) {
	*this = copy;
}

Request& Request::operator=(const Request &other) {
	if (this != &other) {
		m_loc = other.m_loc;
	}
	return *this;
}

void Request::methodePost(std::vector<std::string>& tab) {
	std::cout << "coucou Post\n";

	

	for (std::vector<std::string>::iterator it = tab.begin(); it != tab.end(); ++it)
	{
		
	}
	(void)tab;
}

void Request::methodeGet(std::vector<std::string>& tab) {
	
	for (std::vector<std::string>::iterator it = tab.begin() + 1; it != tab.end(); ++it)
	{
		std::string line = *it;

		if (m_foundBody)
			m_body += line + "\n";
		if (line.empty())
			m_foundBody = true;
		else
		{
			std::vector<std::string> words = split(line, " ");
			if (words[0] == "Content-Length:")
				m_content_length = words[1];
			if (words[0] == "Content-Type:")
				m_content_type = words[1];
		}
	}
}

void Request::methodeDelete(std::vector<std::string>& tab) {
	std::cout << "coucou Delete\n";

	(void)tab;

}

void Request::methodeHead(std::vector<std::string>& tab) {
	std::cout << "coucou Head\n";
	for (std::vector<std::string>::iterator it = tab.begin(); it != tab.end(); ++it)
		std::cout << *it << std::endl;
	(void)tab;
}

void Request::methodePut(std::vector<std::string>& tab) {
	std::cout << "coucou Head\n";
	for (std::vector<std::string>::iterator it = tab.begin(); it != tab.end(); ++it)
		std::cout << *it << std::endl;
	(void)tab;
}

void Request::parseType(std::string& request) {
	(void)request;
}

void Request::parseLenght(std::string& request) {
	(void)request;
}

void Request::parseBody(std::string& request) {
	(void)request;
}

void Request::parseCGI(std::string& request) {
	(void)request;
}


std::ostream& operator<<(std::ostream& o, Request& req)
{
	o << "=================================\n" << "Method: " << req.getMethod() << "\nContent-Length: " << req.getContentLength() << 
	"\nContent-Type: " << req.getContentType() <<
	"\nUrl: " << req.getUrl() << 
	"\nVersion: " << req.getVersion() <<
	"\nBody: " << req.getBody() <<
	"\n------------------\n" << req.getLocation() << "\n----------------------------------------------\n" << "=================================";
	return o;
}