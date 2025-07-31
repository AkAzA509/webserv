/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/24 10:42:54 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/31 15:16:50 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.h"

Response::Response(Request req, Server serv) : m_req(req), m_serv(serv) {
	std::string status = m_req.getResponseStatus();
	if (status != HEADER_OK)
		isErrorPage(status);
	else {
		std::string name[5] = {"GET", "HEAD", "DELETE", "POST", "PUT"};
		void(Response::*fonction[])() = {&Response::methodeWithBodyResponse, &Response::methodeWithinBodyResponse};
		int j = -1;
		for (int i = 0; i < 5; i++) {
			if (name[i].compare(m_req.getMethod())) {
				j = i;
				break;
			}
		}
		if (j >= 0 && j <= 1)
			(this->*fonction[0])();
		else

			(this->*fonction[1])();
	}
		
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

std::string Response::getResponse() const{
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

void Response::methodeWithBodyResponse() {
	std::string type;
	std::string file;

	// Cookie testing: Get cookies from request
	std::string cookie_header = m_req.getHeader("Cookie");
	
	// Set test cookies
	setCookie("webserv_session", "session_" + getCurrentTimestamp(), "/", 3600);
	setCookie("last_visit", getCurrentTimestamp(), "/", 86400);
	
	// If cookies were received, set a response cookie
	if (!cookie_header.empty()) {
		setCookie("cookie_received", "yes", "/", 300);
	}

	if (m_req.getUrl() == "/") {
		std::string url = m_serv.getRoot() + m_serv.getIndexFile(0);
		if (url.find(".html"))
			type = HTML;
		else if (url.find(".css"))
			type = CSS;
		else if (url.find(".js"))
			type = JS;
		file = loadFile(url);
		
		// Check if index file exists
		if (file.empty()) {
			std::string error = ERROR_404;
			isErrorPage(error);
			return;
		}
	}
	else {
		// Handle other file requests - extract document root from index file
		std::string index_path = m_serv.getIndexFile(0); // "page/index.html"
		std::string doc_root = "";
		
		// Extract directory part from index file (everything before the filename)
		size_t last_slash = index_path.find_last_of('/');
		if (last_slash != std::string::npos) {
			doc_root = index_path.substr(0, last_slash); // "page"
		}
		
		// Construct URL: root + document_directory + requested_file
		std::string url = m_serv.getRoot();
		if (!doc_root.empty()) {
			url += doc_root + m_req.getUrl();
		} else {
			url += m_req.getUrl().substr(1); // Remove leading '/'
		}
		
		// Determine content type based on file extension
		if (url.find(".html") != std::string::npos)
			type = HTML;
		else if (url.find(".css") != std::string::npos)
			type = CSS;
		else if (url.find(".js") != std::string::npos)
			type = JS;
		else if (url.find(".png") != std::string::npos)
			type = "image/png";
		else if (url.find(".jpg") != std::string::npos || url.find(".jpeg") != std::string::npos)
			type = "image/jpeg";
		else
			type = "text/plain";
			
		file = loadFile(url);
		
		// Check if requested file exists
		if (file.empty()) {
			std::string error = ERROR_404;
			isErrorPage(error);
			return;
		}
	}
	fillResponse(type, file, m_req.getResponseStatus());
	
	// Add cookies to response
	addCookieHeader();
}

void Response::methodeWithinBodyResponse() {
	std::string empty;
	fillResponse(empty, empty, m_req.getResponseStatus());
}

void Response::fillResponse(std::string type, std::string& file, std::string head) {
	std::ostringstream oss;
	oss << file.size();
	std::string size = oss.str();
	m_response = head;
	if (!type.empty()) {
		m_response += CONTENT_LENGHT + size + RETURN;
		m_response += CONTENT_TYPE + type + RETURN;
	}
	m_response += LOCATION_ROOT;
	m_response += CONNECTION_CLOSE;
	m_response += RETURN;
	if (!file.empty())
		m_response += file;
}

// Cookie implementation methods
void Response::setCookie(const std::string& name, const std::string& value, 
                        const std::string& path, int max_age) {
    std::string cookie_value = value;
    
    // Basic cookie format: name=value
    std::string cookie_header = name + "=" + cookie_value;
    
    // Add path if specified
    if (!path.empty()) {
        cookie_header += "; Path=" + path;
    }
    
    // Add max-age if specified
    if (max_age >= 0) {
        std::stringstream ss;
        ss << max_age;
        cookie_header += "; Max-Age=" + ss.str();
    }
    
	cookie_header += "\r\n";
    // Store cookie to be added to response headers
    m_cookies_to_set[name] = cookie_header;
}

void Response::addCookieHeader() {
    // Add Set-Cookie headers to response
    for (std::map<std::string, std::string>::iterator it = m_cookies_to_set.begin(); 
         it != m_cookies_to_set.end(); ++it) {
        // Find the position to insert before the final \r\n\r\n
        size_t final_crlf = m_response.find("\r\n\r\n");
        if (final_crlf != std::string::npos) {
            std::string cookie_line = "Set-Cookie: " + it->second + "\r\n";
            m_response.insert(final_crlf, cookie_line);
        } else {
            // If no double CRLF found, add before Connection: close
            size_t connection_pos = m_response.find("Connection: close");
            if (connection_pos != std::string::npos) {
                std::string cookie_line = "Set-Cookie: " + it->second + "\r\n";
                m_response.insert(connection_pos, cookie_line);
            }
        }
    }
}

std::string Response::getCurrentTimestamp() {
    time_t now = time(0);
    std::stringstream ss;
	ss << std::ctime(&now);
    // ss << now;
    return ss.str();
}

std::ostream& operator<<(std::ostream& o, const Response& a)
{
	o << a.getResponse();
	return o;
}