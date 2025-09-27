/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 12:03:36 by ggirault          #+#    #+#             */
/*   Updated: 2025/09/27 01:43:03 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include "Server.h"
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>

struct BinaryInfo
{
	std::string field_name;
	std::string filename;
	std::string data;

	BinaryInfo() {}
	BinaryInfo(const std::string& name, const std::string& file, const std::string& data) : field_name(name), filename(file), data(data) {}
};

std::ostream& operator<<(std::ostream& o, const BinaryInfo& info);

enum Error
{
	E_HEADER_OK = 200,
	E_HEADER_201 = 201,
	E_HEADER_303 = 303,
	E_ERROR_400 = 400,
	E_ERROR_403 = 403,
	E_ERROR_404 = 404,
	E_ERROR_405 = 405,
	E_ERROR_408 = 408,
	E_ERROR_411 = 411,
	E_ERROR_418 = 418,
	E_ERROR_500 = 500,
};

class Request {
private:
	Location m_loc;
	std::string m_method;
	std::string m_path;
	std::string m_httpVersion;
	bool m_iserrorPage;
	std::string m_boundary;
	int m_error_page;
	std::map<std::string, std::string> m_headers;
	std::vector<BinaryInfo> m_BinaryInfos;
	char **m_env;
	std::string m_cgiOutput;
	std::string m_autoIndexPage;
	bool m_isAutoIndex;
	std::string m_rawBody;
	std::vector<CookieData> m_cookies;
	std::string m_sessionId;

	void setError(int error_code);

public:
	Request();
	Request(Server& server, const std::string& request, char **env);
	Request(const Request& copy);
	~Request() {};
	Request& operator=(const Request& other);

	
public:
	const std::vector<CookieData>& getCookies() const { return m_cookies; }
	const std::string& getMethod() const { return m_method; }
	bool getIsAutoIndex() const { return m_isAutoIndex; }
	const std::string& getCgiOutput() const { return m_cgiOutput; }
	const std::string& getAutoIndex() const { return m_autoIndexPage; }
	const std::string& getRawBody() const { return m_rawBody; }
	const std::string& getPath() const { return m_path; }
	const std::map<std::string, std::string> getAllHeaders() const { return m_headers; }
	const std::string& getHttpVersion() const { return m_httpVersion; }
	bool IsErrorPage() const { return m_iserrorPage; }
	int getErrorPage() const { return m_error_page; }
	const Location& getLocation() const { return m_loc; }
	const std::vector<BinaryInfo>& getBinaryInfos() const { return m_BinaryInfos; }
	std::string getHeader(const std::string& key) const;
	std::string getCookieValue(const std::string& name) const;
	void setSessionId(const std::string& sessionId) { m_sessionId = sessionId; }
	const std::string& getSessionId() const { return m_sessionId; }

	void autoIndex(const std::string& customPath = "", const std::string& customDisplayPath = "");
	std::vector<std::string> convertEnv();
	bool parentProcess(int* pipe_out, int* pipe_in, bool hasBody, std::string& cgiOutput, pid_t pid);
	void childProcess(int* pipe_out, int* pipe_in, bool hasBody, const std::string& scriptPath, const std::vector<std::string>& args, char** m_env, const std::vector<std::string>& extraEnv);
	void parseBody(const std::string& request);
	void parseContentDispo(const std::string& line, BinaryInfo& outBinInfo);
	void parseHeader(const std::string& request, const Server& server);
	void parseCookies();
	void parseBinaryInfos(const std::string& body);
	bool doCGI(const std::string& scriptPath, const std::vector<std::string>& args, const std::vector<std::string>& extraEnv, std::string& cgiOutput);
	
	// friend std::ostream& operator<<(std::ostream& o, const Request& req);
};