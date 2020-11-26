#ifndef HTTPCONNECT_H
#define HTTPCONNECT_H

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "Buffer/Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpConnect {
 private:
  int fd;
  struct sockaddr_in address;
  bool is_close;
  struct iovec io_vector[2];
  int iov_count;
  Buffer write_buffer, read_buffer;
  HttpRequest request;
  HttpResponse response;

 public:
  struct sockaddr_in get_address() const;
  int get_fd() const;
  int get_port() const;
  HttpConnect();
  ~HttpConnect();
  void init(int socket_fd, struct sockaddr_in& address);
  ssize_t read(int* save_errno);
  ssize_t write(int* save_errno);
  void close_connect();
  const char* get_ip() const;
  bool process();
  int bytes_to_write();
  bool is_keep_alive() const;

  static bool is_ET;
  static const char* src_dir;
  static const std::string src_dir_str;
  static std::atomic<int> connection_count;
};

#endif
