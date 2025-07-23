/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 12:03:36 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/23 18:59:04 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include "Server.h"

class Request {
private:
	Location m_loc;
	std::string m_methode;
	std::string m_url;
	std::string m_version;
	std::string m_content_type;
	std::string m_content_length;
	std::string m_body;
	bool m_foundBody;
public:
	Request() {};
	Request(Location loc, std::vector<std::string>& firstRequestLine, std::vector<std::string>& request, std::string& full_request);
	~Request() {};
	Request(const Request& copy);
	Request& operator=(const Request& other);
public:
	void parseRequest(std::string& request);
	void parseType(std::string& request);
	void parseLenght(std::string& request);
	void parseBody(std::string& request);
	void parseCGI(std::string& request);
public:
	void methodePost(std::vector<std::string>& tab, std::string& request);
	void methodeGet(std::vector<std::string>& tab, std::string& request);
	void methodeDelete(std::vector<std::string>& tab, std::string& request);
	void methodePut(std::vector<std::string>& tab, std::string& request);
	void methodeHead(std::vector<std::string>& tab, std::string& request);
public:
	std::string getBody() { return m_body; }
	std::string getMethod() { return m_methode; }
	std::string getUrl() { return m_url; }
	std::string getVersion() { return m_version; }
	std::string getContentType() { return m_content_type; }
	std::string getContentLength() { return m_content_length; }
	Location getLocation() { return m_loc; }
};


std::ostream& operator<<(std::ostream& o, Request& req);
