/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 12:33:21 by macorso           #+#    #+#             */
/*   Updated: 2025/08/06 18:43:18 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.h"
#include "Logger.h"

Response::Response() : m_server(NULL), m_request(NULL) {}

Response::Response(Request& req, Server& server) : m_server(&server), m_request(&req)
{
	buildResponse();
}

void Response::setDefaultResponse()
{
	m_header["Server"] = "Webserv/1.0";
}

std::pair<std::string, std::string> Response::getError(int page, const std::string& page_path) const
{
	switch (page)
	{
	case 404:
		return std::make_pair(ERROR_404, loadFile(buildPath(page_path)));
	case 403:
		return std::make_pair(ERROR_403, loadFile(buildPath(page_path)));
	case 400:
		return std::make_pair(ERROR_400, loadFile(buildPath(page_path)));
	case 405:
		return std::make_pair(ERROR_405, loadFile(buildPath(page_path)));
	case 411:
		return std::make_pair(ERROR_411, loadFile(buildPath(page_path)));
	case 500:
		return std::make_pair(ERROR_500, loadFile(buildPath(page_path)));
	default:
		return std::make_pair(ERROR_500, loadFile(buildPath(page_path)));
	}
}

std::string Response::normalizePath(const std::string& path) const
{
	std::vector<std::string> stack;
	std::istringstream ss(path);
	std::string token;
	bool isAbsolute = !path.empty() && path[0] == '/';

	while (std::getline(ss, token, '/')) {
		if (token == "" || token == ".") continue;
		if (token == "..") {
			if (!stack.empty()) stack.pop_back();
		} else {
			stack.push_back(token);
		}
	}

	std::string normalized;
	if (isAbsolute) normalized += "/";
	for (size_t i = 0; i < stack.size(); ++i) {
		normalized += stack[i];
		if (i + 1 < stack.size()) normalized += "/";
	}

	return normalized.empty() ? (isAbsolute ? "/" : ".") : normalized;
}

std::string Response::buildPath(const std::string& page_path) const
{
	std::string root = normalizePath(m_server->getRoot());
	std::string page_path_normalized = normalizePath(page_path);
	std::string full_path;

	if (page_path_normalized.size() >= 1 && page_path_normalized[0] == '/')
		full_path = root + page_path_normalized;
	else
		full_path = root + "/" + page_path_normalized;
	return full_path;
}

void Response::setErrorResponse(int errorCode)
{
	const std::map<int, std::string>& pages = m_server->getErrorPages();
	std::map<int, std::string>::const_iterator it = pages.find(errorCode);

	if (it != pages.end()) {
		std::pair<std::string, std::string> page = getError(errorCode, it->second);
		m_firstline = page.first;
		m_body = page.second;
	} else {
		switch (errorCode) {
			case 400: m_firstline = ERROR_400; m_body = P_ERROR_400; break;
			case 403: m_firstline = ERROR_403; m_body = P_ERROR_403; break;
			case 404: m_firstline = ERROR_404; m_body = P_ERROR_404; break;
			case 405: m_firstline = ERROR_405; m_body = P_ERROR_405; break;
			case 411: m_firstline = ERROR_411; m_body = P_ERROR_411; break;
			case 500: m_firstline = ERROR_500; m_body = P_ERROR_500; break;
			default: m_firstline = ERROR_500; m_body = P_ERROR_500; break;
		}
	}
}

std::string Response::getFullResponse() const
{
	std::string header;

	for (std::map<std::string, std::string>::const_iterator it = m_header.begin(); it != m_header.end(); ++it)
		header += it->first + ": " + it->second + "\r\n";
	return header + m_body;
}

void Response::handleGet()
{
	std::string filePath = buildPath(m_request->getPath());
	std::string body = loadFile(filePath);

	if (!body.empty()) {
		m_firstline = HEADER_OK;
		m_body = body;
	} else {
		setErrorResponse(404);
	}
}

void Response::handlePost()
{
	// Placeholder for POST handling logic
	m_firstline = HEADER_OK;
	m_body = "POST request processed";
}

void Response::handleDelete()
{
	// Placeholder for DELETE handling logic
	m_firstline = HEADER_OK;
	m_body = "DELETE request processed";
}

void Response::handlePut()
{
	// Placeholder for DELETE handling logic
	m_firstline = HEADER_OK;
	m_body = "PUT request processed";
}

void Response::buildResponse()
{
	std::string method = m_request->getMethod();
	setDefaultResponse();

	if (m_request->IsErrorPage()) {
		const std::map<int, std::string>& pages = m_server->getErrorPages();
		std::map<int, std::string>::const_iterator it = pages.find(m_request->getErrorPage());

		if (it != pages.end()) {
			std::pair<std::string, std::string> page = getError(it->first, it->second);
			m_firstline = page.first;
			m_body = page.second;
		}
		else {
			m_firstline = ERROR_500;
			m_body = P_ERROR_500;
		}
	}
	else {
		if (!m_request->getLocation().isAllowedMethod(m_request->getMethod())) {
			std::cout << "Ca va la" << std::endl;
			setErrorResponse(405);
		}
		if (m_request->getMethod().find("cgi-bin") != std::string::npos)
			m_request->doCGI(a changer ca);
		else {
			if (method == "GET")
				handleGet();
			else if (method == "POST")
				handlePost();
			else if (method == "DELETE")
				handleDelete();
			else if (method == "PUT")
				handlePut();
			else
				setErrorResponse(500);
		}
	}
	Logger::log(RED, "Body: %s\n", m_body.c_str());
}