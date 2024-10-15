#include "logger.h"
#include <time.h>

void log_message(const char *level, const char *file_name, int line, const char* format, ...){
  va_list args;
  va_start(args, format);

  time_t now = time(NULL);
  struct tm* cur_time = localtime(&now);

  fprintf(stderr, "%02d:%02d:%02d [%s] [%s %d]", cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec, level, file_name, line);
  vfprintf(stderr, format,  args);
  fprintf(stderr, "\n");

  va_end(args);
}

/*
int main(int args, char** argv){
  LOG_INFO("INFO LOGGING");
  LOG_DEBUG("DEBUG LOGGING");
  LOG_ERROR("ERROR LOGGING");
  LOG_WARN("WARNING LOGGING");
  LOG_NORM("NORM LOGGING");
  return 0;
}
*/
