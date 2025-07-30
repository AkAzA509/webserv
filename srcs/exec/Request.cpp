/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 10:13:39 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/30 17:54:50 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.h"
#include "Parser.h"
#include "Logger.h"

Request::Request(Location loc, std::vector<std::string>& firstRequestLine, std::vector<std::string>& request, std::string& full_request)
: m_loc(loc), m_methode(firstRequestLine[0]), m_url(firstRequestLine[1]), m_version(firstRequestLine[2]), m_foundBody(false), m_errorPage(false), m_responseStatus(HEADER_OK) {

	// Parse headers from request
	parseHeaders(request);

	std::string methode_name[4] = {"GET", "POST", "DELETE", "PUT"};
	void(Request::*fonction[])(std::vector<std::string>&, std::string&) = {&Request::methodeGet, &Request::methodePost, &Request::methodeDelete, &Request::methodePut};

	if (!m_loc.isAllowedMethode(m_methode)) {
		Logger::log(RED, "methode %s not autorized for this location", m_methode.c_str());
		m_responseStatus = ERROR_405;
		m_errorPage = true;
	}
	else {
		int j = -1;
		for (int i = 0; i < 5; i++) {
			if (methode_name[i].compare(m_methode) == 0)
				j = i;
		}
		if (j != -1)
				(this->*fonction[j])(request, full_request);
	}
}

Request::Request(const Request &copy) {
	*this = copy;
}

Request& Request::operator=(const Request &other) {
	if (this != &other) {
		m_loc = other.m_loc;
		m_methode = other.m_methode;
		m_url = other.m_url;
		m_version = other.m_version;
		m_content_type = other.m_content_type;
		m_content_length = other.m_content_length;
		m_body = other.m_body;
		m_foundBody = other.m_foundBody;
		m_errorPage = other.m_errorPage;
		m_responseStatus = other.m_responseStatus;
	}
	return *this;
}

std::vector<std::string>& Request::convertEnv() {
	std::vector<std::string> env;
	
	for (size_t i = 0; m_env[i]; i++)
		env.push_back(m_env[i]);
	return env;
}

void Request::doCGI(size_t end_header, std::string& request) {
	std::vector<std::string> env;
	char *av[] = { (char *)"./cgi-bin/change_color.sh", NULL };
	int fd[2];

	env = convertEnv();

	env.push_back("REQUEST_METHOD=" + m_methode);
	env.push_back("CONTENT_LENGTH=" + request.substr(end_header).size());
	env.push_back("SERVER_PROTOCOL=HTTP/1.1");

	std::vector<char *> new_env;
	for (size_t i = 0; i < env.size(); ++i)
		new_env.push_back(const_cast<char *>(env[i].c_str()));
	new_env.push_back(NULL);

	if (pipe(fd)) {
		Logger::log(RED, "error pipe : pipe failed");
		return ;
	}

	pid_t pid = fork();
	if(pid == 0) {
		dup2();
		execve("./cgi-bin/change_color.sh", av, new_env.data());
	}
}

void Request::methodePost(std::vector<std::string>& tab, std::string& full_request) {
	(void)tab;

	// cherche la fin du header et divise la string en 2 header/body
	size_t header_end = full_request.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		Logger::log(WHITE, "error request build: header not complete");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
	}
	
	if (m_url.find("cgi-bin")) {
		doCGI(header_end + 4, full_request);
		return ;
	}

	std::string headers = full_request.substr(0, header_end);
	std::string body = full_request.substr(header_end + 4);

	// recupere les donnees du header
	std::istringstream stream(headers);
	std::string line;
	while (std::getline(stream, line)) {
		if (line.find("Content-Length:") == 0)
			m_content_length = line.substr(15);
		else if (line.find("Content-Type:") == 0)
			m_content_type = line.substr(13);
		// Note: Cookie parsing is already handled by parseHeaders() in constructor
	}
	if (m_content_length.empty() || std::atoi(m_content_length.c_str()) == 0) {
		m_errorPage = true;
		m_responseStatus = ERROR_411;
		return;
		return;
	}

	// cherche le boundary et le stock avec l'ajout de '--' avant
	std::string boundary_key = "boundary=";
	size_t b_pos = m_content_type.find(boundary_key);
	if (b_pos == std::string::npos) {
		Logger::log(WHITE, "error request build: boundary not found in header");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
		return;
	}
	std::string boundary = "--" + m_content_type.substr(b_pos + boundary_key.size());

	// supprime les caracteres polluant dans le boundary
	boundary.erase(boundary.find_last_not_of("\r\n") + 1);

	// cherche le boundary dans le body
	size_t part_start = body.find(boundary);
	if (part_start == std::string::npos) {
		Logger::log(WHITE, "error request build: boundary not found in body");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
		return;
	}
	
	part_start += boundary.length() + 2;

	
	size_t part_header_end = body.find("\r\n\r\n", part_start);
	if (part_header_end == std::string::npos) {
		Logger::log(WHITE, "error request build: missing Content-disposition");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
		return;
	}
	std::string part_header = body.substr(part_start, part_header_end - part_start);

	// cherche le nom du fichier dans le Content-disposition
	std::string filename;
	size_t fname_pos = part_header.find("filename=\"");
	if (fname_pos != std::string::npos) {
		fname_pos += 10;
		size_t fname_end = part_header.find("\"", fname_pos);
		if (fname_end != std::string::npos)
			filename = part_header.substr(fname_pos, fname_end - fname_pos);
	}

	// ajoute le '--' du boundary de fin pour le cherche a la fin du body
	std::string end_boundary = boundary + "--";

	size_t data_start = part_header_end + 4;
	size_t data_end = body.find(end_boundary, data_start);

	if (data_end == std::string::npos) {
		Logger::log(WHITE, "error request build: end boundary not found");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
		return;
	}

	if (data_end >= 2 && body.substr(data_end - 2, 2) == "\r\n")
		data_end -= 2;

	// extrait le binaire du body et l'ecrit dans le fichier correspondant
	std::string file_data = body.substr(data_start, data_end - data_start);

	m_responseStatus = HEADER_201;

	writeFile(filename, file_data);
}


void Request::methodeGet(std::vector<std::string>& tab, std::string& request) {
	(void)request;
	for (std::vector<std::string>::iterator it = tab.begin() + 1; it != tab.end(); ++it) {
		std::string line = *it;

		std::vector<std::string> words = split(line, " ");
		if (!words.empty()) {
			if (words[0] == "Content-Length:")
				m_content_length = words[1];
			if (words[0] == "Content-Type:")
				m_content_type = words[1];
		}
		if (!words.empty()) {
			if (words[0] == "Content-Length:")
				m_content_length = words[1];
			if (words[0] == "Content-Type:")
				m_content_type = words[1];
		}
	}
}

void Request::methodeDelete(std::vector<std::string>& tab, std::string& request) {
	(void)request;
	std::cout << "coucou Delete\n";

	(void)tab;

}

void Request::writeFile(std::string& filename, std::string& file_data) {
	std::string path = m_loc.getPath() + "/" + filename;
	if (path[0] == '/')
		path.erase(0, 1);

	if (access(path.c_str(), F_OK) == 0 && access(path.c_str(), W_OK) != 0) {
		Logger::log(RED, "error open : permission denied to open %s !", filename.c_str());
		m_errorPage = true;
		m_responseStatus = ERROR_500;
		return;
	}
	if (access(path.c_str(), F_OK) == 0)
		m_responseStatus = HEADER_OK;

	std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
	if (!out.is_open()) {
		Logger::log(RED, "error open : the location path not existing upload failed !");
		m_errorPage = true;
		m_responseStatus = ERROR_404;
		return;
	}

	out.write(file_data.data(), file_data.size());
	out.close();
}

void Request::methodePut(std::vector<std::string>& tab, std::string& full_request) {
	(void)tab;

	// cherche la fin du header et divise la string en 2 header/body
	size_t header_end = full_request.find("\r\n\r\n");
	if (header_end == std::string::npos) {
		Logger::log(WHITE, "error request build: header not complete");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
	}

	std::string headers = full_request.substr(0, header_end);
	std::string body = full_request.substr(header_end + 4);

	// recupere les donnees du header
	std::istringstream stream(headers);
	std::string line;
	while (std::getline(stream, line)) {
		if (line.find("Content-Length:") == 0)
			m_content_length = line.substr(15);
		if (line.find("Content-Type:") == 0)
			m_content_type = line.substr(13);
	}
	if (m_content_length.empty() || std::atoi(m_content_length.c_str()) == 0) {
		m_errorPage = true;
		m_responseStatus = ERROR_411;
		return;
	}

	// cherche le boundary et le stock avec l'ajout de '--' avant
	std::string boundary_key = "boundary=";
	size_t b_pos = m_content_type.find(boundary_key);
	if (b_pos == std::string::npos) {
		Logger::log(WHITE, "error request build: boundary not found in header");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
	}
	std::string boundary = "--" + m_content_type.substr(b_pos + boundary_key.size());

	// supprime les caracteres polluant dans le boundary
	boundary.erase(boundary.find_last_not_of("\r\n") + 1);

	// cherche le boundary dans le body
	size_t part_start = body.find(boundary);
	if (part_start == std::string::npos) {
		Logger::log(WHITE, "error request build: boundary not found in body");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
	}
	
	part_start += boundary.length() + 2;

	
	size_t part_header_end = body.find("\r\n\r\n", part_start);
	if (part_header_end == std::string::npos) {
		Logger::log(WHITE, "error request build: missing Content-disposition");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
	}
	std::string part_header = body.substr(part_start, part_header_end - part_start);

	// cherche le nom du fichier dans le Content-disposition
	std::string filename;
	size_t fname_pos = part_header.find("filename=\"");
	if (fname_pos != std::string::npos) {
		fname_pos += 10;
		size_t fname_end = part_header.find("\"", fname_pos);
		if (fname_end != std::string::npos)
			filename = part_header.substr(fname_pos, fname_end - fname_pos);
	}

	// ajoute le '--' du boundary de fin pour le cherche a la fin du body
	std::string end_boundary = boundary + "--";

	size_t data_start = part_header_end + 4;
	size_t data_end = body.find(end_boundary, data_start);

	if (data_end == std::string::npos) {
		Logger::log(WHITE, "error request build: end boundary not found");
		m_errorPage = true;
		m_responseStatus = ERROR_400;
		return;
	}

	if (data_end >= 2 && body.substr(data_end - 2, 2) == "\r\n")
		data_end -= 2;

	// extrait le binaire du body et l'ecrit dans le fichier correspondant
	std::string file_data = body.substr(data_start, data_end - data_start);

	m_responseStatus = HEADER_201;

	writeFile(filename, file_data);
	m_responseStatus = HEADER_303;
}

std::string Request::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = m_headers.find(name);
    if (it != m_headers.end()) {
        return it->second;
    }
    return "";
}

void Request::parseHeaders(const std::vector<std::string>& request) {
    // Parse headers from request lines (skip first line which is method/url/version)
    for (size_t i = 1; i < request.size(); ++i) {
        const std::string& line = request[i];
        if (line.empty()) break; // End of headers
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim spaces
            size_t start = value.find_first_not_of(" \t");
            size_t end = value.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                value = value.substr(start, end - start + 1);
            }
            
            m_headers[name] = value;
        }
    }
}

std::string Request::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = m_headers.find(name);
    if (it != m_headers.end()) {
        return it->second;
    }
    return "";
}

void Request::parseHeaders(const std::vector<std::string>& request) {
    // Parse headers from request lines (skip first line which is method/url/version)
    for (size_t i = 1; i < request.size(); ++i) {
        const std::string& line = request[i];
        if (line.empty()) break; // End of headers
        
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // Trim spaces
            size_t start = value.find_first_not_of(" \t");
            size_t end = value.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos) {
                value = value.substr(start, end - start + 1);
            }
            
            m_headers[name] = value;
        }
    }
}

std::ostream& operator<<(std::ostream& o, Request& req)
{
	o << "=================================\n" << "Method: " << req.getMethod() << "\nContent-Length: " << req.getContentLength() << 
	"\nContent-Type: " << req.getContentType() <<
	"\nUrl: " << req.getUrl() << 
	"\nVersion: " << req.getVersion() <<
	"\nBody: " << req.getBody() <<
	"\n------------------\n" << req.getLocation() << "\n----------------------------------------------\n" << "=================================";
	return o;
}