
# webserv

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-98-blue?logo=c%2B%2B&logoColor=white&style=for-the-badge" alt="C++98">
</p>

## Description

**webserv** is an HTTP server written in C++98, inspired by the 42 school project. It supports hosting multiple websites on different ports, handling HTTP requests (GET, POST, PUT, DELETE), executing CGI scripts, managing cookies and user sessions, file uploads, and advanced configuration via `.dlq` files.

## Main Features

- Multi-server, multi-port (host several servers on different addresses/ports)
- HTTP methods: GET, POST, PUT, DELETE
- Static file serving and file upload (PUT/POST)
- CGI script execution (Python, Bash, etc.)
- Cookie and session management
- Custom error pages (400, 403, 404, 405, 408, 411, 413, 500)
- Autoindex (directory listing if enabled)
- Advanced configuration via `.dlq` files
- Logging and detailed error handling

## Project Structure

- `bin/` : Compiled binaries
- `file/` : Static files, CGI scripts, error pages, etc.
- `includes/` : Project headers
- `obj/` : Compiled objects
- `srcs/` : C++ source files
- `test_configs/` : Example configuration files

## Build

```sh
make        # Production build
make dev    # Debug build with sanitizers
make clean  # Clean object files
make fclean # Clean objects + binaries
make re     # Full rebuild
```

## Run

```sh
./bin/webserv test_configs/website.dlq
```

## Example Configuration (`.dlq`)

```nginx
server {
    listen 8080;
    server_name localhost;
    root ./file;
    error_page 404 error_pages/404.html;
    location / {
        allow_methods GET POST;
        autoindex on;
    }
    location /cgi-bin {
        root ./file/cgi-bin;
        allow_methods GET POST;
        cgi_pass /usr/bin/bash;
        cgi_ext .sh;
    }
}
```

## Quick Endpoints & Tests

- Access `/`, `/cookie`, `/put_file`, `/post_file`, `/cgi-bin`, `/stylesheet` as configured
- Multi-server test:
  ```sh
  curl -s -w "%{http_code}\n" -o /dev/null http://localhost:8080/
  ```
- Unknown method test:
  ```sh
  curl -s -w "%{http_code}\n" -o /dev/null -X UNKNOW http://localhost:8080/
  ```

## Technical Details

- **C++98** only, manual memory management
- Uses `epoll` for efficient I/O multiplexing (multiple connections)
- Object-oriented architecture:
  - `Config`, `Parser`: configuration file parsing and validation
  - `Server`: socket, client, session, and timeout management
  - `Request`/`Response`: HTTP request parsing and response building
  - CGI support: secure execution of external scripts (Python, Bash, ...)
- Session and cookie management for user tracking
- Security: error handling, request size limits, timeouts, CGI isolation

## Dependencies

- Standard C++98
- Linux (uses epoll, sockets, etc.)

## Authors

- ggirault
- macorso

---
Project made for 42 school.
