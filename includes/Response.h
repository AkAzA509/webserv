/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/24 10:43:15 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/24 15:33:14 by ggirault         ###   ########.fr       */
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
	void buildResponse();
	void isErrorPage();
	void fillResponse(std::string type, std::string& file, std::string head);
public:
	Response() {};
	Response(Request req, Server serv);
	~Response() {};
	Response(const Response& copy);
	Response& operator=(const Response& other);
};
