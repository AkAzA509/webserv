
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 12:33:21 by macorso           #+#    #+#             */
/*   Updated: 2025/09/12 12:52:52 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.h"
#include "Logger.h"
#include <functional>

Response::Response() : m_server(NULL), m_request(NULL), m_firstline(), m_header(), m_body(), m_servedFilePath() {}

Response::Response(Request& req, int client_fd, Server& server) : m_server(&server), m_request(&req) {
	bool created = false;
	std::string existingSessionId = req.getCookieValue(server.getSessionCookieName());
	std::string clientIp = server.getClientIp(client_fd);
	SessionData& session = server.ensureSession(existingSessionId, clientIp, created);
	req.setSessionId(session.id);

	CookieData cookie;
	cookie.name = server.getSessionCookieName();
	cookie.value = session.id;
	cookie.path = "/";
	cookie.httpOnly = true;
	cookie.secure = true;
	cookie.expires = (getCurrentTimeMs() + server.getSessionTimeoutMs()) / 1000;

	addCookie(cookie);
	
	buildResponse();
}

void Response::setDefaultResponse() {
	m_firstline = HEADER_OK;
}

std::pair<std::string, std::string> Response::getError(int page, const std::string& page_path) const {
	std::string error_file_path = joinPaths(m_server->getRoot(), page_path);
	
	switch (page)
	{
		case 404:
			return std::make_pair(ERROR_404, loadFile(error_file_path));
		case 403:
			return std::make_pair(ERROR_403, loadFile(error_file_path));
		case 400:
			return std::make_pair(ERROR_400, loadFile(error_file_path));
		case 405:
			return std::make_pair(ERROR_405, loadFile(error_file_path));
		case 408:
			return std::make_pair(ERROR_408, loadFile(error_file_path));
		case 411:
			return std::make_pair(ERROR_411, loadFile(error_file_path));
		case 413:
			return std::make_pair(ERROR_413, loadFile(error_file_path));
		case 500:
			return std::make_pair(ERROR_500, loadFile(error_file_path));
		default:
			return std::make_pair(ERROR_500, loadFile(error_file_path));
	}
}


std::string Response::selectCgiInterpreter(const Location& loc, const std::string& scriptName) {
	size_t dot = scriptName.find_last_of('.');
	std::string ext;

	if (dot != std::string::npos)
		ext = scriptName.substr(dot);
	else
		return "";
	std::vector<std::string> cgi_exts = loc.getCgiExt();
	std::vector<std::string> cgi_passes = loc.getCgiPath();
	std::string cgiPass;

	for (size_t i = 0; i < cgi_exts.size() && i < cgi_passes.size(); ++i) {
		if (cgi_exts[i] == ext) {
			cgiPass = cgi_passes[i];
			break;
		}
	}
	if (cgiPass.empty() && cgi_passes.size() == 1)
		return "";
	if (cgiPass.empty()) {
		setErrorResponse(500);
		m_body = "CGI interpreter not configured for the script";
	}
	return cgiPass;
}

std::string Response::joinPaths(const std::string& base, const std::string& relative) const {
	if (base.empty())
		return relative;
	if (relative.empty())
		return base;
	if (base[base.size() - 1] == '/' && relative[0] == '/')
		return base + relative.substr(1);
	if (base[base.size() - 1] != '/' && relative[0] != '/')
		return base + "/" + relative;
	return base + relative;
}


std::string Response::buildPath(const std::string& page_path) const {
	const Location& location = m_request->getLocation();
	std::string location_path = location.getPath();
	std::string root = location.getRoot().empty() ? m_server->getRoot() : location.getRoot();
	std::string req_path = page_path;
	
	if (!location_path.empty() && location_path != "/") {
		std::string loc_for_match = location_path;
		if (loc_for_match.size() > 1 && loc_for_match[loc_for_match.size() - 1] == '/')
			loc_for_match = loc_for_match.substr(0, loc_for_match.size() - 1);
			
		if (req_path.compare(0, loc_for_match.size(), loc_for_match) == 0)
			req_path = req_path.substr(loc_for_match.size());
	}

	while (!req_path.empty() && req_path[0] == '/')
		req_path = req_path.substr(1);
	
	bool endsWithSlash = (!req_path.empty() && req_path[req_path.size() - 1] == '/');
	if (endsWithSlash)
		req_path = req_path.substr(0, req_path.size() - 1);
	
	if (req_path.empty()) {
		std::vector<std::string> locIndexes = location.getIndexFiles();
		if (!locIndexes.empty())
			req_path = locIndexes[0];
		else {
			std::string uploadPath = location.getUploadPath();
			while (!uploadPath.empty() && uploadPath[0] == '/')
				uploadPath = uploadPath.substr(1);
			req_path = uploadPath.empty() ? "." : uploadPath;
		}
	}
	else {
		std::string uploadPath = location.getUploadPath();
		if (!uploadPath.empty() && req_path.find("cgi-bin/") != 0) {
			while (!uploadPath.empty() && uploadPath[0] == '/')
				uploadPath = uploadPath.substr(1);
			if (!uploadPath.empty())
				req_path = joinPaths(uploadPath, req_path);
		}
	}

	return joinPaths(root, req_path);
}

std::string Response::buildDirPath(const std::string& page_path) const {
	const Location& location = m_request->getLocation();
	std::string location_path = location.getPath();
	std::string root = location.getRoot().empty() ? m_server->getRoot() : location.getRoot();
	std::string req_path = page_path;
	
	if (!location_path.empty() && location_path != "/") {
		std::string loc_for_match = location_path;
		if (loc_for_match.size() > 1 && loc_for_match[loc_for_match.size() - 1] == '/')
			loc_for_match = loc_for_match.substr(0, loc_for_match.size() - 1);
			
		if (req_path.compare(0, loc_for_match.size(), loc_for_match) == 0)
			req_path = req_path.substr(loc_for_match.size());
	}

	while (!req_path.empty() && req_path[0] == '/')
		req_path = req_path.substr(1);
	
	bool endsWithSlash = (!req_path.empty() && req_path[req_path.size() - 1] == '/');
	if (endsWithSlash)
		req_path = req_path.substr(0, req_path.size() - 1);
	
	if (req_path.empty()) {
		std::string uploadPath = location.getUploadPath();
		while (!uploadPath.empty() && uploadPath[0] == '/')
			uploadPath = uploadPath.substr(1);
		req_path = uploadPath.empty() ? "." : uploadPath;
	}
	else {
		std::string uploadPath = location.getUploadPath();
		if (!uploadPath.empty() && req_path.find("cgi-bin/") != 0) {
			while (!uploadPath.empty() && uploadPath[0] == '/')
				uploadPath = uploadPath.substr(1);
			if (!uploadPath.empty())
				req_path = joinPaths(uploadPath, req_path);
		}
	}

	return joinPaths(root, req_path);
}

std::vector<std::string> Response::getLocationOrServerIndexes() const {
	const Location& loc = m_request->getLocation();
	std::vector<std::string> locIndexes = loc.getIndexFiles();

	if (!locIndexes.empty())
		return locIndexes;

	std::vector<std::string> serverIndexes = m_server->getIndexFiles();
	if (!serverIndexes.empty())
		return serverIndexes;

	return std::vector<std::string>(1, "index.html");
}

std::string Response::getFullResponse() const {
	// Construit les headers comme string
	std::string headers = m_firstline;

	std::string contentType;
	if (!m_servedFilePath.empty())
		contentType = getFileType(m_servedFilePath);
	else
		contentType = "text/html";
		
	if (m_firstline == HEADER_303)
		headers += "Location: " + m_request->getLocation().getRedirectionPath() + "\r\n";

	headers += "Content-Type: " + contentType + "\r\n";
	std::ostringstream sizeSs;
	sizeSs << m_body.size();
	headers += "Content-Length: " + sizeSs.str() + "\r\n";
	headers += "Connection: close\r\n";

	for (std::map<std::string, std::string>::const_iterator it = m_header.begin(); it != m_header.end(); ++it)
		headers += it->first + ": " + it->second + "\r\n";

	headers += "\r\n";
	
	// Concatène directement headers + body pour préserver les données binaires
	std::string response;
	response.reserve(headers.size() + m_body.size());
	response.append(headers);
	response.append(m_body);
	
	return response;
}

void Response::handleGet() {
	// const Location& location = m_request->getLocation();
	std::string requestPath = m_request->getPath();
	
	bool endsWithSlash = (requestPath.size() > 1 && requestPath[requestPath.size() - 1] == '/');
	bool forceAutoIndex = (endsWithSlash && m_request->getIsAutoIndex());
	
	if (forceAutoIndex) {
		std::string dirPath = urlDecode(buildDirPath(requestPath));
		m_firstline = HEADER_OK;
		m_request->autoIndex(dirPath, requestPath);
		m_body = m_request->getAutoIndex();
		m_servedFilePath.clear();
		return;
	}
	
	std::string filePath = urlDecode(buildPath(requestPath));

	struct stat st;
	if (stat(filePath.c_str(), &st) == 0) {
		if (S_ISDIR(st.st_mode)) {
			std::vector<std::string> indexes = getLocationOrServerIndexes();
			bool indexFound = false;
			
			for (size_t i = 0; i < indexes.size(); ++i) {
				std::string indexPath = "/" + indexes[i];
				std::string indexFile = buildPath(indexPath);
				struct stat indexSt;
				if (stat(indexFile.c_str(), &indexSt) == 0 && S_ISREG(indexSt.st_mode)) {
					std::string body = loadFile(indexFile);
					if (!body.empty()) {
						m_firstline = HEADER_OK;
						m_body = body;
						m_servedFilePath = indexFile;
						indexFound = true;
						break;
					}
				}
			}
			
			if (!indexFound && m_request->getIsAutoIndex()) {
				m_firstline = HEADER_OK;
				m_request->autoIndex(filePath, m_request->getPath());
				m_body = m_request->getAutoIndex();
				m_servedFilePath.clear();
				return;
			}
			
			if (!indexFound) {
				setErrorResponse(403);
				m_servedFilePath.clear();
				return;
			}
		}
		else if (S_ISREG(st.st_mode)) {
			std::string body = loadFile(filePath);
			if (!body.empty()) {
				m_firstline = HEADER_OK;
				m_body = body;
				m_servedFilePath = filePath;
			}
			else {
				setErrorResponse(500);
				m_servedFilePath.clear();
				return;
			}
		}
		else {
			setErrorResponse(404);
			m_servedFilePath.clear();
			return;
		}
	}
	else {
		setErrorResponse(404);
		m_servedFilePath.clear();
		return;
	}
}

void Response::setErrorResponse(int errorCode) {
	const std::map<int, std::string>& pages = m_server->getErrorPages();
	std::map<int, std::string>::const_iterator it = pages.find(errorCode);

	if (it != pages.end()) {
		std::pair<std::string, std::string> page = getError(errorCode, it->second);
		m_firstline = page.first;
		m_body = page.second;
		m_servedFilePath = it->second;
	}
	else {
		switch (errorCode) {
			case 400:
				m_firstline = ERROR_400;
				m_body = P_ERROR_400;
				break;
			case 403:
				m_firstline = ERROR_403;
				m_body = P_ERROR_403;
				break;
			case 404:
				m_firstline = ERROR_404;
				m_body = P_ERROR_404;
				break;
			case 405:
				m_firstline = ERROR_405;
				m_body = P_ERROR_405;
				break;
			case 408:
				m_firstline = ERROR_408;
				m_body = P_ERROR_408;
				break;
			case 411:
				m_firstline = ERROR_411;
				m_body = P_ERROR_411;
				break;
			case 413:
				m_firstline = ERROR_413;
				m_body = P_ERROR_413;
				break;
			case 500:
				m_firstline = ERROR_500;
				m_body = P_ERROR_500;
				break;
			default:
				m_firstline = ERROR_500;
				m_body = P_ERROR_500;
				break;
		}
		m_servedFilePath.clear();
	}
}

void Response::handlePost() {
	const Location& loc = m_request->getLocation();
	std::string root = loc.getRoot().empty() ? m_server->getRoot() : loc.getRoot();
	std::string uploadPath = loc.getUploadPath();
	const std::vector<BinaryInfo>& binaries = m_request->getBinaryInfos();

	if (!binaries.empty()) {
		bool all_success = true;
		for (size_t i = 0; i < binaries.size(); ++i) {
			const BinaryInfo& bin = binaries[i];
			std::stringstream ss;
			ss << getCurrentTimeMs();
			std::string filename = std::string("File") + "-" + ss.str() + "_" + bin.filename;
			std::string filePath = normalizePath(joinPaths(joinPaths(root, uploadPath), filename));
			std::ofstream outfile(filePath.c_str(), std::ios::binary);

			if (!outfile) {
				all_success = false;
				setErrorResponse(500);
				m_servedFilePath.clear();
				return;
			}
			outfile.write(bin.data.c_str(), bin.data.size());
			outfile.close();
			m_servedFilePath = filePath;
			Logger::log(CYAN, "File uploaded: %s", filePath.c_str());
		}
		if (all_success)
			m_firstline = HEADER_201;
		if (m_request->getLocation().getRedirectionPath() != "")
			m_firstline = HEADER_303;
	}
	else if (!m_request->getRawBody().empty()) {
		std::string filePath = normalizePath(joinPaths(joinPaths(root, uploadPath), "post_body.txt"));
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
		if (m_request->getLocation().getRedirectionPath() != "")
			m_firstline = HEADER_303;
		m_body = "Body posted successfully.";
	}
	else {
		setErrorResponse(400);
		m_servedFilePath.clear();
	}
}

void Response::handleDelete() {
	const Location& loc = m_request->getLocation();
	std::vector<std::string> indexes = loc.getIndexFiles();
	if (indexes.empty()) {
		setErrorResponse(500);
		return;
	}
	std::string scriptName = indexes[0];

	std::string cgiPass = selectCgiInterpreter(loc, scriptName);
	if (cgiPass.empty())
		return;

	std::string root = loc.getRoot().empty() ? m_server->getRoot() : loc.getRoot();
	std::string scriptPath = normalizePath(joinPaths(root, scriptName));
	
	std::string mappedPath = buildPath(urlDecode(m_request->getPath()));
	std::string filePath = mappedPath;
	if (filePath.size() >= 2 && filePath[0] == '.' && filePath[1] == '/') {
		filePath = filePath.substr(1);
	}
	
	std::vector<std::string> args;
	args.push_back(scriptPath);
	args.push_back(filePath);

	std::vector<std::string> extraEnv = prepareCgiEnv(*m_request, scriptName, filePath);
	std::string cgiOutput;

	bool success = m_request->doCGI(cgiPass, args, extraEnv, cgiOutput);
	if (success && cgiOutput.find("success") != std::string::npos) {
		Logger::log(CYAN, "File deleted: %s", filePath.c_str());
		m_firstline = HEADER_OK;
		m_body = cgiOutput;
		if (!m_request->getLocation().getRedirectionPath().empty())
			m_firstline = HEADER_303;
	}
	else {
		Logger::log(RED, "Failed to delete file: %s", filePath.c_str());
		setErrorResponse(500);
		m_body = cgiOutput.empty() ? "Error deleting file." : cgiOutput;
	}
}

void Response::handlePut() {
	const Location& loc = m_request->getLocation();
	std::string uploadPath = loc.getUploadPath();
	std::string root = loc.getRoot().empty() ? m_server->getRoot() : loc.getRoot();
	const std::vector<BinaryInfo>& binaries = m_request->getBinaryInfos();

	if (!binaries.empty()) {
		bool all_success = true;
		for (size_t i = 0; i < binaries.size(); ++i) {
			const BinaryInfo& bin = binaries[i];
			std::string filePath = normalizePath(joinPaths(joinPaths(root, uploadPath), bin.filename));
			std::ofstream outfile(filePath.c_str(), std::ios::binary);

			if (!outfile) {
				all_success = false;
				setErrorResponse(500);
				return;
			}
			outfile.write(bin.data.c_str(), bin.data.size());
			outfile.close();
			Logger::log(CYAN, "File uploaded: %s", filePath.c_str());
		}
		if (all_success)
			m_firstline = HEADER_201;
		if (m_request->getLocation().getRedirectionPath() != "")
			m_firstline = HEADER_303;
	}
	else
		setErrorResponse(400);
}

void Response::prepareCgi() {
	const Location& loc = m_request->getLocation();
	std::string reqPath = m_request->getPath();

	std::string scriptName;
	size_t lastSlash = reqPath.find_last_of("/");
	scriptName = (lastSlash != std::string::npos) ? reqPath.substr(lastSlash + 1) : reqPath;

	if (scriptName.empty()) {
		setErrorResponse(404);
		return;
	}

	std::string cgiPass = selectCgiInterpreter(loc, scriptName);
	if (cgiPass.empty()) {
		setErrorResponse(404);
		return;
	}

	std::string scriptPath = normalizePath(buildPath(reqPath));
	std::vector<std::string> args;
	args.push_back(scriptPath);

	std::vector<std::string> extraEnv = prepareCgiEnv(*m_request, scriptName, reqPath);

	std::string cgiOutput;
	bool success = m_request->doCGI(cgiPass, args, extraEnv, cgiOutput);

	if (success) {
		Logger::log(LIGHTMAGENTA, "CGI executed successfully", scriptName.c_str(), cgiPass.c_str());
		m_firstline = HEADER_OK;
		m_body = cgiOutput;
	}
	else {
		m_body = cgiOutput.empty() ? "CGI execution failed" : cgiOutput;
		setErrorResponse(500);
	}
}

void Response::buildResponse() {
	std::string method = m_request->getMethod();
	setDefaultResponse();

	for (size_t i = 0; i < m_setCookies.size(); ++i)
	{
		const CookieData& c = m_setCookies[i];

		std::string setCookie = c.name + "=" + c.value;
		
		if (!c.path.empty())
			setCookie += "; Path=" + c.path;
		if (!c.domain.empty())
			setCookie += "; Domain=" + c.domain;
		if (c.expires > 0)
		{
			char buffer[100];
			time_t t = static_cast<time_t>(c.expires);
			struct tm* gmt = std::gmtime(&t);
			std::strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", gmt);
			setCookie += "; Expires=" + std::string(buffer);
		}
		if (c.httpOnly)
			setCookie += "; HttpOnly";
		if (c.secure)
			setCookie += "; Secure";

		m_header.insert(std::make_pair("Set-Cookie", setCookie));
	}

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
		if (!m_request->getLocation().isAllowedMethod(m_request->getMethod()))
			setErrorResponse(405);
		else if (m_request->getPath().find("cgi-bin") != std::string::npos && m_request->getMethod() != "GET")
			prepareCgi();
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
}

std::ostream& operator<<(std::ostream& o, const Response& r)
{
	o << "Response {\n";

	o << r.getFullResponse();

	o << "\r\n}";
	return o;
}