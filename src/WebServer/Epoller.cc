#include "WebServer/Epoller.h"

Epoller::Epoller(int max_events) {
  this->epoll_fd = epoll_create(1024);
  this->events = std::vector<struct epoll_event>(max_events);
}

Epoller::~Epoller() {
  close(this->epoll_fd);
}

bool Epoller::add_fd(int fd, uint32_t events) {
  if (fd < 0)
    return false;

  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = events;
  return 0 == epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::mod_fd(int fd, uint32_t events) {
  if (fd < 0)
    return false;
  epoll_event ev = {0};
  ev.data.fd = fd;
  ev.events = events;

  return 0 == epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::del_fd(int fd) {
  if (fd < 0)
    return false;
  epoll_event ev = {0};
  return 0 == epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::wait(int time_out) {
  return epoll_wait(this->epoll_fd, &this->events[0], static_cast<int>(events.size()), time_out);
}
int Epoller::get_event_fd(size_t i) const {
  return this->events[i].data.fd;
}

uint32_t Epoller::get_event(size_t i) const {
  return this->events[i].events;
}
