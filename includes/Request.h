/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 12:03:36 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/30 16:08:55 by ggirault         ###   ########.fr       */
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
	bool m_errorPage;
	std::string m_responseStatus;
	char **m_env;
public:
	Request() {};
	Request(Location loc, std::vector<std::string>& firstRequestLine, std::vector<std::string>& request, std::string& full_request, char **env);
	~Request() {};
	Request(const Request& copy);
	Request& operator=(const Request& other);
public:
	void parseRequest(std::string& request);
	void writeFile(std::string& filename, std::string& file_data);
public:
	void methodePost(std::vector<std::string>& tab, std::string& request);
	void methodeGet(std::vector<std::string>& tab, std::string& request);
	void methodeDelete(std::vector<std::string>& tab, std::string& request);
	void methodePut(std::vector<std::string>& tab, std::string& request);
	void doCGI(size_t end_header, std::string& request);
	std::vector<std::string>& convertEnv();
public:
	std::string getBody() { return m_body; }
	std::string getMethod() { return m_methode; }
	std::string getUrl() { return m_url; }
	std::string getVersion() { return m_version; }
	std::string getContentType() { return m_content_type; }
	std::string getContentLength() { return m_content_length; }
	std::string getResponseStatus() { return m_responseStatus; }
	bool getIsFoundBody() { return m_foundBody; }
	bool getErrorPage() { return m_errorPage; }
	Location getLocation() { return m_loc; }
};


std::ostream& operator<<(std::ostream& o, Request& req);
