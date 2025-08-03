/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   listen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:25:18 by ggirault          #+#    #+#             */
/*   Updated: 2025/08/02 10:56:40 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"
#include "../../includes/Config.h"
#include "../../includes/Parser.h"
#include "../../includes/Logger.h"

// Check si j'ai le crlf dans la requete et pas de content lenght donc requete finie
// Si j'ai le crlf et un body lenght je check si j'ai deja tout les octets pour verifier
// la fin de la requete ou non
bool Server::requestComplete(std::string& request) {
	size_t crlf_pos = request.find("\r\n\r\n");
	if (crlf_pos == std::string::npos)
		return false;

	std::string headers = request.substr(0, crlf_pos);
	std::transform(headers.begin(), headers.end(), headers.begin(), ::tolower);
	
	size_t content_len_pos = headers.find("content-length:");
	
	if (content_len_pos == std::string::npos)
		return true;

	size_t value_start = content_len_pos + 15;
	size_t line_end = headers.find("\r\n", value_start);
	if (line_end == std::string::npos)
		return false;

	std::string head_len = headers.substr(value_start, line_end - value_start);
	head_len.erase(0, head_len.find_first_not_of(" \t"));
	head_len.erase(head_len.find_last_not_of(" \t") + 1);
	
	try {
		size_t expected_body_size = std::atoi(head_len.c_str());
		size_t body_size = request.size() - (crlf_pos + 4);
		return body_size >= expected_body_size;
	} catch (const std::exception& e) {
		Logger::log(RED, "error content-length : content_lenght invalide");
		return false;
	}
}

bool Server::recvClient(int epfd, struct epoll_event ev, int client_fd) {
	char buffer[4096];

	if (m_clients.find(client_fd) == m_clients.end())
		m_clients[client_fd] = ClientState();

	ClientState& client = m_clients[client_fd];

	if (client.request_complete)
		return true;

	ssize_t query = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (query > 0) {
		buffer[query] = '\0';
		client.request_buffer.append(buffer, query);

		if (requestComplete(client.request_buffer)) {
			client.request_complete = true;
			Logger::log(YELLOW, "Request :\n%s\n=======================", client.request_buffer.c_str());
			return true;
		}
		
		if (client.request_buffer.size() > m_Client_max_body_size) {
			Logger::log(RED, "error request : request size overflow the max size in the config file");
			cleanupClient(epfd, client_fd, ev);
			return false;
		}
		return false;
	}
	else if (query == 0) {
		Logger::log(YELLOW, "Client %d close connection", client_fd);
		cleanupClient(epfd, client_fd, ev);
		return false;
	}
	else {
		Logger::log(RED, "error recv : failed to read request");
		cleanupClient(epfd, client_fd, ev);
		return false;
	}
}

Response Server::parseRequest(std::string& request) {
	std::vector<std::string> request_lines = splitRequest(request);

	if (request_lines.empty()) {
		// Logger::log(RED, "error request : empty request");
		std::string error = ERROR_400;
		Response resp(error, *this);
		return resp;
	}

	std::vector<Location>::iterator it = m_locations.begin();
	std::vector<std::string> words = split(request_lines[0], " ");

	// First try exact match
	while (it != m_locations.end()) {
		if (words[1] == it->getPath())
			break;
		++it;
	}
	
	// If no exact match found, try to find root location "/" as fallback
	if (it == m_locations.end()) {
		it = m_locations.begin();
		while (it != m_locations.end()) {
			if (it->getPath() == "/")
				break;
			++it;
		}
	}
	
	if (it == m_locations.end()) {
		Logger::log(RED, "error location : location %s not found in the config file", words[1].c_str());
		std::string error = ERROR_404;
		Response resp(error, *this);
		return resp;
	}
	Request req(*it, words, request_lines, request, m_ep);
	Response resp(req, *this);
	return resp;
}

//EAGAIN : signifie qu'il n'y a rien a lire
//EWOULDBLOCK : signifie qu'il n'y a rien a lire
//EINTR : interruption du signal
void Server::acceptClient(int ready, std::vector<int> socketFd, struct epoll_event *ev, int epfd) {
	for (int i = 0; i < ready; i++) {
		int fd = ev[i].data.fd;

		if (std::find(socketFd.begin(), socketFd.end(), fd) != socketFd.end()) {
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);

			int client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
			if (client_fd < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;
				else if (errno == EINTR)
					continue;
				else {
					perror("accept failed");
					break;
				}
			}
			int flags = fcntl(client_fd, F_GETFL, 0);
			if (flags != -1) {
				fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
			}
			addEpollClient(client_fd, epfd, socketFd);
		}
		else {
			if (recvClient(epfd, ev[i], fd)) {
				std::string request = getClientRequest(fd);
				if (!request.empty()) {
					Response resp = parseRequest(request);
					std::cout << "Response: " << resp.getResponse() << std::endl;
					sendClient(resp, fd);
					cleanupClient(epfd, fd, ev[i]);
				}
			}
			// Si request_ready == false, on attend juste le prochain événement epoll
		}
	}
}

void Server::waitConnection() {
	int epfd = epoll_create(10), ready = 0;
	if (epfd < 0)
		print_error("epoll init fail", m_socketFd);

	addEpollServer(m_socketFd, epfd);

	signal(SIGINT, sigint_handler);
	while(sig == 0) {
		struct epoll_event ev[MAX_EVENT];
		ready = epoll_wait(epfd, ev, MAX_EVENT, 1000);
		if (ready == 0)
			continue;
		if (ready == -1) {
			close(epfd);
			print_error("epoll_wait failed", m_socketFd);
		}
		acceptClient(ready, m_socketFd, ev, epfd);
	}
}

void Server::setupSocket() {
	int opt = 1, i = 0;

	for (std::vector<size_t>::iterator it = m_port.begin(); it != m_port.end() && !sig; ++it, ++i) {
		int sokFd = socket(AF_INET, SOCK_STREAM, 0);
		if (sokFd < 0)
			print_error("Error creation socket", m_socketFd);
		if (setsockopt(sokFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
			print_error("Error socket init", m_socketFd);

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(static_cast<uint16_t>(*it));
		addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(sokFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			perror("bind");
			print_error("binding fail", m_socketFd);
		}
		if (listen(sokFd, SOMAXCONN) < 0)
			print_error("listening", m_socketFd);

		m_socketFd.push_back(sokFd);
	}
}