/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: macorso <macorso@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/09 15:24:48 by macorso           #+#    #+#             */
/*   Updated: 2025/07/09 15:57:55 by macorso          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <cstdio>

#define RESET          "\x1B[0m"
#define RED            "\x1B[31m"
#define LIGHT_RED      "\x1B[91m"
#define WHITE          "\x1B[37m"
#define BLINK           "\x1b[5m"
#define YELLOW         "\x1B[33m"
#define LIGHT_BLUE     "\x1B[94m"
#define CYAN           "\x1B[36m"
#define DARK_GREY      "\x1B[90m"
#define LIGHTMAGENTA   "\x1B[95m"
#define GST             2;

class Logger
{
	public:
		static void log(const char* color, const char* msg, ...);
		static std::string getCurrentTime();
};