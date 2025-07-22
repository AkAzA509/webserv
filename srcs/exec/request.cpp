/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 10:13:39 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/22 13:12:57 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.h"
#include "Logger.h"

Request::Request(Location loc, std::string& request, std::vector<std::string>& tab) : m_loc(loc), m_methode(tab[0]), m_url(tab[1]) {
	std::string methode_name[4] = {"GET", "POST", "DELETE", "HEAD"};
	void(Request::*fonction[])(std::vector<std::string>&) = {&Request::methodeGet, &Request::methodePost, &Request::methodeDelete, &Request::methodeHead};
	int j = -1;
	for (int i = 0; i < 4; i++) {
		if (methode_name[i].compare(m_methode) == 0)
			j = i;
	}
	if (j != -1)
		(this->*fonction[j])(tab);
	else {
		Logger::log(RED, "methode %s not autorized error ...", m_methode.c_str());
		//send_error_400;
	}
	(void)request;
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
	(void)tab;
}

void Request::methodeGet(std::vector<std::string>& tab) {
	std::cout << "coucou Get\n";
	(void)tab;
}

void Request::methodeDelete(std::vector<std::string>& tab) {
	std::cout << "coucou Delete\n";
	(void)tab;
}

void Request::methodeHead(std::vector<std::string>& tab) {
	std::cout << "coucou Head\n";
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
