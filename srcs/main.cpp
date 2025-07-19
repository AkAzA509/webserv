/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ggirault <ggirault@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/19 12:11:10 by ggirault          #+#    #+#             */
/*   Updated: 2025/07/19 12:11:11 by ggirault         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.h"

volatile sig_atomic_t sig = 0;

int main(int ac, char **av)
{
	Config config(ac, av);
	config.launchServers();
}