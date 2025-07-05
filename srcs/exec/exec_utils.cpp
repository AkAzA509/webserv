/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:31:41 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/04 11:17:30 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"

void print_error(const std::string& str, int *fd) {
	for (size_t i = 0; fd[i] != -1; i++)
		close(fd[i]);
	throw std::runtime_error(str);
}

std::string loadFile(const std::string& path) {
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