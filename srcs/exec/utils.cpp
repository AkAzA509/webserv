#include "Server.h"
#include "Config.h"
#include "Parser.h"
#include "Logger.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdexcept>
#include <cstring> // for strerror
#include <sstream>
#include <iomanip>
#include <cctype>

void Server::cleanupClient(int epfd, int client_fd, struct epoll_event ev) {
	// Safely remove from epoll
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev) < 0) {
		Logger::log(RED, "epoll_ctl DEL failed for fd %d: %s", 
				client_fd, strerror(errno));
	}
	
	if (close(client_fd)) {
		Logger::log(RED, "close failed for fd %d: %s", 
				client_fd, strerror(errno));
	}
	
	// Remove from client map
	m_clients.erase(client_fd);
	Logger::log(YELLOW, "Cleaned up client fd %d", client_fd);
}


void print_error(const std::string& str, int fd) {
	if (fd != -1) close(fd);
	Logger::log(RED, "Error: %s", str.c_str());
	throw std::runtime_error(str);
}

std::string loadFile(const std::string& path) {
	if (path.empty()) {
		Logger::log(RED, "Empty file path");
		return "";
	}

	if (access(path.c_str(), F_OK | R_OK) < 0) {
		Logger::log(RED, "File access failed for %s: %s", 
				path.c_str(), strerror(errno));
		return "";
	}

	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0) {
		Logger::log(RED, "open failed for %s: %s", 
				path.c_str(), strerror(errno));
		return "";
	}

	std::string content;
	char buffer[4096];
	ssize_t bytesRead;

	while ((bytesRead = read(fd, buffer, sizeof(buffer)))) {
		if (bytesRead < 0) {
			if (errno == EINTR) continue;
			Logger::log(RED, "read failed for %s: %s", 
					path.c_str(), strerror(errno));
			close(fd);
			return "";
		}
		content.append(buffer, bytesRead);
	}

	if (close(fd)) {
		Logger::log(RED, "close failed for %s: %s", 
				path.c_str(), strerror(errno));
	}

	return content;
}

bool Location::isAllowedMethod(const std::string& method) const {
	for (std::vector<std::string>::const_iterator it = m_allowed_methods.begin(); it != m_allowed_methods.end(); ++it)
	{
		if (*it == method)
			return true;
	}
	return false;
}

std::string getMimeType(const std::string& path) {
	size_t dot = path.rfind('.');
	if (dot == std::string::npos) {
		return "application/octet-stream";
	}

	std::string ext = path.substr(dot + 1);
	if (ext == "html" || ext == "htm") return "text/html";
	if (ext == "css") return "text/css";
	if (ext == "js") return "application/javascript";
	if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
	if (ext == "png") return "image/png";
	if (ext == "gif") return "image/gif";
	if (ext == "txt") return "text/plain";
	if (ext == "pdf") return "application/pdf";
	if (ext == "json") return "application/json";
	if (ext == "xml") return "application/xml";

	return "application/octet-stream";
}
std::string normalizePath(const std::string& path)
{
    Logger::log(YELLOW, "path a normalizer: '%s'", path.c_str());
    std::vector<std::string> stack;
    bool isAbsolute = !path.empty() && path[0] == '/';

    size_t i = 0;
    while (i < path.size()) {
        while (i < path.size() && path[i] == '/') ++i;
        size_t start = i;
        while (i < path.size() && path[i] != '/') ++i;
        std::string token = path.substr(start, i - start);
        if (token.empty() || token == ".") continue;
        if (token == "..") {
            if (!stack.empty()) stack.pop_back();
        } else {
            stack.push_back(token);
        }
    }

    std::string normalized;
    if (isAbsolute) normalized += "/";
    for (size_t j = 0; j < stack.size(); ++j) {
        normalized += stack[j];
        if (j + 1 < stack.size()) normalized += "/";
    }
    if (normalized.empty()) return isAbsolute ? "/" : ".";
	Logger::log(YELLOW, "path normalizer: '%s'", normalized.c_str());
    return normalized;
}

// --- URL decode utility ---
std::string urlDecode(const std::string& str) {
    std::ostringstream decoded;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length() && std::isxdigit(str[i+1]) && std::isxdigit(str[i+2])) {
            std::string hex = str.substr(i+1, 2);
            char ch = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
            decoded << ch;
            i += 2;
        } else if (str[i] == '+') {
            decoded << ' ';
        } else {
            decoded << str[i];
        }
    }
    return decoded.str();
}