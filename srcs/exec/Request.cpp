#include "Request.h"
#include "Parser.h"
#include <cctype>
#include <algorithm>

std::ostream& operator<<(std::ostream& o, const BinaryInfo& info)
{
	o << info.filename << " " << info.data;
	return o;
}

Request::Request(Server& server, const std::string& request) 
	: m_iserrorPage(false), m_error_page(0) 
{
	parseHeader(request, server);
	if (!m_iserrorPage) {
		parseBody(request);
	}
}

// Case-insensitive string comparison helper
bool iequals(const std::string& a, const std::string& b) {
	if (a.length() != b.length()) {
		return false;
	}
	
	for (size_t i = 0; i < a.length(); ++i) {
		if (std::tolower(a[i]) != std::tolower(b[i])) {
			return false;
		}
	}
	return true;
}

void Request::parseContentDispo(const std::string& line, BinaryInfo& binInfo) {
	std::vector<std::string> args = split(line, ";");
	for (size_t i = 1; i < args.size(); i++) {
		std::string arg = trim(args[i]);
		size_t eq_pos = arg.find('=');
		if (eq_pos == std::string::npos) continue;

		std::string key = trim(arg.substr(0, eq_pos));
		std::string value = trim(arg.substr(eq_pos + 1));
		
		if (value.size() >= 2 && value[0] == '"' && value[value.size() - 1] == '"') {
			value = value.substr(1, value.size() - 2);
		}

		if (iequals(key, "name")) {
			binInfo.field_name = value;
		} else if (iequals(key, "filename")) {
			binInfo.filename = value;
		}
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
			if (end_pos == std::string::npos) break;
		}

		std::string part = body.substr(start_pos, end_pos - start_pos);
		start_pos = end_pos + boundary.length();

		// Skip leading CRLF
		if (part.substr(0, 2) == "\r\n") part = part.substr(2);
		
		// Separate headers and body
		size_t header_end = part.find("\r\n\r\n");
		if (header_end == std::string::npos) continue;

		BinaryInfo binInfo;
		std::string headers_block = part.substr(0, header_end);
		binInfo.data = part.substr(header_end + 4);

		std::vector<std::string> headers = splitset(headers_block, "\r\n");
		for (size_t i = 0; i < headers.size(); i++) {
			if (iequals(headers[i].substr(0, 21), "Content-Disposition:")) {
				parseContentDispo(headers[i], binInfo);
			}
		}

		if (binInfo.data.size() >= 2 && 
			binInfo.data.substr(binInfo.data.size() - 2) == "\r\n") {
			binInfo.data.resize(binInfo.data.size() - 2);
		}

		// Add these checks in parseBinaryInfos:
		std::cout << "Boundary: '" << m_boundary << "'" << std::endl;
		std::cout << "Body size: " << body.size() << std::endl;

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
			if (!m_boundary.empty()) {
				parseBinaryInfos(body);
			}
		}
	}
}

void Request::parseHeader(const std::string& request, const Server& server) {
	size_t header_end = request.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		setError(E_ERROR_400);
		return;
	}

	std::string header_block = request.substr(0, header_end);
	std::vector<std::string> headers = splitset(header_block, "\r\n");

	std::vector<std::string> req_line = splitset(headers[0], " ");
	if (req_line.size() != 3) {
		setError(E_ERROR_400);
		return;
	}

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
		if (m_path.compare(0, loc_path.size(), loc_path) == 0 && 
			loc_path.size() > max_len) {
			best_match = &locations[i];
			max_len = loc_path.size();
		}
	}

	if (!best_match) {
		std::cout << "La" << std::endl;
		setError(E_ERROR_404);
		return;
	}
	m_loc = *best_match;

	for (size_t i = 1; i < headers.size(); i++) {
		size_t colon = headers[i].find(':');
		if (colon == std::string::npos) continue;

		std::string key = trim(headers[i].substr(0, colon));
		std::string value = trim(headers[i].substr(colon + 1));
		
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		m_headers[key] = value;
	}
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

std::ostream& operator<<(std::ostream& o, const Request& req) {
    // Basic request info
    o << "=== HTTP Request ===" << std::endl;
    o << "Method: " << req.getMethod() << std::endl;
    o << "Path: " << req.getPath() << std::endl;
    o << "HTTP Version: " << req.getHttpVersion() << std::endl;
    
    // Location info
    const Location& loc = req.getLocation();
    o << "\n=== Location Configuration ===" << std::endl;
    o << "Path: " << loc.getPath() << std::endl;
    o << "Root: " << loc.getRoot() << std::endl;
    o << "Upload Path: " << loc.getUploadPath() << std::endl;
    o << "Autoindex: " << (loc.isAutoIndexOn() ? "On" : "Off") << std::endl;
    o << "Index Files: ";
    const std::vector<std::string>& indexes = loc.getIndexFiles();
    for (size_t i = 0; i < indexes.size(); ++i) {
        if (i != 0) o << ", ";
        o << indexes[i];
    }
    o << std::endl;
    
    // Error status
    if (req.IsErrorPage()) {
        o << "\n=== Error ===" << std::endl;
        o << "Error Code: " << req.getErrorPage() << std::endl;
    }
    
    // Headers
    o << "\n=== Headers ===" << std::endl;
	const std::map<std::string, std::string>& headers = req.getAllHeaders();
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		o << it->first << ": " << it->second << std::endl;
    // You would need to add a method to get all headers from Request
    // For example: const std::map<std::string, std::string>& headers = req.getAllHeaders();
    // Then iterate through them
    
    // Binary data (uploads)
    const std::vector<BinaryInfo>& binaries = req.getBinaryInfos();
    if (!binaries.empty()) {
        o << "\n=== Binary Data ===" << std::endl;
        for (size_t i = 0; i < binaries.size(); ++i) {
            const BinaryInfo& bin = binaries[i];
            o << "Field: " << bin.field_name << std::endl;
            o << "Filename: " << bin.filename << std::endl;
            o << "Data Size: " << bin.data.size() << " bytes" << std::endl;
            if (i != binaries.size() - 1) o << "---" << std::endl;
        }
    }
    
    return o;
}