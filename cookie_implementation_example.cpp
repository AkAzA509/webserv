/* ************************************************************************** */
/*                                                                            */
/*   Cookie Implementation Example for WebServ                               */
/*   This file shows how to add cookie support to your Response class        */
/*                                                                            */
/* ************************************************************************** */

/*
// Add these methods to your Response.h:

class Response {
private:
    std::string m_response;
    Request m_req;
    Server m_serv;
    std::map<std::string, std::string> m_cookies_to_set; // New member for cookies to send

public:
    // ... existing methods ...
    
    // New cookie methods:
    void setCookie(const std::string& name, const std::string& value, 
                   const std::string& path = "/", int max_age = -1);
    void addCookieHeader();
    std::map<std::string, std::string> parseCookies(const std::string& cookie_header);
};
*/

// Add these implementations to your Response.cpp:

/*
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
        cookie_header += "; Max-Age=" + std::to_string(max_age);
    }
    
    // Store cookie to be added to response headers
    m_cookies_to_set[name] = cookie_header;
}

void Response::addCookieHeader() {
    // Add Set-Cookie headers to response
    for (std::map<std::string, std::string>::iterator it = m_cookies_to_set.begin(); 
         it != m_cookies_to_set.end(); ++it) {
        m_response += "Set-Cookie: " + it->second + "\r\n";
    }
}

std::map<std::string, std::string> Response::parseCookies(const std::string& cookie_header) {
    std::map<std::string, std::string> cookies;
    
    if (cookie_header.empty()) {
        return cookies;
    }
    
    // Split by semicolon and space
    size_t start = 0;
    size_t end = 0;
    
    while ((end = cookie_header.find(';', start)) != std::string::npos) {
        std::string cookie_pair = cookie_header.substr(start, end - start);
        
        // Trim leading/trailing spaces
        size_t first = cookie_pair.find_first_not_of(" \t");
        size_t last = cookie_pair.find_last_not_of(" \t");
        
        if (first != std::string::npos && last != std::string::npos) {
            cookie_pair = cookie_pair.substr(first, last - first + 1);
            
            // Split by '=' to get name and value
            size_t eq_pos = cookie_pair.find('=');
            if (eq_pos != std::string::npos) {
                std::string name = cookie_pair.substr(0, eq_pos);
                std::string value = cookie_pair.substr(eq_pos + 1);
                cookies[name] = value;
            }
        }
        
        start = end + 1;
    }
    
    // Handle the last cookie (after the last semicolon)
    std::string last_cookie = cookie_header.substr(start);
    size_t first = last_cookie.find_first_not_of(" \t");
    size_t last = last_cookie.find_last_not_of(" \t");
    
    if (first != std::string::npos && last != std::string::npos) {
        last_cookie = last_cookie.substr(first, last - first + 1);
        size_t eq_pos = last_cookie.find('=');
        if (eq_pos != std::string::npos) {
            std::string name = last_cookie.substr(0, eq_pos);
            std::string value = last_cookie.substr(eq_pos + 1);
            cookies[name] = value;
        }
    }
    
    return cookies;
}
*/

/*
// Example usage in your response building:

void Response::buildResponse() {
    // ... your existing response building code ...
    
    // Get cookies from request
    std::string cookie_header = m_req.getHeader("Cookie");
    std::map<std::string, std::string> request_cookies = parseCookies(cookie_header);
    
    // Process cookies (example: set a session cookie)
    if (request_cookies.find("session_id") == request_cookies.end()) {
        // No session cookie found, create one
        setCookie("session_id", "new_session_" + getCurrentTimestamp(), "/", 3600);
    }
    
    // Set a test cookie
    setCookie("last_visit", getCurrentTimestamp(), "/", 86400);  // 24 hours
    
    // Build your response headers
    m_response = "HTTP/1.1 200 OK\r\n";
    m_response += "Content-Type: text/html\r\n";
    
    // Add cookie headers
    addCookieHeader();
    
    m_response += "Connection: close\r\n";
    m_response += "\r\n";
    
    // Add your content...
    m_response += your_html_content;
}

// Helper function to get current timestamp
std::string getCurrentTimestamp() {
    time_t now = time(0);
    return std::to_string(now);
}
*/
