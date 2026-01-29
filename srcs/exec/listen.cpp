#include "Server.h"
#include "Config.h"
#include "Parser.h"
#include "Logger.h"

// Enleve le client de epoll et ferme le socket
void Server::cleanupClient(int epfd, int client_fd, struct epoll_event ev) {
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, client_fd, &ev) < 0)
		Logger::log(RED, "epoll_ctl DEL failed for fd %d", client_fd);
	
	if (close(client_fd))
		Logger::log(RED, "close failed for fd %d", client_fd);

	m_clients.erase(client_fd);
}

// Set une reponse a envoyer sans requete valide pour bypass le parser etc
void Server::setErrorForced(int error_code, int client_fd, int epfd, struct epoll_event ev) {
	Request req;
	Response resp(req, client_fd, *this);
	resp.setErrorResponse(error_code);
	sendClient(resp, client_fd, epfd, ev);
}

// check si la requette est complete
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
	}
	catch (const std::exception& e) {
		Logger::log(RED, "error content-length : content_lenght invalide");
		return false;
	}
}

// Enregistre les donnees envoyer par le client sur sa socket a chaque
// notification d'event epoll, ajoute ses requettes dans un struct qui contient
// toute les infos de chaques clients
bool Server::recvClient(int epfd, struct epoll_event ev, int client_fd) {
	char buffer[4096];

	if (m_clients.find(client_fd) == m_clients.end())
		m_clients[client_fd] = ClientState();

	ClientState& client = m_clients[client_fd];

	if (client.request_complete)
		return true;

	ssize_t query = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (query > 0) {
		client.last_activity = getCurrentTimeMs();
		buffer[query] = '\0';
		client.request_buffer.append(buffer, query);
		if (requestComplete(client.request_buffer)) {
			// Logger::log(YELLOW, "Request %s", client.request_buffer.c_str());
			client.request_complete = true;
			return true;
		}

		if (client.request_buffer.size() > m_Client_max_body_size) {
			Logger::log(RED, "Request too large (max: %zu)", m_Client_max_body_size);
			setErrorForced(413, client_fd, epfd, ev);
			cleanupClient(epfd, client_fd, ev);
			return false;
		}
		return false;
	}
	else if (query == 0) {
		cleanupClient(epfd, client_fd, ev);
		return false;
	}
	else {
		Logger::log(RED, "recv error: %s", strerror(errno));
		cleanupClient(epfd, client_fd, ev);
		return false;
	}
}

// envoie la reponse au client
void Server::sendClient(Response& response, int client_fd, int epfd, struct epoll_event ev) {
	const std::string& resp_str = response.getFullResponse();

	int flags = fcntl(client_fd, F_GETFL, 0);
	bool wasNonBlocking = (flags != -1) && (flags & O_NONBLOCK);
	if (wasNonBlocking)
		fcntl(client_fd, F_SETFL, flags & ~O_NONBLOCK);

	const char* buf;
	size_t len;
	std::string const* src;
	if (m_forcedResponse.empty())
		src = &resp_str;
	else
		src = &m_forcedResponse;
	buf = src->c_str();
	len = src->size();

	size_t totalSent = 0;
	while (totalSent < len) {
		ssize_t n = send(client_fd, buf + totalSent, len - totalSent, MSG_NOSIGNAL);
		if (n > 0) {
			totalSent += static_cast<size_t>(n);
			continue;
		}
		Logger::log(RED, "send error: %s", strerror(errno));
		break;
	}

	if (wasNonBlocking)
		fcntl(client_fd, F_SETFL, flags);

	if (totalSent < len) {
		Logger::log(RED, "Failed to send all response to the client");
		cleanupClient(epfd, client_fd, ev);
		return;
	}

	m_clients[client_fd].last_activity = getCurrentTimeMs();
}

// Accepte les nouvelles connexions et gere les clients existants
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
			if (flags != -1)
				fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
			
			struct epoll_event client_ev;
			client_ev.events = EPOLLIN;
			client_ev.data.fd = client_fd;
			if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0) {
				perror("epoll_ctl add client failed");
				close(client_fd);
				continue;
			}
			m_clients[client_fd] = ClientState();
			char ip_buffer[INET_ADDRSTRLEN];
			if (inet_ntop(AF_INET, &client_addr.sin_addr, ip_buffer, sizeof(ip_buffer)))
				m_clients[client_fd].client_ip = ip_buffer;
			Logger::log(GREEN, "New client connected from %s on fd %d", ip_buffer, client_fd);
		}
		else {
			if (recvClient(epfd, ev[i], fd)) {
				std::string request = m_clients[fd].request_buffer;
				if (!request.empty()) {
					try {
						Request req(*this, request, m_ep);
						Response resp(req, fd, *this);
						sendClient(resp, fd, epfd, ev[i]);
					} catch (const std::exception& e) {
						Logger::log(RED, "Error processing request: %s", e.what());
						Response resp;
						resp.setErrorResponse(500);
						sendClient(resp, fd, epfd, ev[i]);
					}
				}
				cleanupClient(epfd, fd, ev[i]);
			}
		}
	}
	checkTimeouts(epfd);
}

void Server::checkTimeouts(int epfd) {
	unsigned long now_ms = getCurrentTimeMs();
	cleanupSessions(now_ms);
	std::vector<int> clients_to_timeout;

	for (std::map<int, ClientState>::iterator it = m_clients.begin(); it != m_clients.end(); ++it) {
		int client_fd = it->first;
		const ClientState& client = it->second;

		if (!client.request_complete && now_ms - client.last_activity > m_timeout)
			clients_to_timeout.push_back(client_fd);
	}

	for (size_t i = 0; i < clients_to_timeout.size(); ++i) {
		int client_fd = clients_to_timeout[i];
		if (m_clients.find(client_fd) != m_clients.end()) {
			struct epoll_event dummy_ev;
			dummy_ev.data.fd = client_fd;
			setErrorForced(408, client_fd, epfd, dummy_ev);
			cleanupClient(epfd, client_fd, dummy_ev);
		}
	}
}

void Server::waitConnection() {
	int epfd = epoll_create(10);
	if (epfd < 0) {
		print_error("epoll creation failed", -1);
		return;
	}

	for (size_t i = 0; i < m_socketFd.size(); ++i) {
		struct epoll_event ev;
		ev.events = EPOLLIN;
		ev.data.fd = m_socketFd[i];
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, m_socketFd[i], &ev) < 0) {
			print_error("epoll_ctl add server failed", m_socketFd[i]);
			close(epfd);
			return;
		}
	}

	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigterm_handler);
	signal(SIGPIPE, sigpipe_handler);
	signal(SIGCHLD, sigchld_handler);
	while(sig == 0) {
		struct epoll_event ev[MAX_EVENT];
		int ready = epoll_wait(epfd, ev, MAX_EVENT, 1000);
		
		if (ready == -1) {
			if (errno == EINTR)
				continue;
			perror("epoll_wait failed");
			break;
		}
		if (ready > 0)
			acceptClient(ready, m_socketFd, ev, epfd);
	}

	for (std::map<int, ClientState>::iterator it = m_clients.begin(); it != m_clients.end(); ++it)
		close(it->first);
	for (size_t i = 0; i < m_socketFd.size(); ++i)
		close(m_socketFd[i]);
	close(epfd);
}

void Server::setupSocket() {
	int opt = 1;

	for (size_t i = 0; i < m_port.size() && !sig; ++i) {

		addrinfo hints, *res;

		std::memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		std::stringstream ss;

		ss << m_port[i];

		int status = getaddrinfo(m_hostIp.c_str(), ss.str().c_str(), &hints, &res);

		if (status != 0) {
			std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
			continue;
		}

		int sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sock_fd < 0) {
			print_error("socket creation failed", -1);
			freeaddrinfo(res);
			continue;
		}

		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
			print_error("setsockopt failed", sock_fd);
			close(sock_fd);
			freeaddrinfo(res);
			continue;
		}
		
		if (bind(sock_fd, res->ai_addr, res->ai_addrlen) == -1) {
			freeaddrinfo(res);
			perror("bind failed");
			print_error("bind failed", sock_fd);
			close(sock_fd);
			continue;
		}

		if (listen(sock_fd, SOMAXCONN)) {
			print_error("listen failed", sock_fd);
			close(sock_fd);
			freeaddrinfo(res);
			continue;
		}

		freeaddrinfo(res);
		m_socketFd.push_back(sock_fd);
	}
}