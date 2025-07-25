/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/24 10:42:54 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/25 13:09:41 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.h"

Response::Response(Request req, Server serv) : m_req(req), m_serv(serv) {
	std::string status = m_req.getResponseStatus();
	if (status != HEADER_OK)
		isErrorPage(status);
	else
		buildResponse();
}

Response::Response(std::string& status, Server serv) : m_serv(serv) {
	if (!status.empty())
		isErrorPage(status);
}

Response::Response(const Response &copy) {
	*this = copy;
}

Response& Response::operator=(const Response &other) {
	if (this != &other) {
		m_req = other.m_req;
		m_response = other.m_response;
		m_serv = other.m_serv;
	}
	return *this;
}

std::string Response::getResponse() {
	return m_response;
}

void Response::isErrorPage(std::string& error_code) {
	std::string page[6] = {"file/error_pages/400.html", "file/error_pages/403.html", "file/error_pages/404.html",
		"file/error_pages/405.html", "file/error_pages/411.html", "file/error_pages/500.html"};
	std::string error[6] = {ERROR_400, ERROR_403, ERROR_404, ERROR_405, ERROR_411, ERROR_500};

	int i = 0;
	for (; i < 6; ++i) {
		if (error[i].compare(error_code) == 0)
			break;
	}

	std::string file = loadFile(page[i]);
	fillResponse(HTML, file, error_code);
}

void Response::buildResponse() {
	std::string type;
	std::string file;

	if (m_req.getUrl() == "/") {
		std::string url = m_serv.getRoot() + m_serv.getIndexFile(0);
		if (url.find(".html"))
			type = HTML;
		else if (url.find(".css"))
			type = CSS;
		else if (url.find(".js"))
			type = JS;
		file = loadFile(url);
	}
	else {
	}
	fillResponse(type, file, m_req.getResponseStatus());
}

void Response::fillResponse(std::string type, std::string& file, std::string head) {
	std::ostringstream oss;
	oss << file.size();
	std::string size = oss.str();
	m_response = head;
	m_response += CONTENT_TYPE + type + RETURN;
	m_response += CONTENT_LENGHT + size + RETURN;
	m_response += CONNECTION_CLOSE;
	m_response += RETURN;
	m_response += file;
}