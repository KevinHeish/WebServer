#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <unordered_map>

#include "Http/HttpConnect.h"
#include "Pool/SQLConnectPool.h"
#include "Pool/SQLConnectRAII.h"
#include "Pool/ThreadPool.h"
#include "Timer/Timer.h"
#include "WebServer/Epoller.h"

class Server {
 public:
  Server(int port, int trig_mode, int timeout_inMs, bool OptLinger, int sql_port, std::string sql_user, std::string sql_passwd, std::string db_name, int connect_pool_num, int thread_num);
  ~Server();
  void start();
  void set_close();

 private:
  bool init_socket();
  void init_event_mode(int trig_mode);
  void add_client(int fd, struct sockaddr_in address);
  void handle_listen();
  void handle_write(HttpConnect* client);
  void handle_read(HttpConnect* client);
  void send_error(int fd, const char* info);
  void extent_time(HttpConnect* client);
  void close_connect(HttpConnect* client);

  void on_read(HttpConnect* client);
  void on_write(HttpConnect* client);
  void on_process(HttpConnect* client);
  static const int MAX_FD = 65535;
  static int set_fd_nonblock(int fd);
  int port;
  bool openLinger;
  int timeout_inMs;
  bool is_close;
  int listen_fd;
  char* src_dir;

  uint32_t listen_event;
  uint32_t connect_event;
  std::unique_ptr<Timer> timer;
  std::unique_ptr<ThreadPool> threadpool;
  std::unique_ptr<Epoller> epoller;
  std::unordered_map<int, HttpConnect> user_table;
};

#endif
