/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 10:13:39 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/23 20:59:21 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.h"
#include "Parser.h"
#include "Logger.h"



Request::Request(Location loc, std::vector<std::string>& firstRequestLine, std::vector<std::string>& request, std::string& full_request) : m_loc(loc), m_methode(firstRequestLine[0]), m_url(firstRequestLine[1]), m_version(firstRequestLine[2]), m_foundBody(false) {

	std::string methode_name[5] = {"GET", "POST", "DELETE", "HEAD", "PUT"};
	void(Request::*fonction[])(std::vector<std::string>&, std::string&) = {&Request::methodeGet, &Request::methodePost, &Request::methodeDelete, &Request::methodeHead, &Request::methodePut};

	int j = -1;
	for (int i = 0; i < 5; i++) {
		if (methode_name[i].compare(m_methode) == 0)
			j = i;
	}
	
	if (j != -1) {
		if (m_methode == "POST")
			(this->*fonction[j])(request, full_request);
		else
			(this->*fonction[j])(request, m_methode);
	}
	else {
		Logger::log(RED, "methode %s not autorized error ...", m_methode.c_str());
		//send_error_400;
	}
}

Request::Request(const Request &copy) {
	*this = copy;
}

Request& Request::operator=(const Request &other) {
	if (this != &other) {
		m_loc = other.m_loc;
	}
	return *this;
}

void Request::methodePost(std::vector<std::string>& tab, std::string& full_request) {
	std::cout << "coucou Post\n";
	(void)tab;

	size_t header_end = full_request.find("\r\n\r\n");
	if (header_end == std::string::npos)
		Logger::log(WHITE, "error header");

	std::string headers = full_request.substr(0, header_end);
	std::string body = full_request.substr(header_end + 4);

	std::istringstream stream(headers);
	std::string line;
	while (std::getline(stream, line)) {
		if (line.find("Content-Length:") == 0)
			m_content_length = line.substr(15);
		if (line.find("Content-Type:") == 0)
			m_content_type = line.substr(13);
	}

	std::string boundary_key = "boundary=";
	size_t b_pos = m_content_type.find(boundary_key);
	if (b_pos == std::string::npos) return;
	std::string boundary = "--" + m_content_type.substr(b_pos + boundary_key.size());

	boundary.erase(boundary.find_last_not_of("\r\n") + 1);

	size_t part_start = body.find(boundary);
	if (part_start == std::string::npos)
		Logger::log(WHITE, "error to find boundary");
	part_start += boundary.length() + 2;

	size_t part_header_end = body.find("\r\n\r\n", part_start);
	if (part_header_end == std::string::npos)
		Logger::log(WHITE, "error find content disposition");
	std::string part_header = body.substr(part_start, part_header_end - part_start);

	std::string filename;
	size_t fname_pos = part_header.find("filename=\"");
	if (fname_pos != std::string::npos) {
		fname_pos += 10;
		size_t fname_end = part_header.find("\"", fname_pos);
		if (fname_end != std::string::npos)
			filename = part_header.substr(fname_pos, fname_end - fname_pos);
	}

	std::string end_boundary = boundary + "--";

	size_t data_start = part_header_end + 4;
	size_t data_end = body.find(end_boundary, data_start);

	if (data_end == std::string::npos)
		Logger::log(WHITE, "error request build: end boundary not found");

	if (data_end >= 2 && body.substr(data_end - 2, 2) == "\r\n")
		data_end -= 2;

	std::string file_data = body.substr(data_start, data_end - data_start);

	std::ofstream out(filename.c_str(), std::ios::binary);
	out.write(file_data.data(), file_data.size());
	out.close();
}


void Request::methodeGet(std::vector<std::string>& tab, std::string& request) {
	(void)request;
	for (std::vector<std::string>::iterator it = tab.begin() + 1; it != tab.end(); ++it)
	{
		std::string line = *it;

		if (m_foundBody)
			m_body += line + "\n";
		if (line.empty())
			m_foundBody = true;
		else
		{
			std::vector<std::string> words = split(line, " ");
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

void Request::methodeHead(std::vector<std::string>& tab, std::string& request) {
	(void)request;
	std::cout << "coucou Head\n";
	for (std::vector<std::string>::iterator it = tab.begin(); it != tab.end(); ++it)
		std::cout << *it << std::endl;
	(void)tab;
}

void Request::methodePut(std::vector<std::string>& tab, std::string& request) {
	(void)request;
	std::cout << "coucou Head\n";
	for (std::vector<std::string>::iterator it = tab.begin(); it != tab.end(); ++it)
		std::cout << *it << std::endl;
	(void)tab;
}

void Request::parseType(std::string& request) {
	(void)request;
}

void Request::parseLenght(std::string& request) {
	(void)request;
}

void Request::parseBody(std::string& request) {
	(void)request;
}

void Request::parseCGI(std::string& request) {
	(void)request;
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