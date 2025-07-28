/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/28 14:01:22 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/28 14:28:04 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"
#include "../../includes/Config.h"
#include "../../includes/Parser.h"
#include "../../includes/Logger.h"

void Server::cleanupClient(int epfd, int client_fd, struct epoll_event ev) {
	epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
	close(client_fd);
	m_clients.erase(client_fd);
}

std::string Server::getClientRequest(int client_fd) {
	std::map<int, ClientState>::iterator it = m_clients.find(client_fd);
	if (it != m_clients.end() && it->second.request_complete)
		return it->second.request_buffer;
	return "";
}

void Server::sendClient(Response& resp, int client_fd) {
	std::string response = resp.getResponse();

	ssize_t r_resp = send(client_fd, response.c_str(), response.size(), 0);
	if (r_resp < 0)
		Logger::log(RED, "send failed");
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
void addEpollClient(int client_fd, int epfd, std::vector<int> fd) {
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = client_fd;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
		close(epfd);
		print_error("epoll_ctl client ADD failed", fd);
	}
}

void print_error(const std::string& str, std::vector<int> fd) {
	for (std::vector<int>::iterator it = fd.begin(); it != fd.end(); ++it)
		close(*it);
	throw std::runtime_error(str);
}

std::string loadFile(const std::string& path) {
	if (path.empty() || access(path.c_str(), F_OK | R_OK) < 0)
		return "";

	int fd = open(path.c_str(), O_RDONLY);

	if (fd < 0) {
		perror("open");
		return "";
	}
	std::string content;
	char buffer[4096];
	ssize_t bytesRead;
	while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
		content.append(buffer, bytesRead);
	if (bytesRead < 0)
		perror("read");
	close(fd);
	return content;
}

bool Location::isAllowedMethode(std::string methode) {
	for (std::map<std::string, void (*)(Request&, Response&)>::iterator it = m_allowed_methods.begin(); it != m_allowed_methods.end(); ++it) {
		if (it->first == methode)
			return true;
	}
	return false;	
}