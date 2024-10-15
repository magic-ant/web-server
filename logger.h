#ifndef SERVER_LOGGER_H
#define SERVER_LOGGER_H

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_WARNING
#endif //LOG_LEVEL

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

enum logger_level_t{
  LOG_LEVEL_NORM,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG
};


void log_message(const char* level, const char* file_name, int line, const char* format, ...);

#define LOG_DEBUG(format, ...) do{ if(LOG_LEVEL >= LOG_LEVEL_DEBUG) log_message("DEBUG", __FILE__, __LINE__, format, ##__VA_ARGS__);}while(0)
#define LOG_INFO(format, ...) do{if(LOG_LEVEL >= LOG_LEVEL_INFO) log_message("INFO", __FILE__, __LINE__, format, ##__VA_ARGS__);}while(0)
#define LOG_WARN(format, ...) do{if(LOG_LEVEL >= LOG_LEVEL_WARNING) log_message("WARNING", __FILE__, __LINE__, format, ##__VA_ARGS__);}while(0)
#define LOG_ERROR(format, ...) do{if(LOG_LEVEL >= LOG_LEVEL_ERROR) log_message("ERROR", __FILE__, __LINE__, format, ##__VA_ARGS__);}while(0)
#define LOG_NORM(format, ...) do{if(LOG_LEVEL >= LOG_LEVEL_NORM) log_message("NORM", __FILE__, __LINE__, format, ##__VA_ARGS__);}while(0)


#endif //SERVER_LOGGER_H
