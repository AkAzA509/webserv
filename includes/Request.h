/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 12:03:36 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/22 13:18:26 by ggirault         ###   ########.fr       */
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
	std::string m_content_lenght;
	std::string m_body;
public:
	Request() {};
	Request(Location loc, std::string& request, std::vector<std::string>& tab);
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
	void methodePost(std::vector<std::string>& tab);
	void methodeGet(std::vector<std::string>& tab);
	void methodeDelete(std::vector<std::string>& tab);
	void methodeHead(std::vector<std::string>& tab);
};
