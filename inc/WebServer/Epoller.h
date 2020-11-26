#ifndef EPOLLER_H
#define EPOLLER_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

class Epoller {
 public:
  explicit Epoller(int max_events = 1024);
  ~Epoller();
  bool add_fd(int fd, uint32_t events);
  bool mod_fd(int fd, uint32_t events);
  bool del_fd(int fd);
  int wait(int time_out = -1);
  int get_event_fd(size_t i) const;
  uint32_t get_event(size_t i) const;

 private:
  int epoll_fd;
  std::vector<struct epoll_event> events;
};

#endif
