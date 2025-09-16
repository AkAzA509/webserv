/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/19 12:11:10 by ggirault          #+#    #+#             */
/*   Updated: 2025/09/13 16:36:58 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.h"

volatile sig_atomic_t sig = 0;

int main(int ac, char **av, char **ep)
{
	std::vector<std::string> test(av, av + ac);
	Config config(ac, av, ep);
	config.launchServers();
}