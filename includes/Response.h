/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/05 17:45:21 by macorso           #+#    #+#             */
/*   Updated: 2025/08/05 17:14:16 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include <string>
#include <map>
#include <set>
#include "Request.h"
#include "Server.h"

// Default error page content
#define P_ERROR_400 "<html><head><title>400 Bad Request</title></head><body><h1>400 Bad Request</h1><p>Your browser sent a request that this server could not understand.</p></body></html>"
#define P_ERROR_403 "<html><head><title>403 Forbidden</title></head><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>"
#define P_ERROR_404 "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1><p>The requested URL was not found on this server.</p></body></html>"
#define P_ERROR_405 "<html><head><title>405 Method Not Allowed</title></head><body><h1>405 Method Not Allowed</h1><p>The requested method is not allowed for this URL.</p></body></html>"
#define P_ERROR_411 "<html><head><title>411 Length Required</title></head><body><h1>411 Length Required</h1><p>Content-Length header required for this request.</p></body></html>"
#define P_ERROR_500 "<html><head><title>500 Internal Server Error</title></head><body><h1>500 Internal Server Error</h1><p>The server encountered an internal error and was unable to complete your request.</p></body></html>"

#define HEADER_OK "HTTP/1.1 200 OK\r\n"
#define HEADER_201 "HTTP/1.1 201 Created\r\n"
#define HEADER_303 "HTTP/1.1 303 See Other\r\n"
#define ERROR_400 "HTTP/1.1 400 Bad request\r\n"
#define ERROR_403 "HTTP/1.1 403 Forbidden\r\n"
#define ERROR_404 "HTTP/1.1 404 Not Found\r\n"
#define ERROR_405 "HTTP/1.1 405 Method Not Allowed\r\n"
#define ERROR_411 "HTTP/1.1 411 Length Required\r\n"
#define ERROR_500 "HTTP/1.1 500 Internal Server Error\r\n"

#define CSS "text/css"
#define HTML "text/html"
#define JS "text/js"
#define PY "text/py"

#define CONTENT_TYPE "Content-Type: "
#define CONTENT_LENGHT "Content-Length: "
#define RETURN "\r\n"
#define LOCATION_ROOT "Location: / \r\n"
#define LOCATION_ROOT "Location: / \r\n"
#define CONNECTION_CLOSE "Connection: close\r\n"

class Request;
class Server;

class Response
{
	public:
		Response();
		Response(Request& req, Server& server);
		
		const std::string& getFirstLine() const;
		const std::map<std::string, std::string>& getHeaders() const;
		const std::string& getBody() const;
		std::string getFullResponse() const;

	private:
		void		buildResponse();
		void		setDefaultResponse();
		void		setErrorResponse(int errorCode);
		void		handleGet();
		void		handlePost();
		void		handleDelete();

		// Helper functions
		std::string	normalizePath(const std::string& path) const;
		std::string	buildPath(const std::string& page_path) const;
		std::pair<std::string, std::string> getError(int page, const std::string& page_path) const;

		Server*		m_server;
		Request*	m_request;
		std::string	m_firstline;
		std::map<std::string, std::string>	m_header;
		std::string	m_body;
};

#endif