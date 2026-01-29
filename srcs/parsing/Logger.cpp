#include "Logger.h"

std::string Logger::getCurrentTime()
{
	char date[1000];

	time_t now = std::time(NULL);
	struct tm tm = *std::localtime(&now);


	tm.tm_hour = tm.tm_hour;
	std::strftime(date, sizeof(date), "[%Y-%m-%d  %H:%M:%S]   ", &tm);
	return (std::string(date));
}

void Logger::log(const char* color, const char* msg, ...)
{
	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];

	std::va_list list;
	va_start(list, msg);

	std::vsnprintf(buffer, BUFFER_SIZE, msg, list);
	va_end(list);

	std::cout << color << getCurrentTime() << buffer << RESET << std::endl;
}
