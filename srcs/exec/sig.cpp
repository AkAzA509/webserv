/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sig.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:25:48 by ggirault          #+#    #+#             */
/*   Updated: 2025/09/29 15:42:25 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.h"
#include "Logger.h"

volatile bool g_sigok = true;

void sigint_handler(int) {
	sig = 1;
}

void sigterm_handler(int) {
	sig = 1;
}

void sigpipe_handler(int) {}

void sigchld_handler(int) {
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0) {
		if (WIFEXITED(status)) {
			int exitStatus = WEXITSTATUS(status);
			if (exitStatus != 0) {
				Logger::log(RED, "Child process exited with status %d", exitStatus);
				g_sigok = false;
			}
		} else if (WIFSIGNALED(status)) {
			int termSig = WTERMSIG(status);
			Logger::log(RED, "Child process terminated by signal %d", termSig);
			g_sigok = false;
		}
	}
}