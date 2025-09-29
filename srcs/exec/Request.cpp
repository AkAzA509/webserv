/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 17:43:51 by ggirault          #+#    #+#             */
/*   Updated: 2025/09/29 15:44:13 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.h"
#include "Parser.h"
#include "Logger.h"
#include <errno.h>
#include <signal.h>

Request::Request() : m_iserrorPage(false), m_error_page(0), m_env(NULL), m_sessionId() {}

std::ostream& operator<<(std::ostream& o, const BinaryInfo& info) {
	o << info.filename << " " << info.data;
	return o;
}

Request::Request(Server& server, const std::string& request, char **env) : m_iserrorPage(false), m_error_page(0), m_env(env), m_sessionId() {
	parseHeader(request, server);
	if (!m_iserrorPage)
		parseBody(request);
}

Request::Request(const Request &copy) {
	*this = copy;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		m_loc = other.m_loc;
		m_method = other.m_method;
		m_path = other.m_path;
		m_httpVersion = other.m_httpVersion;
		m_iserrorPage = other.m_iserrorPage;
		m_boundary = other.m_boundary;
		m_error_page = other.m_error_page;
		m_headers = other.m_headers;
		m_BinaryInfos = other.m_BinaryInfos;
		m_env = other.m_env;
		m_cgiOutput = other.m_cgiOutput;
		m_autoIndexPage = other.m_autoIndexPage;
		m_isAutoIndex = other.m_isAutoIndex;
		m_rawBody = other.m_rawBody;
		m_cookies = other.m_cookies;
		m_sessionId = other.m_sessionId;
	}
	return *this;
}

bool iequals(const std::string& a, const std::string& b) {
	if (a.length() != b.length())
		return false;

	for (size_t i = 0; i < a.length(); ++i) {
		if (std::tolower(a[i]) != std::tolower(b[i]))
			return false;
	}
	return true;
}

void Request::parseContentDispo(const std::string& line, BinaryInfo& binInfo) {
	std::vector<std::string> args = split(line, ";");

	for (size_t i = 1; i < args.size(); i++) {
		std::string arg = trim(args[i]);
		size_t eq_pos = arg.find('=');
		if (eq_pos == std::string::npos)
			continue;

		std::string key = trim(arg.substr(0, eq_pos));
		std::string value = trim(arg.substr(eq_pos + 1));
		
		if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"')
			value = value.substr(1, value.size() - 2);

		if (iequals(key, "name"))
			binInfo.field_name = value;
		else if (iequals(key, "filename"))
			binInfo.filename = value;
	}
}

void Request::parseBinaryInfos(const std::string& body) {
	const std::string boundary = "--" + m_boundary;
	const std::string end_boundary = boundary + "--";

	size_t start_pos = body.find(boundary);
	if (start_pos == std::string::npos) {
		setError(E_ERROR_404);
		return;
	}
	start_pos += boundary.length();

	while (start_pos < body.size()) {
		size_t end_pos = body.find(boundary, start_pos);
		if (end_pos == std::string::npos) {
			end_pos = body.find(end_boundary, start_pos);
			if (end_pos == std::string::npos)
				break;
		}

		std::string part = body.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos + boundary.length();

		if (part.substr(0, 2) == "\r\n")
			part = part.substr(2);

		size_t header_end = part.find("\r\n\r\n");
		if (header_end == std::string::npos)
			continue;

		BinaryInfo binInfo;
		std::string headers_block = part.substr(0, header_end);
		binInfo.data = part.substr(header_end + 4);

		std::vector<std::string> headers = splitset(headers_block, "\r\n");
		for (size_t i = 0; i < headers.size(); i++) {
			if (iequals(headers[i].substr(0, 20), "Content-Disposition:"))
				parseContentDispo(headers[i], binInfo);
		}

		if (binInfo.data.size() >= 2 && binInfo.data.substr(binInfo.data.size() - 2) == "\r\n")
			binInfo.data.resize(binInfo.data.size() - 2);

		m_BinaryInfos.push_back(binInfo);
	}
}

void Request::parseBody(const std::string& request) {
	size_t header_end = request.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		setError(E_ERROR_400);
		return;
	}

	std::string body = request.substr(header_end + 4);
	std::string c_type = getHeader("content-type");

	if (c_type.find("multipart/form-data") != std::string::npos) {
		size_t boundary_pos = c_type.find("boundary=");
		if (boundary_pos != std::string::npos) {
			m_boundary = trim(c_type.substr(boundary_pos + 9));
			if (!m_boundary.empty())
				parseBinaryInfos(body);
		}
	}
	else
		m_rawBody = body;
}

void Request::parseCookies()
{
	std::string cookieHeader = getHeader("Cookie");

	if (cookieHeader.empty()) return;

	std::stringstream ss(cookieHeader);
	std::string pair;

	while (std::getline(ss, pair, ';'))
	{
		size_t eq = pair.find('=');

		if (eq != std::string::npos)
		{
			CookieData cookie;
			cookie.name = pair.substr(0, eq);
			cookie.value = pair.substr(eq + 1);

			cookie.name.erase(0, cookie.name.find_first_not_of(" \t"));
			cookie.value.erase(0, cookie.value.find_first_not_of(" \t"));
			size_t lastName = cookie.name.find_last_not_of(" \t");
			if (lastName != std::string::npos)
				cookie.name.erase(lastName + 1);
			else
				cookie.name.clear();
			size_t lastValue = cookie.value.find_last_not_of(" \t");
			if (lastValue != std::string::npos)
				cookie.value.erase(lastValue + 1);
			else
				cookie.value.clear();

			m_cookies.push_back(cookie);
		}
	}
}

std::string Request::getCookieValue(const std::string& name) const {
	for (size_t i = 0; i < m_cookies.size(); ++i) {
		if (m_cookies[i].name == name)
			return m_cookies[i].value;
	}
	return std::string();
}

void Request::parseHeader(const std::string& request, const Server& server) {
	size_t header_end = request.find("\r\n\r\n");

	if (header_end == std::string::npos) {
		setError(E_ERROR_400);
		return;
	}

	std::string header_block = request.substr(0, header_end);
	std::vector<std::string> headers = splitset(header_block, "\r\n");
	if (headers.empty()) {
		setError(E_ERROR_400);
		return;
	}

	std::vector<std::string> req_line = splitset(headers[0], " ");
	if (req_line.size() != 3) {
		setError(E_ERROR_400);
		return;
	}

	for (size_t i = 1; i < headers.size(); i++) {
		size_t colon = headers[i].find(':');
		if (colon == std::string::npos)
			continue;

		std::string key = trim(headers[i].substr(0, colon));
		std::string value = trim(headers[i].substr(colon + 1));
		
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		m_headers[key] = value;
	}

	parseCookies();

	m_method = req_line[0];
	m_path = req_line[1];

	if (m_path.empty() || m_path[0] != '/')
		m_path = "/" + m_path;

	m_httpVersion = req_line[2];
	std::transform(m_method.begin(), m_method.end(), m_method.begin(), ::toupper);

	const Location* best_match = NULL;
	size_t max_len = 0;
	std::vector<Location> locations = server.getLocations();

	for (size_t i = 0; i < locations.size(); i++) {
		const std::string& loc_path = locations[i].getPath();
		
		std::string normalized_loc = loc_path;
		std::string normalized_path = m_path;

		if (normalized_loc.size() > 1 && normalized_loc[normalized_loc.size() - 1] == '/') {
			normalized_loc = normalized_loc.substr(0, normalized_loc.size() - 1);
		}
		
		if (normalized_path != "/" && normalized_path[normalized_path.size() - 1] != '/') {
			normalized_path += "/";
		}
		
		// Check si le path commence par la location OU si c'est un match exact
		bool matches = false;
		if (normalized_path.compare(0, normalized_loc.size(), normalized_loc) == 0) {
			// Vérifie que c'est un vrai préfixe (suivi de '/' ou fin de chaîne)
			if (normalized_path.size() == normalized_loc.size() || 
				normalized_path[normalized_loc.size()] == '/') {
				matches = true;
			}
		}
		
		if (matches && normalized_loc.size() > max_len) {
			best_match = &locations[i];
			max_len = normalized_loc.size();
			break;
		}
	}

	if (!best_match) {
		// Cherche la location racine "/"
		for (size_t i = 0; i < locations.size(); i++) {
			if (locations[i].getPath() == "/") {
				best_match = &locations[i];
				break;
			}
		}
		
		// Si même la location "/" n'existe pas, erreur 404
		if (!best_match) {
			setError(E_ERROR_404);
			return;
		}
	}

	m_loc = *best_match;
	m_isAutoIndex = false;

	if (m_loc.isAutoIndexOn())
		m_isAutoIndex = true;

	
}

std::string Request::getHeader(const std::string& key) const {
	std::string lkey = key;
	std::transform(lkey.begin(), lkey.end(), lkey.begin(), ::tolower);
	
	std::map<std::string, std::string>::const_iterator it = m_headers.find(lkey);
	return (it != m_headers.end()) ? it->second : "";
}

void Request::setError(int error_code) {
	m_error_page = error_code;
	m_iserrorPage = true;
}



std::vector<std::string> Request::convertEnv() {
	std::vector<std::string> env;
	for (size_t i = 0; m_env[i]; i++)
		env.push_back(m_env[i]);
	return env;
}

void Request::childProcess(int* pipe_out, int* pipe_in, bool hasBody, const std::string& scriptPath, const std::vector<std::string>& args, char** m_env, const std::vector<std::string>& extraEnv) {
	dup2(pipe_out[1], STDOUT_FILENO);
	close(pipe_out[0]);
	close(pipe_out[1]);

	std::cerr << "Executing CGI script: " << scriptPath << std::endl;

	if (hasBody) {
		dup2(pipe_in[0], STDIN_FILENO);
		close(pipe_in[1]);
		close(pipe_in[0]);
	}
	
	size_t argc = args.size() + 2;
	char **av = new char*[argc];
	av[0] = const_cast<char *>(scriptPath.c_str());

	for (size_t i = 0; i < args.size(); ++i)
		av[i + 1] = const_cast<char *>(args[i].c_str());
	av[argc - 1] = NULL;

	std::vector<std::string> envVec;

	for (size_t i = 0; m_env && m_env[i]; ++i)
		envVec.push_back(m_env[i]);

	for (size_t i = 0; i < extraEnv.size(); ++i)
		envVec.push_back(extraEnv[i]);

	char **envp = new char*[envVec.size() + 1];

	for (size_t i = 0; i < envVec.size(); ++i)
		envp[i] = const_cast<char *>(envVec[i].c_str());

	envp[envVec.size()] = NULL;
	execve(av[0], av, envp);
	delete[] av;
	delete[] envp;
	std::cerr << "execve failed: " << strerror(errno) << std::endl;
	exit(EXIT_FAILURE);
}

void Request::parentProcess(int* pipe_out, int* pipe_in, bool hasBody, std::string& cgiOutput) {
	close(pipe_out[1]);
	if (hasBody)
		close(pipe_in[0]);

	if (hasBody) {
		const std::string& body = getRawBody();
		ssize_t written = 0;
		ssize_t total = 0;
		while (total < (ssize_t)body.size()) {
			written = write(pipe_in[1], body.data() + total, body.size() - total);
			if (written <= 0)
				break;
			total += written;
		}
		close(pipe_in[1]);
	}
	
	char buffer[256];
	cgiOutput.clear();
	ssize_t bytes;

	while ((bytes = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
		cgiOutput.append(buffer, bytes);

	close(pipe_out[0]);
}

bool Request::doCGI(const std::string& scriptPath, const std::vector<std::string>& args, const std::vector<std::string>& extraEnv, std::string& cgiOutput) {
	int pipe_out[2];
	int pipe_in[2];

	if (pipe(pipe_out) == -1) {
		Logger::log(RED, "pipe failed for CGI");
		return false;
	}

	bool hasBody = (getMethod() == "POST" || getMethod() == "PUT");

	if (hasBody && pipe(pipe_in) == -1) {
		Logger::log(RED, "pipe_in failed for CGI");
		close(pipe_out[0]);
		close(pipe_out[1]);
		return false;
	}

	pid_t pid = fork();
	if (pid < 0) {
		Logger::log(RED, "fork failed for CGI");
		close(pipe_out[0]);
		close(pipe_out[1]);
		if (hasBody) {
			close(pipe_in[0]);
			close(pipe_in[1]);
		}
		return false;
	}
	
	if (pid == 0)
		childProcess(pipe_out, pipe_in, hasBody, scriptPath, args, m_env, extraEnv);
	else {
		parentProcess(pipe_out, pipe_in, hasBody, cgiOutput);
		if (!g_sigok) {
			g_sigok = true;
			return false;
		}
		return true;
	}
	return true;
}

void Request::autoIndex(const std::string& customPath, const std::string& customDisplayPath) {
	std::string localPath;
	std::string displayPath;
	
	if (!customPath.empty()) {
		localPath = customPath;
		displayPath = customDisplayPath.empty() ? m_path : customDisplayPath;
	}
	else {
		std::string root = m_loc.getRoot();
		localPath = normalizePath(root + "/" + m_path);
		displayPath = m_path;
	}

	DIR* directory = opendir(localPath.c_str());
	if (!directory) {
		Logger::log(RED, "Failed to open directory: %s", localPath.c_str());
		return;
	}

	struct dirent* entry;
	std::string page;

	page += "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n";
	page += "<title>Index of " + displayPath + "</title>\n";
	page += "<style>\nbody { font-family: sans-serif; max-width: 800px; margin: 40px auto; }\n";
	page += "table { border-collapse: collapse; width: 100%; }\n";
	page += "th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n";
	page += "a { text-decoration: none; color: #0066cc; }\n";
	page += "a:hover { text-decoration: underline; }\n</style>\n</head>\n";
	page += "<body>\n<h1>Index of " + displayPath + "</h1>\n";
	page += "<table>\n<tr><th>Name</th><th>Size</th></tr>\n";

	if (displayPath != "/")
		page += "<tr><td><a href=\"../\">../</a></td><td>-</td></tr>\n";

	while ((entry = readdir(directory)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		std::string filename = entry->d_name;
		std::string fullpath = localPath + "/" + filename;

		struct stat fileStat;
		if (stat(fullpath.c_str(), &fileStat) == 0) {
			page += "<tr><td><a href=\"" + filename;
			if (S_ISDIR(fileStat.st_mode))
				page += "/";
			page += "\">" + filename;
			if (S_ISDIR(fileStat.st_mode))
				page += "/";
			page += "</a></td>";

			if (S_ISDIR(fileStat.st_mode))
				page += "<td>-</td>";
			else {
				std::stringstream ss;
				ss << fileStat.st_size;
				page += "<td>" + ss.str() + " bytes</td>";
			}
			page += "</tr>\n";
		}
	}
	page += "</table>\n</body>\n</html>";
	closedir(directory);
	m_autoIndexPage = page;
}