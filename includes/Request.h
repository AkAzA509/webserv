/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/21 12:03:36 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/21 16:17:19 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class Request {
private:
	Location* m_loc;
public:
	Request() {};
	Request(Location* loc);
	~Request() {};
	Request(const Request& copy);
	Request& operator=(const Request& other);
	void parseRequest(std::string& request);
	void parseMethode(std::string& request);
	void parseType(std::string& request);
	void parseLenght(std::string& request);
	void parseBody(std::string& request);
	void parseCGI(std::string& request);
};
