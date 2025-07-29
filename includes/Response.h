/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/24 10:43:15 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/28 14:42:36 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include "Server.h"
#include "Request.h"

class Response {
private:
	std::string m_response;
	Request m_req;
	Server m_serv;
public:
	std::string getResponse();
	void isErrorPage(std::string& error);
	void fillResponse(std::string type, std::string& file, std::string head);
	void methodeWithBodyResponse();
	void methodeWithinBodyResponse();
public:
	Response() {};
	Response(Request req, Server serv);
	Response(std::string& error, Server serv);
	~Response() {};
	Response(const Response& copy);
	Response& operator=(const Response& other);
};
