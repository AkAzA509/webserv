/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_connection.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/02 10:21:09 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/02 17:12:48 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"
#include "../../includes/Config.h"
#include "../../includes/Parser.h"

void waitConnection(int sokFd, std::string& website, std::string& css) {
	while (1) {
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
	
		int client_fd = accept(sokFd, (struct sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) {
			perror("accept failed");
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

		if (cmp.find("GET /style.css") != std::string::npos) {
			to_send = css;
			type = "text/css";
		} else {
			to_send = website;
			type = "text/html";
		}

		sprintf(bite, "%zu", to_send.size());
		std::string len(bite);

		std::string msg = "HTTP/1.1 200 OK\r\n";
		msg += "Content-Type: " + type + "\r\n";
		msg += "Content-Length: " + len + "\r\n";
		msg += "Connection: close\r\n";
		msg += "\r\n";
		msg += to_send;

		ssize_t response = send(client_fd, msg.c_str(), msg.size(), 0);
		if (response < 0)
			perror("send failed");
		close(client_fd);
	}
}

std::string loadWebsite(const std::string& path) {
	int fd = open(path.c_str(), O_RDONLY);

	if (fd < 0) {
		perror("open");
		return NULL;
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

void setupSocket() {
	int sokFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sokFd < 0) {
		perror("socket");
		exit(1);
	}
	int opt = 1;
	if (setsockopt(sokFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
		close(sokFd);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(sokFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("binding");
		close(sokFd);
		exit(1);
	}
	if (listen(sokFd, SOMAXCONN) < 0) {
		perror("listening");
		close(sokFd);
		exit(1);
	}
	std::string website = loadWebsite("/home/ggirault/Documents/Sixth/webserv/test.html");
	std::string css = loadWebsite("/home/ggirault/Documents/Sixth/webserv/style.css");
	waitConnection(sokFd, website, css);
	close(sokFd);
}

int main(void) {
	setupSocket();
	return 0;
}