/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   listen.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:25:18 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/19 14:42:49 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"
#include "../../includes/Config.h"
#include "../../includes/Parser.h"
#include "../../includes/Logger.h"

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


void recvClient(int epfd, std::vector<int> socketFd, struct epoll_event ev) {
	ssize_t query = -1;
	char buffer[4096];
	int client_fd = ev.data.fd;

	query = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
	if (query < 0) {
		if (query == EINTR || query == EWOULDBLOCK)
			print_error("recv failed", socketFd);
	}
	else if (query == 0) {
		epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev);
		close(client_fd);
		return;
	}
	else {
		buffer[query] = '\0';
		Logger::log(WHITE, "Requete : %s\n\n", buffer);
	}
}

//EAGAIN : signifie qu'il n'y a rien a lire
//EWOULDBLOCK : signifie qu'il n'y a rien a lire
//EINTR : interruption du signal
void acceptClient(int ready, std::vector<int> socketFd, struct epoll_event *ev, int epfd) {
	for (size_t i = 0; i < ready; i++)
	{
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
			addEpollClient(client_fd, epfd, socketFd);
			recvClient(epfd, socketFd, ev[i]);
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