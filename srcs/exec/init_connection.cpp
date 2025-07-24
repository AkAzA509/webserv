/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_connection.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 10:21:09 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/24 11:59:44 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.h"

void Server::waitConnection() {
	while (sig != 1) {
		signal(SIGINT, sigint_handler);
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
	
		int client_fd = accept(m_socketFd[0], (struct sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) {
			if (errno == EINTR)
				break;
			print_error("accept failed", m_socketFd);
			continue;
		}
		ssize_t query = -1;
		char buffer[4096];
		std::string request;
		while (1) {
			query = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
			if (query > 0) {
				request.append(buffer, query);
				if (requestComplete(request)) {
					//Logger::log(WHITE, "Requete : %s\n=======================", request.c_str());
					break;
				}
				continue;
			}
			else if (query < 0) {
				if (errno == EINTR || errno == EWOULDBLOCK) 
					break;
				else
					close(client_fd);
			}
			else if (query == 0) {
				close(client_fd);
				continue;
			}
		}
		printf("Reçu : %s\n", request.c_str());
		std::cout << "len = " << request.size() << std::endl;
		std::string to_send;
		std::string type;
		char bite[20];
		
		std::string website = loadFile("html/index.html");
		if (website.empty())
			website = loadFile("html/error_404.html");
		std::string css = loadFile("html/style.css");

		if (request.find("GET /style.css") != std::string::npos) {
			to_send = css;
			type = "text/css";
		} else if (request.find("GET /") != std::string::npos) {
			to_send = website;
			type = "text/html";
		} else if (request.find("POST /") != std::string::npos) {
			parseRequest(request);
		}

		sprintf(bite, "%zu", to_send.size());
		std::string len(bite);

		std::string msg = HEADER_OK;
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