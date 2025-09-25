/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sig.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 10:25:48 by ggirault          #+#    #+#             */
/*   Updated: 2025/09/25 16:23:08 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/Server.h"

void sigint_handler(int) {
	sig = 1;
}

void sigterm_handler(int) {
	sig = 1; // Même comportement que SIGINT - arrêt propre
}

void sigpipe_handler(int) {
	// SIGPIPE ignoré - connexion fermée côté client
}

void sigchld_handler(int) {
	// Récupère les processus CGI terminés pour éviter les zombies
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0) {
		// Processus CGI terminé, nettoyage automatique
	}
}