/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:27:25 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/05 14:55:55 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"

Server::Server() {
	for (size_t i = 0; i < 1024; i++)
		m_socketFD[i] = -1;
}

Server::~Server() {
	for (size_t i = 0; i < 1024; i++) {
		if (m_socketFD[i] != -1)
			close(m_socketFD[i]);
	}
}