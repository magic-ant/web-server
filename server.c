#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "server.h"
#include "logger.h"
#include "status.h"

con_item connlist[MAX_CONNECTIONS] = {0};
int epfd = -1;

int init_socket(char *ip, char *port, int backlog){
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(-1 == sockfd){
    LOG_ERROR("create sockfd failure, error[%d]: %s", errno, strerror(errno));
    return STATUS_FAILUER;
  }
  LOG_INFO("create socket successfully, sockfd: %d", sockfd);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(port));
  addr.sin_addr.s_addr = inet_addr(ip);
  if(-1 == bind(sockfd, (struct sockaddr*)&addr, sizeof(addr))){
    LOG_ERROR("socket %d bind with %s:%s failure, error[%d]: %s", sockfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), \
              errno, strerror(errno));
    close(sockfd);
    return STATUS_FAILUER;
  }
  LOG_INFO("socket %d bind with %s:%d successfully", sockfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

  if(-1 == listen(sockfd, backlog)){
    LOG_ERROR("socket %d listen failure, error[%d]: %s", sockfd, errno, strerror(errno));
    close(sockfd);
    return STATUS_FAILUER;
  }
  LOG_INFO("socket %d listen successfully, backlog: %d", sockfd, backlog);

  return sockfd;
}

int set_events(int fd, int events, enum event_opt_t opt){
 struct epoll_event ev;
 ev.data.fd = fd;
 ev.events = events;
 if(ADD_EVENT == opt){
   if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev)){
     LOG_ERROR("epoll ctl add socket %d failure, error[%d]: %s", fd, errno, strerror(errno));
     return STATUS_FAILUER;
   }
   LOG_DEBUG("epoll ctl add socket %d successfully", fd);
 }else if(MOD_EVENT == opt){
   if(-1 == epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev)){
     LOG_ERROR("epoll ctl mod socket %d failure, error[%d]: %s", fd, errno, strerror(errno));
     return STATUS_FAILUER;
   }
   LOG_DEBUG("epoll ctl mod socket %d successfully", fd);
 }else if(DEL_EVENT == opt){
   if(-1 == epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev)){
     LOG_ERROR("epoll ctl del socket %d failure, error[%d]: %s", fd, errno, strerror(errno));
     return STATUS_FAILUER;
   }
   LOG_DEBUG("epoll ctl del socket %d successfully", fd);
 }
 return STATUS_SUCCESS;
}

int accept_cb(int fd){
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  int sockfd = accept(fd, (struct sockaddr*)&addr, &len);
  if(-1 == sockfd){
    LOG_WARN("socket: %d accept client connection failure, error[%d]: %s", fd, errno, strerror(errno));
    return STATUS_FAILUER;
  }
  if(STATUS_FAILUER == set_events(sockfd, EPOLLIN, ADD_EVENT)){
    return STATUS_FAILUER;
  }
  connlist[sockfd].fd = sockfd;
  connlist[sockfd].ip = inet_ntoa(addr.sin_addr);
  connlist[sockfd].port = ntohs(addr.sin_port);
  connlist[sockfd].rlen = 0;
  memset(connlist[sockfd].rbuffer, 0, BUFFER_SIZE);
  connlist[sockfd].wlen = 0;
  memset(connlist[sockfd].wbuffer, 0, BUFFER_SIZE);
  connlist[sockfd].send_cb = send_cb;
  connlist[sockfd].recv_t.recv_cb = recv_cb;
  LOG_INFO("accept a connection socket %d from %s:%d", connlist[sockfd].fd, connlist[sockfd].ip, connlist[sockfd].port);
  return STATUS_SUCCESS;
}

int send_cb(int fd){
  int nbytes = send(fd, connlist[fd].wbuffer, connlist[fd].wlen, 0);
  if(-1 == nbytes){
    LOG_ERROR("socket %d send data error[%d]: %s", fd, errno, strerror(errno));
    return STATUS_FAILUER;
  }
  LOG_INFO("socket %d send %d bytes data successfully", fd, nbytes);
  set_events(fd, EPOLLIN, MOD_EVENT);
  if(connlist[fd].wlen >= BUFFER_SIZE){
    memset(connlist[fd].wbuffer, 0, BUFFER_SIZE);
    connlist[fd].wlen = 0;
  }
  return nbytes;
}


int recv_cb(int fd){
  int nbytes = recv(fd, connlist[fd].rbuffer + connlist[fd].rlen, BUFFER_SIZE - connlist[fd].rlen, 0);
  if(0 == nbytes){
    LOG_INFO("socket %d disclose, %s:%d", fd, connlist[fd].ip, connlist[fd].port);
    set_events(fd, EPOLLIN, DEL_EVENT);
    close(fd);
    memset(connlist + fd, 0, sizeof(con_item));
    return STATUS_SUCCESS;
  }else if(-1 == nbytes){
    LOG_ERROR("socket %d recv data from %s:%d failure, error[%d]: %s", fd, connlist[fd].ip, connlist[fd].port, errno, strerror(errno));
    return STATUS_FAILUER;
  }else{
    LOG_INFO("socket %d recv %d bytes: [%s] from %s:%d", fd, nbytes, connlist[fd].rbuffer + connlist[fd].rlen, \
              connlist[fd].ip, connlist[fd].port);
    memcpy(connlist[fd].wbuffer + connlist[fd].wlen, connlist[fd].rbuffer + connlist[fd].rlen, nbytes);
    connlist[fd].rlen += nbytes;
    connlist[fd].wlen += nbytes;
    if(connlist[fd].rlen >= BUFFER_SIZE){
      memset(connlist[fd].rbuffer, 0, BUFFER_SIZE);
      connlist[fd].rlen = 0;
    }
    set_events(fd, EPOLLOUT, MOD_EVENT);
    return nbytes;
  }
}


int main(int args, char** argv){
  assert(args == 4);
  
  int sockfd = init_socket(argv[1], argv[2], BACKLOG);
  if(STATUS_FAILUER == sockfd){
    LOG_ERROR("init socket error");
    EXIT_FAILURE;
  }

  connlist[sockfd].fd = sockfd;
  connlist[sockfd].recv_t.accept_cb = accept_cb;

  epfd = epoll_create(EPOLLFD_SIZE);
  if(-1 == epfd){
    LOG_ERROR("epoll create error");
    close(sockfd);
    EXIT_FAILURE;
  }
  LOG_INFO("create epoll successfully, epfd: %d", epfd);

  if( STATUS_SUCCESS != set_events(sockfd, EPOLLIN, ADD_EVENT)){
    LOG_ERROR("set server sockfd %d into epfd %d error[%d]: %s", sockfd, epfd, errno, strerror(errno));
    close(sockfd);
    close(epfd);
    EXIT_FAILURE;
  }

  struct epoll_event evs[EPOLLFD_SIZE] = {0};
  while(1){
    int nready = epoll_wait(epfd, evs, EPOLLFD_SIZE, -1);
    if(-1 == nready){
      LOG_ERROR("epoll wait error[%d]: %s", errno, strerror(errno));
      close(sockfd);
      close(epfd);
      EXIT_FAILURE;
    }
    for(int i = 0; i < nready; ++i){
      int ev_fd = evs[i].data.fd;
      if(evs[i].events & EPOLLIN){
        connlist[ev_fd].recv_t.recv_cb(ev_fd);
      }else if(evs[i].events & EPOLLOUT){
        connlist[ev_fd].send_cb(ev_fd);
      }else{
        LOG_WARN("epoll return socket %d status: %#X", ev_fd, evs[i].events);
      }
    }
  }

  close(sockfd);
  close(epfd);
  return 0;
}
