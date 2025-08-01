/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 10:13:39 by ggirault          #+#    #+#             */
/*   Updated: 2025/08/01 12:26:15 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.h"
#include "Parser.h"
#include "Logger.h"

Request::Request(Location loc, std::vector<std::string>& firstRequestLine, std::vector<std::string>& request, std::string& full_request, char **env)
: m_loc(loc), m_methode(firstRequestLine[0]), m_url(firstRequestLine[1]), m_version(firstRequestLine[2]), m_foundBody(false), m_errorPage(false), m_responseStatus(HEADER_OK), m_env(env) {
    if (!m_loc.isAllowedMethode(m_methode)) {
        setError(ERROR_405, "methode " + m_methode + " not autorized for this location");
        return;
    }
	std::cerr << BLINK << " VOICI LA METHODE RECU " << m_methode << std::endl;
	parseHeaders(request);
    if (m_methode == "GET")
        methodeGet(request, full_request);
    else if (m_methode == "POST")
        methodePost(request, full_request);
    else if (m_methode == "DELETE")
        methodeDelete(request, full_request);
    else if (m_methode == "PUT")
        methodePut(request, full_request);
    else
        setError(ERROR_405, "Unknown HTTP method: " + m_methode);
}

Request::Request(const Request &copy) { *this = copy; }

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
		m_headers = other.m_headers;
		m_env = other.m_env;
    }
    return *this;
}

// std::vector<std::string> Request::convertEnv() {
//     std::vector<std::string> env;
//     for (size_t i = 0; m_env[i]; i++)
//         env.push_back(m_env[i]);
//     return env;
// }

// void Request::doCGI(size_t end_header, std::string& request) {
//     std::vector<std::string> env = convertEnv();
//     char *av[] = { (char *)"./cgi-bin/change_color.sh", NULL };
//     int fd[2];

//     env.push_back("REQUEST_METHOD=" + m_methode);
//     env.push_back("CONTENT_LENGTH=" + request.substr(end_header).size());
//     env.push_back("SERVER_PROTOCOL=HTTP/1.1");

//     std::vector<char *> new_env;
//     for (size_t i = 0; i < env.size(); ++i)
//         new_env.push_back(const_cast<char *>(env[i].c_str()));
//     new_env.push_back(NULL);

//     if (pipe(fd)) {
//         Logger::log(RED, "error pipe : pipe failed");
//         return ;
//     }

//     pid_t pid = fork();
//     if (pid == 0) {
//         // dup2();
//         execve("./cgi-bin/change_color.sh", av, new_env.data());
//     }
// }

bool Request::methodePost(std::vector<std::string>&, std::string& full_request) {
    size_t header_end = full_request.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return setError(ERROR_400, "header not complete");

    if (m_url.find("cgi-bin") == 0) {
        doCGI(header_end + 4, full_request);
        return true;
    }

    std::string headers = full_request.substr(0, header_end);
    std::string body = full_request.substr(header_end + 4);

    m_content_length = getHeader("Content-Length");
    m_content_type = getHeader("Content-Type");
    if (m_content_length.empty() || std::atoi(m_content_length.c_str()) == 0)
        return setError(ERROR_411, "missing or zero Content-Length");

    std::string filename, file_data;
    if (!parseBody(m_content_type, full_request, filename, file_data))
        return false;
    m_responseStatus = HEADER_201;
    writeFile(filename, file_data);
	return true;
}

bool Request::methodePut(std::vector<std::string>&, std::string& full_request) {
    size_t header_end = full_request.find("\r\n\r\n");
    if (header_end == std::string::npos)
        return setError(ERROR_400, "header not complete");

    std::string headers = full_request.substr(0, header_end);
    std::string body = full_request.substr(header_end + 4);

    m_content_length = getHeader("Content-Length");
    m_content_type = getHeader("Content-Type");
    if (m_content_length.empty() || std::atoi(m_content_length.c_str()) == 0)
        return setError(ERROR_411, "missing or zero Content-Length");

    std::string filename, file_data;
    if (!parseBody(m_content_type, full_request, filename, file_data))
        return false;
    m_responseStatus = HEADER_201;
    writeFile(filename, file_data);
	return true;
}

bool Request::methodeGet(std::vector<std::string>& tab, std::string&) {
    for (size_t i = 1; i < tab.size(); ++i) {
        std::vector<std::string> words = split(tab[i], " ");
        if (words.size() > 1) {
            if (words[0] == "Content-Length:")
                m_content_length = words[1];
            else if (words[0] == "Content-Type:")
                m_content_type = words[1];
        }
    }
	return true;
}

bool Request::methodeDelete(std::vector<std::string>&, std::string&) {
    std::cout << "coucou Delete\n";
	return true;
}

bool Request::writeFile(std::string& filename, std::string& file_data) {
    std::string path = m_loc.getPath() + "/" + filename;
    if (path[0] == '/')
        path.erase(0, 1);

    if (access(path.c_str(), F_OK) == 0 && access(path.c_str(), W_OK) != 0)
        return setError(ERROR_500, "permission denied to open " + filename);
    if (access(path.c_str(), F_OK) == 0)
        m_responseStatus = HEADER_OK;

    std::ofstream out(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!out.is_open())
        return setError(ERROR_404, "the location path not existing upload failed !");

    out.write(file_data.data(), file_data.size());
    out.close();
	return true;
}

// bool Request::parseBody(const std::string& content_type, const std::string& body, std::string& filename, std::string& file_data) {
//     std::string boundary_key = "boundary=";
//     size_t b_pos = content_type.find(boundary_key);
//     if (b_pos == std::string::npos)
//         return setError(ERROR_400, "boundary not found in header");

//     std::string boundary = "--" + content_type.substr(b_pos + boundary_key.size());
//     boundary.erase(boundary.find_last_not_of("\r\n") + 1);

//     size_t part_start = body.find(boundary);
//     if (part_start == std::string::npos)
//         return setError(ERROR_400, "boundary not found in body");
//     part_start += boundary.length() + 2;

//     size_t part_header_end = body.find("\r\n\r\n", part_start);
//     if (part_header_end == std::string::npos)
//         return setError(ERROR_400, "missing Content-disposition");

//     std::string part_header = body.substr(part_start, part_header_end - part_start);
//     size_t fname_pos = part_header.find("filename=\"");
//     if (fname_pos != std::string::npos) {
//         fname_pos += 10;
//         size_t fname_end = part_header.find("\"", fname_pos);
//         if (fname_end != std::string::npos)
//             filename = part_header.substr(fname_pos, fname_end - fname_pos);
//     }

//     std::string end_boundary = boundary + "--";
//     size_t data_start = part_header_end + 4;
//     size_t data_end = body.find(end_boundary, data_start);
//     if (data_end == std::string::npos)
//         return setError(ERROR_400, "end boundary not found");
//     if (data_end >= 2 && body.substr(data_end - 2, 2) == "\r\n")
//         data_end -= 2;

//     file_data = body.substr(data_start, data_end - data_start);
//     return true;
// }

bool Request::parseBody(const std::string& content_type, const std::string& body, std::string& filename, std::string& file_data) {
{
	std::vector<std::string> content = split(content_type, ";");
	std::string boundary;
	
	for (std::vector<std::string>::iterator it = content.begin(); it != content.end(); ++it)
	{
		std::string value = *it;

		if (value.find("boundary=") != std::string::npos)
		{
			
		}
	}
	return true;
}

bool Request::setError(std::string code, const std::string& msg) {
    Logger::log(WHITE, "error request build: %s", msg.c_str());
    m_errorPage = true;
    m_responseStatus = code;
    return false;
}

std::string Request::getHeader(const std::string& name) const {
    std::map<std::string, std::string>::const_iterator it = m_headers.find(name);
    if (it != m_headers.end())
        return it->second;
    return "";
}

void Request::parseHeaders(const std::vector<std::string>& request) {
    for (size_t i = 1; i < request.size(); ++i) {
        const std::string& line = request[i];
        if (line.empty()) break;
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string name = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            size_t start = value.find_first_not_of(" \t");
            size_t end = value.find_last_not_of(" \t");
            if (start != std::string::npos && end != std::string::npos)
                value = value.substr(start, end - start + 1);
            m_headers[name] = value;
        }
    }
}

std::ostream& operator<<(std::ostream& o, Request& req) {
    o << "=================================\n" << "Method: " << req.getMethod() << "\nContent-Length: " << req.getContentLength() << 
    "\nContent-Type: " << req.getContentType() <<
    "\nUrl: " << req.getUrl() << 
    "\nVersion: " << req.getVersion() <<
    "\nBody: " << req.getBody() <<
    "\n------------------\n" << req.getLocation() << "\n----------------------------------------------\n" << "=================================";
    return o;
}