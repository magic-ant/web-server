#ifndef WEB_SERVER_H
#define WEB_SERVER_H
#include <stdio.h>

#define LOG_LEVEL -1

#define BUFFER_SIZE 1024
#define EPOLLFD_SIZE 1024
#define MAX_CONNECTIONS 1024
#define BACKLOG 128

typedef int (*RECALLBACK)();

int init_socket(char* ip, char* port, int backlog);
int accept_cb(int fd);
int recv_cb(int fd);
int send_cb(int fd);

enum event_opt_t{
  ADD_EVENT,
  DEL_EVENT,
  MOD_EVENT
};
int set_events(int fd, int events, enum event_opt_t opt); 

typedef struct con_item_t{
  int fd;

  char* ip;
  short port;

  unsigned long start_time;
  
  int rlen;
  char rbuffer[BUFFER_SIZE];

  int wlen;
  char wbuffer[BUFFER_SIZE];

  union{
    RECALLBACK recv_cb;
    RECALLBACK accept_cb;
  }recv_t;
  
  RECALLBACK send_cb;

}con_item;

int server_create(int fd);
int server_wait(int fd);

#endif //WEB_SERVER_H
