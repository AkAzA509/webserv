/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_connection.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 10:21:09 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/03 17:51:14 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"
#include "../../includes/Config.h"
#include "../../includes/Parser.h"

volatile sig_atomic_t stop = 0;

void sigint_handler(int) {
	stop = 1;
}

std::string loadWebsite(const std::string& path) {
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

void Server::waitConnection() {
	while (stop != 1) {
		signal(SIGINT, sigint_handler);
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
	
		int client_fd = accept(m_socketFD[0], (struct sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) {
			if (errno == EINTR)
				break;
			print_error("accept failed", m_socketFD);
			continue;
		}
		ssize_t query = -1;
		char buffer[4096];
		//while (query != 0) {
			query = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
			if (query < 0) {
				perror("recv failed");
			} else if (query == 0) {
				printf("Client déconnecté\n");
			} else {
				buffer[query] = '\0';
				printf("Reçu : %s\n", buffer);
			}
		//}
		std::string cmp(buffer);
		std::string to_send;
		std::string type;
		char bite[20];
		
		std::string website = loadWebsite("../../index.html");
		if (website.empty())
		website = loadWebsite("../../error_404.html");
		std::string css = loadWebsite("../../style.css");

		if (cmp.find("GET /style.css") != std::string::npos) {
			to_send = css;
			type = "text/css";
		} else {
			to_send = website;
			type = "text/html";
		}
		
		sprintf(bite, "%zu", to_send.size());
		std::string len(bite);

		std::string msg = HEADER;
		msg += CONTENT_TYPE + type + RETURN;
		msg += CONTENT_LENGHT + len + RETURN;
		msg += CONNECTION_CLOSE;
		msg += RETURN;
		msg += to_send;

		ssize_t response = send(client_fd, msg.c_str(), msg.size(), 0);
		if (response < 0)
			perror("send failed");
		close(client_fd);
	}
}

void print_error(const std::string& str, int *fd) {
	for (size_t i = 0; fd[i] != -1; i++)
		close(fd[i]);
	throw std::runtime_error(str);
}

void Server::setupSocket() {
	int opt = 1, i = 0;

	for (std::vector<int>::iterator it = port.begin(); it != port.end() && !stop; ++it, ++i) {
		int sokFd = socket(AF_INET, SOCK_STREAM, 0);
		if (sokFd < 0)
			print_error("Error creation socket", m_socketFD);
		if (setsockopt(sokFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
			print_error("Error socket init", m_socketFD);

		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(static_cast<uint16_t>(*it));
		addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(sokFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			perror("bind =");
			print_error("binding fail", m_socketFD);
		}
		if (listen(sokFd, SOMAXCONN) < 0)
			print_error("listening", m_socketFD);

		m_socketFD[i] = sokFd;
	}
}

Server::Server() {
	for (size_t i = 0; i < 1024; i++)
		m_socketFD[i] = -1;
	port.push_back(8080);
}

void Server::clean() {
	for (size_t i = 0; i < 1024; i++) {
		if (m_socketFD[i] != -1)
			close(m_socketFD[i]);
	}
}

int main(void) {
	Server *test = new Server();
	signal(SIGINT, sigint_handler);
	try
	{
		test->setupSocket();
		test->waitConnection();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}
	test->clean();
	delete test;
	return 0;
}