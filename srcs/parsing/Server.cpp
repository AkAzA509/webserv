/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/09/27 02:10:08 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.h"
#include <numeric>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "Logger.h"

Server::Server() : m_Client_max_body_size(std::numeric_limits<size_t>::infinity()), m_root("./"), m_timeout(5000), m_sessionTimeoutMs(30 * 60 * 1000), m_sessionCookieName("session_id") {
	std::srand(static_cast<unsigned>(std::time(NULL) ^ reinterpret_cast<long>(this)));
}

Server::~Server() {}

void Server::addPort(size_t port)
{
	m_port.push_back(port);
}

void Server::removePort(size_t idx)
{
	if (idx < m_port.size())
		m_port.erase(m_port.begin() + idx);
}

std::vector<size_t> Server::getPorts() const
{
	return m_port;
}

size_t Server::getPort(size_t idx) const
{
	return m_port[idx];
}

void Server::setServerName(const std::string& name) { m_serverName = name; }

void Server::setHostIp(const std::string& ip)
{
	m_hostIp = ip;
}

void Server::setTimeout(const std::string& time)
{
	std::stringstream ss(time);
	ss >> m_timeout;
}

void Server::setRoot(const std::string& root) { m_root = root; }

std::string Server::getIndexFile(size_t idx) const
{
	if (idx < m_indexFiles.size())
		return m_indexFiles[idx];
	return std::string();
}

void Server::addIndexFile(const std::string& file)
{
	m_indexFiles.push_back(file);
}

void Server::removeIndexFile(size_t idx)
{
	if (idx < m_indexFiles.size())
		m_indexFiles.erase(m_indexFiles.begin() + idx);
}

std::string Server::getErrorPage(int page) const
{
	std::map<int, std::string>::const_iterator it;

	if ((it = m_errorPages.find(page)) != m_errorPages.end())
		return it->second;
	return std::string();
}

void Server::addErrorPage(int page, const std::string& path)
{
	m_errorPages[page] = path;
}

void Server::addLocation(Location& loc)
{
	m_locations.push_back(loc);
}

void Server::removeLocation(size_t idx)
{
	if (idx < m_locations.size())
		m_locations.erase(m_locations.begin() + idx);
}

void Server::removeSocket(int idx)
{
	if (idx <= (int)m_socketFd.size())
	{
		close(m_socketFd[idx]);
		m_socketFd.erase(m_socketFd.begin() + idx);
	}
}

void Server::setClientMaxBodySize(size_t size) {
	m_Client_max_body_size = size;
}

std::vector<std::string> splitRequest(const std::string& str) {
	std::vector<std::string> result;
	std::string line;
	std::istringstream stream(str);
	
	while (std::getline(stream, line)) {
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.end() - 1);
		result.push_back(line);
	}

	if (!str.empty() && (str[str.length() - 1] == '\n' || str[str.length() - 1] == '\r'))
		result.push_back("");

	return result;
}

std::string Server::getClientIp(int client_fd) const {
	std::map<int, ClientState>::const_iterator it = m_clients.find(client_fd);
	if (it != m_clients.end())
		return it->second.client_ip;
	return std::string();
}

std::string Server::generateSessionId() {
	unsigned long now = getCurrentTimeMs();
	std::ostringstream oss;
	oss << std::hex << std::uppercase << now << "-";
	oss << std::setw(8) << std::setfill('0') << (static_cast<unsigned long>(std::rand()) & 0xffffffffUL);
	oss << "-" << std::setw(4) << std::setfill('0') << (static_cast<unsigned long>(std::rand()) & 0xffffUL);
	return oss.str();
}

void Server::cleanupSessions(unsigned long nowMs) {
	for (std::map<std::string, SessionData>::iterator it = m_sessions.begin(); it != m_sessions.end();) {
		if (nowMs < it->second.last_seen || nowMs - it->second.last_seen > m_sessionTimeoutMs) {
			std::map<std::string, SessionData>::iterator toErase = it;
			++it;
			m_sessions.erase(toErase);
		}
		else
			++it;
	}
}

SessionData& Server::ensureSession(const std::string& incomingSessionId, const std::string& clientIp, bool& created) {
	unsigned long now = getCurrentTimeMs();
	cleanupSessions(now);
	created = false;

	if (!incomingSessionId.empty()) {
		std::map<std::string, SessionData>::iterator it = m_sessions.find(incomingSessionId);
		if (it != m_sessions.end()) {
			it->second.last_seen = now;
			if (!clientIp.empty())
				it->second.client_ip = clientIp;
			return it->second;
		}
	}

	std::string newId;
	do {
		newId = generateSessionId();
	} while (m_sessions.find(newId) != m_sessions.end());

	SessionData session;
	session.id = newId;
	session.created_at = now;
	session.last_seen = now;
	session.client_ip = clientIp;
	m_sessions[newId] = session;
	created = true;
	return m_sessions[newId];
}

SessionData* Server::getSession(const std::string& sessionId) {
	std::map<std::string, SessionData>::iterator it = m_sessions.find(sessionId);
	if (it != m_sessions.end())
		return &it->second;
	return NULL;
}

const SessionData* Server::getSession(const std::string& sessionId) const {
	std::map<std::string, SessionData>::const_iterator it = m_sessions.find(sessionId);
	if (it != m_sessions.end())
		return &it->second;
	return NULL;
}

void Server::destroySession(const std::string& sessionId) {
	m_sessions.erase(sessionId);
}

void Server::setSessionValue(const std::string& sessionId, const std::string& key, const std::string& value) {
	unsigned long now = getCurrentTimeMs();
	cleanupSessions(now);
	std::map<std::string, SessionData>::iterator it = m_sessions.find(sessionId);
	if (it == m_sessions.end())
		return;
	it->second.values[key] = value;
	it->second.last_seen = now;
}

bool Server::getSessionValue(const std::string& sessionId, const std::string& key, std::string& out) const {
	std::map<std::string, SessionData>::const_iterator it = m_sessions.find(sessionId);
	if (it == m_sessions.end())
		return false;
	std::map<std::string, std::string>::const_iterator valueIt = it->second.values.find(key);
	if (valueIt == it->second.values.end())
		return false;
	out = valueIt->second;
	return true;
}

void Server::touchSession(const std::string& sessionId) {
	unsigned long now = getCurrentTimeMs();
	cleanupSessions(now);
	std::map<std::string, SessionData>::iterator it = m_sessions.find(sessionId);
	if (it != m_sessions.end())
		it->second.last_seen = now;
}

std::ostream& operator<<(std::ostream& os, const Server& server)
{
	os << "Server {\n";
	os << "  Server Name: " << server.getServerName() << "\n";
	os << "  Host IP: " << server.getHostIp() << "\n";
	os << "  Root: " << server.getRoot() << "\n";

	os << "  Ports: [";
	for (size_t i = 0; i < server.getPorts().size(); ++i) {
		os << server.getPorts()[i];
		if (i != server.getPorts().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  Index Files: [";
	for (size_t i = 0; i < server.getIndexFiles().size(); ++i) {
		os << server.getIndexFiles()[i];
		if (i != server.getIndexFiles().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  Error Pages: {\n";
	for (std::map<int, std::string>::const_iterator it = server.getErrorPages().begin(); it != server.getErrorPages().end(); ++it) {
		os << "    " << it->first << ": " << it->second << "\n";
	}
	os << "  }\n";

	os << "  Locations: \n";
	const std::vector<Location>& locs = server.getLocations();
	for (size_t i = 0; i < locs.size(); ++i) {
		os << "    [" << i << "] " << locs[i] << "\n";
	}

	os << "}";
	return os;
}

Location::Location() : m_autoIndex(false) 
{
	#if DEBUG
		Logger::log(CYAN, "Location CONSTRUCTION");
	#endif
}

Location::~Location()
{
	#if DEBUG
		Logger::log(CYAN, "Location DESTRUCTION");
	#endif
}

Location::Location(const std::string& path) : m_path(path)
{
	#if DEBUG
		Logger::log(CYAN, "LOCATION DESTRUCTION");
	#endif
}


std::ostream& operator<<(std::ostream& os, const Location& loc)
{
	os << "Location {\n";
	os << "  Path: " << loc.getPath() << "\n";
	os << "  Redirection Path: " << loc.getRedirectionPath() << "\n";
	os << "  Root: " << loc.getRoot() << "\n";
	os << "  AutoIndex: " << (loc.isAutoIndexOn() ? "On" : "Off") << "\n";

	os << "  Index Files: [";
	for (size_t i = 0; i < loc.getIndexFiles().size(); ++i) {
		os << loc.getIndexFiles()[i];
		if (i != loc.getIndexFiles().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  CGI Paths: [";
	for (size_t i = 0; i < loc.getCgiPath().size(); ++i) {
		os << loc.getCgiPath()[i];
		if (i != loc.getCgiPath().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  CGI Extensions: [";
	for (size_t i = 0; i < loc.getCgiExt().size(); ++i) {
		os << loc.getCgiExt()[i];
		if (i != loc.getCgiExt().size() - 1) os << ", ";
	}
	os << "]\n";

	os << "  Allowed Methods: [";
	const std::vector<std::string>& methods = loc.getAllowedMethods();
	for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it)
	{
		os << *it;
		if (it < methods.end() - 1)
			os << " ";
	}
	os << "]\n";

	os << "}";

	return os;
}
