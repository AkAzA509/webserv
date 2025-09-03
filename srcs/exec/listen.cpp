#include "Server.h"
#include "Config.h"
#include "Parser.h"
#include "Logger.h"

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
            Logger::log(YELLOW, "Request received:\n%s\n=======================", client.request_buffer.c_str());
            return true;
        }

        if (client.request_buffer.size() > m_Client_max_body_size) {
            Logger::log(RED, "Request too large (max: %zu)", m_Client_max_body_size);
            cleanupClient(epfd, client_fd, ev);
            return false;
        }
        return false;
    }
    else if (query == 0) {
        Logger::log(YELLOW, "Client %d disconnected", client_fd);
        cleanupClient(epfd, client_fd, ev);
        return false;
    }
    else {
        Logger::log(RED, "recv error: %s", strerror(errno));
        cleanupClient(epfd, client_fd, ev);
        return false;
    }
}

void Server::sendClient(Response& response, int client_fd) {
    const std::string& resp_str = response.getFullResponse();

	Logger::log(CYAN, "Response: %s\n", resp_str.c_str());
    ssize_t sent = send(client_fd, resp_str.c_str(), resp_str.size(), 0);
    
    if (sent < 0) {
        Logger::log(RED, "send error: %s", strerror(errno));
    } else {
        Logger::log(CYAN, "Sent %zd bytes to client %d", 
                  sent, client_fd);
    }
}

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
            
            struct epoll_event client_ev;
            client_ev.events = EPOLLIN | EPOLLET;
            client_ev.data.fd = client_fd;
            if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &client_ev) < 0) {
                perror("epoll_ctl add client failed");
                close(client_fd);
                continue;
            }
            m_clients[client_fd] = ClientState();
            Logger::log(CYAN, "New client connected: %d", client_fd);
        }
        else {
            if (recvClient(epfd, ev[i], fd)) {
                std::string request = m_clients[fd].request_buffer;
                if (!request.empty()) {
                    try {
                        Request req(*this, request, m_ep);
                        Logger::log(CYAN, "Request parsed for client %d", fd);
                        Response resp(req, *this);
                        sendClient(resp, fd);
                    } catch (const std::exception& e) {
                        Logger::log(RED, "Error processing request: %s", e.what());
                        Response resp;
                        resp.setErrorResponse(500);
                        sendClient(resp, fd);
                    }
                }
                cleanupClient(epfd, fd, ev[i]);
            }
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
    while(sig == 0) {
        struct epoll_event ev[MAX_EVENT];
        int ready = epoll_wait(epfd, ev, MAX_EVENT, 1000);
        
        if (ready == -1) {
            if (errno == EINTR) continue;
            perror("epoll_wait failed");
            break;
        }
        if (ready > 0)
            acceptClient(ready, m_socketFd, ev, epfd);
    }
    
    for (std::map<int, ClientState>::iterator it = m_clients.begin(); it != m_clients.end(); ++it) {
        close(it->first);
    }
    for (size_t i = 0; i < m_socketFd.size(); ++i) {
        close(m_socketFd[i]);
    }
    close(epfd);
}

void Server::setupSocket() {
    int opt = 1;

    for (size_t i = 0; i < m_port.size() && !sig; ++i) {
        int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            print_error("socket creation failed", -1);
            continue;
        }

        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            print_error("setsockopt failed", sock_fd);
            close(sock_fd);
            continue;
        }

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(m_port[i]);
        addr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr))) {
            perror("bind failed");
            print_error("bind failed", sock_fd);
            close(sock_fd);
            continue;
        }

        if (listen(sock_fd, SOMAXCONN)) {
            print_error("listen failed", sock_fd);
            close(sock_fd);
            continue;
        }

        m_socketFd.push_back(sock_fd);
        Logger::log(CYAN, "Server listening on port %d", m_port[i]);
    }
}