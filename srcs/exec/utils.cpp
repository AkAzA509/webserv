#include "Server.h"
#include "Config.h"
#include "Parser.h"
#include "Logger.h"
#include "Request.h"

void print_error(const std::string& str, int fd) {
	if (fd != -1)
		close(fd);
	Logger::log(RED, "Error: %s", str.c_str());
	throw std::runtime_error(str);
}

// Ouvre le fichier et le charge en buffer pour le retourner
std::string loadFile(const std::string& path) {
	if (path.empty()) {
		Logger::log(RED, "Empty file path");
		return "";
	}

	if (access(path.c_str(), F_OK | R_OK) < 0) {
		Logger::log(RED, "File access failed for %s", path.c_str());
		return "";
	}

	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0) {
		Logger::log(RED, "open failed for %s", path.c_str());
		return "";
	}

	std::string content;
	char buffer[4096];
	ssize_t bytesRead;

	while ((bytesRead = read(fd, buffer, sizeof(buffer)))) {
		if (bytesRead < 0) {
			if (errno == EINTR) 
				continue;
			Logger::log(RED, "read failed for %s", path.c_str());
			close(fd);
			return "";
		}
		content.append(buffer, bytesRead);
	}

	if (close(fd))
		Logger::log(RED, "close failed for %s", path.c_str());

	return content;
}

bool Location::isAllowedMethod(const std::string& method) const {
	for (std::vector<std::string>::const_iterator it = m_allowed_methods.begin(); it != m_allowed_methods.end(); ++it){
		if (*it == method)
			return true;
	}
	return false;
}

unsigned long hash(int x) {
	return static_cast<u_long>(x * 2654435761UL);
}

unsigned long hash(const std::string& str)
{
	ulong hash = 5381;

	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
		hash = ((hash << 5) + hash) + *it;

	return hash;
}

// Evalue le type de body renvoyer en response
std::string getFileType(const std::string& path) {
	size_t dot = path.rfind('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = path.substr(dot + 1);
	if (ext == "html" || ext == "htm")
		return "text/html";
	else if (ext == "css")
		return "text/css";
	else if (ext == "js")
		return "application/javascript";
	else if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	else if (ext == "png")
		return "image/png";
	else if (ext == "gif")
		return "image/gif";
	else if (ext == "txt")
		return "text/plain";
	else if (ext == "py")
		return "text/plain";
	else if (ext == "sh")
		return "text/plain";
	else if (ext == "pdf")
		return "application/pdf";
	else if (ext == "json")
		return "application/json";
	else if (ext == "xml")
		return "application/xml";
	return "application/octet-stream";
}

// Parcour les path recus et les nettoies
std::string normalizePath(const std::string& path) {
	std::vector<std::string> stack;
	bool isAbsolute = !path.empty() && path[0] == '/';

	size_t i = 0;
	while (i < path.size()) {
		while (i < path.size() && path[i] == '/')
			++i;
		size_t start = i;
		while (i < path.size() && path[i] != '/')
			++i;
		std::string token = path.substr(start, i - start);
		if (token.empty() || token == ".")
			continue;
		if (token == "..") {
			if (!stack.empty())
				stack.pop_back();
		}
		else
			stack.push_back(token);
	}

	std::string normalized;
	if (isAbsolute)
		normalized += "/";
	for (size_t j = 0; j < stack.size(); ++j) {
		normalized += stack[j];
		if (j + 1 < stack.size())
			normalized += "/";
	}
	if (normalized.empty())
		return isAbsolute ? "/" : ".";
	return normalized;
}

// Decode les url recu
std::string urlDecode(const std::string& str) {
	std::ostringstream decoded;
	for (size_t i = 0; i < str.length(); ++i) {
		if (str[i] == '%' && i + 2 < str.length() && std::isxdigit(str[i + 1]) && std::isxdigit(str[i + 2])) {
			std::string hex = str.substr(i + 1, 2);
			char ch = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
			decoded << ch;
			i += 2;
		}
		else if (str[i] == '+')
			decoded << ' ';
		else
			decoded << str[i];
	}
	return decoded.str();
}

// Remplis l'environnement pour exec les cgi
std::vector<std::string> prepareCgiEnv(const Request& req, const std::string& scriptName, const std::string& reqPath) {
	std::vector<std::string> env;
	env.push_back("REQUEST_METHOD=" + req.getMethod());
	env.push_back("PATH_INFO=" + reqPath);
	env.push_back("SCRIPT_NAME=" + scriptName);

	if (req.getMethod() == "GET") {
		std::string::size_type qpos = reqPath.find('?');
		std::string query = (qpos != std::string::npos) ? reqPath.substr(qpos + 1) : "";
		env.push_back("QUERY_STRING=" + query);
	}
	if (req.getMethod() == "POST") {
		std::string cl = req.getHeader("Content-Length");
		if (!cl.empty())
			env.push_back("CONTENT_LENGTH=" + cl);
		std::string ct = req.getHeader("Content-Type");
		if (!ct.empty())
			env.push_back("CONTENT_TYPE=" + ct);
	}
	return env;
}

unsigned long getCurrentTimeMs() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000L + tv.tv_usec / 1000L; 
}