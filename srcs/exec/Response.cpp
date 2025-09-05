/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 12:33:21 by macorso           #+#    #+#             */
/*   Updated: 2025/09/04 16:09:29 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.h"
#include "Logger.h"

Response::Response() : m_server(NULL), m_request(NULL), m_firstline(), m_header(), m_body(), m_servedFilePath() {}

Response::Response(Request& req, Server& server) : m_server(&server), m_request(&req)
{
	buildResponse();
}

void Response::setDefaultResponse()
{
	m_firstline = HEADER_OK;
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

std::string Response::joinPaths(const std::string& base, const std::string& relative) const
{
	if (base.empty()) return relative;
	if (relative.empty()) return base;
	if (base[base.size() - 1] == '/' && relative[0] == '/')
		return base + relative.substr(1);
	if (base[base.size() - 1] != '/' && relative[0] != '/')
		return base + "/" + relative;
	return base + relative;
}


std::string Response::buildPath(const std::string& page_path) const
{
	std::string root = m_server->getRoot(); // Use config root directly
	std::string req_path = page_path;

	// Remove leading slashes from request path
	while (!req_path.empty() && req_path[0] == '/')
		req_path = req_path.substr(1);

	// If request is empty, use index.html or configured index
	if (req_path.empty()) {
		std::vector<std::string> indexes = getLocationOrServerIndexes();
		req_path = indexes.empty() ? "index.html" : indexes[0];
	}

	std::string full_path = joinPaths(root, req_path);
	// Logger::log(YELLOW, "root: %s, req_path: %s, full_path: %s\n", root.c_str(), req_path.c_str(), full_path.c_str());
	return full_path;
}

std::vector<std::string> Response::getLocationOrServerIndexes() const
{
	const Location& loc = m_request->getLocation();
	std::vector<std::string> locIndexes = loc.getIndexFiles();
	if (!locIndexes.empty())
		return locIndexes;

	std::vector<std::string> serverIndexes = m_server->getIndexFiles();
	if (!serverIndexes.empty())
		return serverIndexes;

	return std::vector<std::string>(1, "index.html");
}

std::string Response::getFullResponse() const
{
	std::ostringstream oss;
	oss << m_firstline;

	std::string contentType;
	if (!m_servedFilePath.empty())
		contentType = getMimeType(m_servedFilePath);
	else
		contentType = "text/html";

	oss << "Content-Type: " << contentType << "\r\n";

	oss << "Content-Length: " << m_body.size() << "\r\n";

	oss << "Server: Webserv/1.0\r\n";
	oss << "Connection: close\r\n";

	for (std::map<std::string, std::string>::const_iterator it = m_header.begin(); it != m_header.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";

	oss << "\r\n";
	oss << m_body;

	return oss.str();
}

void Response::handleGet()
{
	// Logger::log(YELLOW, "path : %s\n", m_request->getPath().c_str());
	std::string filePath = buildPath(m_request->getPath());
	Logger::log(YELLOW, "path builder : %s\n", filePath.c_str());

	struct stat st;
	if (stat(filePath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
		if (m_request->getIsAutoIndex()) {
			m_firstline = HEADER_OK;
			m_body = m_request->getAutoIndex();
			m_servedFilePath.clear();
			return;
		}
		std::vector<std::string> indexes = getLocationOrServerIndexes();
		for (size_t i = 0; i < indexes.size(); ++i) {
			std::string indexFile = joinPaths(filePath, indexes[i]);
			std::string body = loadFile(indexFile);
			if (!body.empty()) {
				m_firstline = HEADER_OK;
				m_body = body;
				m_servedFilePath = indexFile;
				return;
			}
		}
		setErrorResponse(404);
		m_servedFilePath.clear();
		return;
	}

	std::string body = loadFile(filePath);

	if (!body.empty()) {
		m_firstline = HEADER_OK;
		m_body = body;
		m_servedFilePath = filePath;
	} else {
		setErrorResponse(404);
		m_servedFilePath.clear();
	}
}

void Response::setErrorResponse(int errorCode)
{
	const std::map<int, std::string>& pages = m_server->getErrorPages();
	std::map<int, std::string>::const_iterator it = pages.find(errorCode);

	if (it != pages.end()) {
		std::pair<std::string, std::string> page = getError(errorCode, it->second);
		m_firstline = page.first;
		m_body = page.second;
		m_servedFilePath = it->second;
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
		m_servedFilePath.clear();
	}
}

void Response::handlePost()
{
	std::string root = m_request->getLocation().getRoot();
	std::string uploadPath = m_request->getLocation().getUploadPath();
	const std::vector<BinaryInfo>& binaries = m_request->getBinaryInfos();

	if (!binaries.empty()) {
		bool all_success = true;
		for (size_t i = 0; i < binaries.size(); ++i) {
			const BinaryInfo& bin = binaries[i];
			std::string filePath = normalizePath(root + "/" + uploadPath + "/" + bin.filename);

			std::ofstream outfile(filePath.c_str(), std::ios::binary);
			if (!outfile) {
				all_success = false;
				Logger::log(RED, "Failed to open file for writing: %s", filePath.c_str());
				setErrorResponse(500);
				m_servedFilePath.clear();
				return;
			}
			outfile.write(bin.data.c_str(), bin.data.size());
			outfile.close();
			m_servedFilePath = filePath;
		}
		if (all_success) {
			m_firstline = HEADER_201;
			m_body = "File(s) uploaded successfully.";
		}
	}
	else if (!m_request->getRawBody().empty()) {
		std::string filePath = normalizePath(root + "/" + uploadPath + "/post_body.txt");
		std::ofstream outfile(filePath.c_str(), std::ios::binary);
		if (!outfile) {
			setErrorResponse(500);
			m_servedFilePath.clear();
			return;
		}
		const std::string& body = m_request->getRawBody();
		outfile.write(body.c_str(), body.size());
		outfile.close();
		m_servedFilePath = filePath;
		m_firstline = HEADER_201;
		m_body = "Body posted successfully.";
	}
	else {
		setErrorResponse(400);
		m_servedFilePath.clear();
	}
}

void Response::handleDelete()
{
	std::string filePath = buildPath(m_request->getPath());
	std::vector<std::string> cgiPass = m_request->getLocation().getCgiPath();
	if (cgiPass.empty()) {
		setErrorResponse(500);
		m_body = "CGI script for DELETE not configured.";
		return;
	}
	std::string cgiScript = cgiPass[0];
	std::string cgiOutput;
	std::vector<std::string> args;
	args.push_back(filePath);
	std::vector<std::string> extraEnv;
	extraEnv.push_back("REQUEST_METHOD=DELETE");
	extraEnv.push_back("PATH_INFO=" + filePath);
	bool success = m_request->doCGI(cgiScript, args, extraEnv, cgiOutput);
	if (success && cgiOutput.find("success") != std::string::npos) {
		m_firstline = HEADER_OK;
		m_body = cgiOutput;
	} else {
		setErrorResponse(500);
		m_body = cgiOutput.empty() ? "Error deleting file." : cgiOutput;
	}
}

void Response::handlePut()
{
	std::string root = m_request->getLocation().getRoot();
	std::string fullPath = normalizePath(root + m_request->getPath());

	const std::vector<BinaryInfo>& binaries = m_request->getBinaryInfos();

	if (!binaries.empty()) {
		const BinaryInfo& bin = binaries[0];
		std::ofstream outfile(fullPath.c_str(), std::ios::binary);
		if (!outfile) {
			setErrorResponse(500);
			return;
		}
		outfile.write(bin.data.c_str(), bin.data.size());
		outfile.close();
		m_firstline = HEADER_201;
	}
	else if (!m_request->getRawBody().empty()) {
		const std::string& body = m_request->getRawBody();
		std::ofstream outfile(fullPath.c_str(), std::ios::binary);
		if (!outfile) {
			setErrorResponse(500);
			return;
		}
		outfile.write(body.c_str(), body.size());
		outfile.close();
		m_firstline = HEADER_201;
	}
	else
		setErrorResponse(400);
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
			Logger::log(RED, "Method %s not allowed for path %s", m_request->getMethod().c_str(), m_request->getPath().c_str());
			setErrorResponse(405);
		}
		else if (m_request->getMethod().find("cgi-bin") != std::string::npos)
			// m_request->doCGI(a changer ca)
			;
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
	// Logger::log(RED, "Body: %s\n", m_body.c_str());
}