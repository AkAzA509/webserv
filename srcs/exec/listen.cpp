/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   listen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:25:18 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/23 19:25:06 by ggirault         ###   ########.fr       */
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

bool isMethodeValid(std::string& methode) {
	return (methode == "GET" || methode == "POST" || methode == "DELETE" || methode == "HEAD");
}

void Server::parseRequest(std::string& request) {
	std::vector<std::string> request_lines = splitRequest(request);
	
	if (request_lines.empty()) {
		Logger::log(RED, "error request empty error ...!");
		return;
	}
	
	std::vector<Location>::iterator it = m_locations.begin();

	std::vector<std::string> words = split(request_lines[0], " ");
	
	while (it != m_locations.end()) {
		if (words[1] == it->getPath())
			break;
		++it;
	}
	if (it == m_locations.end()) {
		Logger::log(RED, "error bad request error ... !");
		return;
	}
	Request req(*it, words, request_lines, request);

	// std::cout << req << std::endl;
	//Response r;
}

// EPOLLIN : met l'event socket en mode lecture
void addEpollServer(std::vector<int> fd, int epfd) {
	for (std::vector<int>::iterator it = fd.begin(); it != fd.end(); ++it) {
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = *it;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, *it, &event) == -1) {
			close(epfd);
			print_error("epoll_ctl server ADD failed", fd);
		}
	}
}

// EPOLLIN : met l'event socket en mode lecture
// EPOLLOUT : met le socket en mode ecriture
void addEpollClient(int client_fd, int epfd, std::vector<int> fd) {
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLOUT;
	event.data.fd = client_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		close(epfd);
		print_error("epoll_ctl client ADD failed", fd);
	}
}

//EAGAIN : signifie qu'il n'y a rien a lire
//EWOULDBLOCK : signifie qu'il n'y a rien a lire
//EINTR : interruption du signal
void Server::acceptClient(int ready, std::vector<int> socketFd, struct epoll_event *ev, int epfd) {
	for (int i = 0; i < ready; i++)
	{
		int fd = ev[i].data.fd;
		if (std::find(socketFd.begin(), socketFd.end(), fd) != socketFd.end()) {
			struct sockaddr_in client_addr;
			socklen_t client_len = sizeof(client_addr);
			std::string request;

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
			addEpollClient(client_fd, epfd, socketFd);
			recvClient(epfd, socketFd, ev[i], request);
			parseRequest(request);
		}
	}
}

// void Server::waitConnection() {
// 	int epfd = epoll_create(10), ready = 0;
// 	if (epfd < 0)
// 		print_error("epoll init fail", m_socketFd);

// 	addEpollServer(m_socketFd, epfd);

// 	signal(SIGINT, sigint_handler);
// 	while(sig == 0) {
// 		struct epoll_event ev[MAX_EVENT];
// 		ready = epoll_wait(epfd, ev, MAX_EVENT, 1000);
// 		if (ready == 0)
// 			continue;
// 		if (ready == -1) {
// 			close(epfd);
// 			print_error("epoll_wait failed", m_socketFd);
// 		}
// 		acceptClient(ready, m_socketFd, ev, epfd);
// 	}
// }