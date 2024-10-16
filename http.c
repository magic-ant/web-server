#include "http.h"
#include "logger.h"
#include "server.h"
#include "status.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

int http_response(connection con){
  const char* index = "statics/index.html";
  int fd = open(index, O_RDONLY);
  if(-1 == fd){
    LOG_ERROR("open file %s error[%d]: %s", index, errno, strerror(errno));
    con->wlen = sprintf(con->wbuffer, "HTTP/1.1 404 Not Found\r\n\r\n");
    return STATUS_FAILUER;
  }
  struct stat file_stat;
  int ret = fstat(fd, &file_stat);
  if(-1 == ret){
    LOG_ERROR("get file stat %s error[%d]: %s", index, errno, strerror(errno));
    con->wlen = sprintf(con->wbuffer, "HTTP/1.1 500 Not Found\r\n\r\n");
    close(fd);
    return STATUS_FAILUER;
  }

  con->wlen = sprintf(con->wbuffer, 
                      "HTTP/1.1 200 OK \r\n"
                      "Accept-Ranges: bytes\r\n"
                      "Content-Length: %ld\r\n"
                      "Content-Type: text/html\r\n\r\n", file_stat.st_size);
  int count = read(fd, con->wbuffer + con->wlen, BUFFER_SIZE - con->wlen);
  con->wlen += count;
  close(fd);
  return STATUS_SUCCESS;
}
