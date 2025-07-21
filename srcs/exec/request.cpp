/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 10:13:39 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/21 16:21:13 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.h"
#include "Logger.h"
#include "Request.h"
#include "Parser.h"

Request::Request(Location* loc) : m_loc(loc)
{
	if (m_loc == NULL) throw std::runtime_error("Didn't find location needed by the Request");
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

Request::Request(const Request &copy) {
	*this = copy;
}

Request& Request::operator=(const Request &other) {
	if (this != &other) {
		
	}
	return *this;
}

void Request::parseMethode(std::string& request) {
	std::stringstream ss(request);
	std::string word;
	std::string url;
	ss >> word;
	ss >> url;
	
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

void Server::parseRequest(std::string& request) {
	size_t url_pos = request.find("/");
	size_t end_url = request.find(" ", url_pos);
	if (url_pos == std::string::npos || end_url == std::string::npos)
		return;
	std::string path = request.substr(url_pos, end_url - url_pos);

	Location* locToParse = NULL;
	for (std::vector<Location>::iterator it = m_locations.begin(); it != m_locations.end(); ++it) {
		if (path == it->getPath())
		{
			locToParse = it.base();
			break;
		}
	}
	try
	{
		Request request(locToParse);
		Response r;
		map[request.getMethod()](request, r);

		r.ge
	}
	catch (const std::exception& e)
	{
		Logger::log(RED, "Error: %s\n", e.what());
	}
	// parseMethode(request);
	// parseType(request);
	// parseLenght(request);
	// parseBody(request);
	// parseCGI(request);
}

// Check si j'ai le crlf dans la requete et pas de content lenght donc requete finie
// Si j'ai le crlf et un body lenght je check si j'ai deja tout les octets pour verifier
// la fin de la requete ou non
bool Server::requestComplete(std::string& request) {
	size_t crlf_pos = request.find("\r\n\r\n");
	size_t content_lenght = request.find(CONTENT_LENGHT);

	if (crlf_pos != std::string::npos && content_lenght == std::string::npos)
		return true;
	else if (crlf_pos != std::string::npos && content_lenght != std::string::npos) {
		size_t body_size = 0;

		std::stringstream ss(request.substr(content_lenght + strlen(CONTENT_LENGHT)));
		ss >> body_size;

		if (request.substr(crlf_pos + strlen("\r\n\r\n")).size() == body_size)
			return true;
		else
			return false;
	}
	return false;
}

void Server::recvClient(int epfd, std::vector<int> socketFd, struct epoll_event ev, std::string& request) {
	ssize_t query = -1;
	char buffer[4096];
	int client_fd = ev.data.fd;

	while (1) {
		query = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
		if (query > 0) {
			request.append(buffer, query);
			if (requestComplete(request)) {
				Logger::log(WHITE, "Requete : %s\n=======================", request.c_str());
				break;
			}
			continue;
		}
		else if (query < 0) {
			if (errno == EINTR || errno == EWOULDBLOCK) 
				break;
			else {
				epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
				close(client_fd);
				print_error("recv failed", socketFd);
			}
		}
		else if (query == 0) {
			epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
			close(client_fd);
			return;
		}
	}
}