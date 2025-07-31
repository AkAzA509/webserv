/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/24 10:43:15 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/30 20:04:47 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <ctime>
#include <sstream>
#include "Server.h"
#include "Request.h"

class Response {
private:
	std::string m_response;
	Request m_req;
	Server m_serv;
	std::map<std::string, std::string> m_cookies_to_set; // New member for cookies to send
public:
	std::string getResponse() const;
	void isErrorPage(std::string& error);
	void fillResponse(std::string type, std::string& file, std::string head);
	void methodeWithBodyResponse();
	void methodeWithinBodyResponse();
	
	// Cookie methods
	void setCookie(const std::string& name, const std::string& value, 
	               const std::string& path = "/", int max_age = -1);
	void addCookieHeader();
	std::string getCurrentTimestamp();
	
public:
	Response() {};
	Response(Request req, Server serv);
	Response(std::string& error, Server serv);
	~Response() {};
	Response(const Response& copy);
	Response& operator=(const Response& other);

};

std::ostream& operator<<(std::ostream& o, const Response& a);
