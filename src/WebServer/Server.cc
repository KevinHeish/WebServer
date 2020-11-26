#include <WebServer/Server.h>

using namespace std;

Server::Server(int port, int trig_mode, int timeout_inMs, bool OptLinger, int sql_port, std::string sql_user, std::string sql_passwd, std::string db_name, int connect_pool_num, int thread_num) : port(port), openLinger(OptLinger), timeout_inMs(timeout_inMs), is_close(false) {
  this->timer = make_unique<Timer>();
  this->threadpool = make_unique<ThreadPool>(thread_num);
  this->epoller = make_unique<Epoller>();
  this->src_dir = getcwd(nullptr, 256);
  assert(this->src_dir);
  strncat(this->src_dir, "/resource/root", 15);
  HttpConnect::connection_count = 0;
  HttpConnect::src_dir = this->src_dir;
  SQLConnectPool::get_instance()->init("localhost", sql_user, sql_passwd, sql_port, db_name, connect_pool_num);
  init_event_mode(trig_mode);
  if (!init_socket()) {
    this->is_close = true;
  }
}

Server::~Server() {
  close(this->listen_fd);
  this->is_close = true;
  free(this->src_dir);
  SQLConnectPool::get_instance()->ClosePool();
}

void Server::init_event_mode(int trig_mode) {
  this->listen_event = EPOLLRDHUP;
  this->connect_event = EPOLLONESHOT | EPOLLRDHUP;
  switch (trig_mode) {
    case 0:
      break;
    case 1:
      this->connect_event |= EPOLLET;
      break;
    case 2:
      this->listen_event |= EPOLLET;
      break;
    case 3:
      this->listen_event |= EPOLLET;
      this->connect_event |= EPOLLET;
      break;
    default:
      this->listen_event |= EPOLLET;
      this->connect_event |= EPOLLET;
      break;
  }
  HttpConnect::is_ET = (this->connect_event & EPOLLET);
}

void Server::start() {
  int time_Ms = -1;
  if (!this->is_close) {
    std::cout << "=======Server start==========\n";
  }
  while (!this->is_close) {
    if (this->timeout_inMs > 0) {
      time_Ms = this->timer->next_remove_start();
    }
    
    int event_count = this->epoller->wait(time_Ms);
    for (int i = 0; i < event_count; ++i) {
      int fd = this->epoller->get_event_fd(i);
      uint32_t events = this->epoller->get_event(i);
      if (fd == this->listen_fd) {
        handle_listen();
      } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
        close_connect(&this->user_table[fd]);
      } else if (events & EPOLLIN) {
        handle_read(&this->user_table[fd]);
      } else if (events & EPOLLOUT) {
        handle_write(&this->user_table[fd]);
      } else {
        std::cout << "Unexpected event";
      }
    }
  }
}

void Server::add_client(int fd, sockaddr_in address) {
  this->user_table[fd].init(fd, address);
  if (this->timeout_inMs) {
    this->timer->add_node(fd, this->timeout_inMs, std::bind(&Server::close_connect, this, &this->user_table[fd]));
  }
  this->epoller->add_fd(fd, EPOLLIN | this->connect_event);
  set_fd_nonblock(fd);
}

void Server::close_connect(HttpConnect* client) {
  this->epoller->del_fd(client->get_fd());
  client->close_connect();
}

void Server::handle_listen() {
  struct sockaddr_in address;
  socklen_t len = sizeof(address);
  do {
    int fd = accept(this->listen_fd, (struct sockaddr*)&address, &len);
    if (fd < 0) {
      //std::cout << "No other Accept.\n";
      return;
    } else if (HttpConnect::connection_count > this->MAX_FD) {
      std::cout << "Over MAX_FD number\n";
      return;
    }
    add_client(fd, address);
  } while (this->listen_event & EPOLLET);
}

void Server::handle_write(HttpConnect* client) {
  extent_time(client);
  this->threadpool->add_task(std::bind(&Server::on_write, this, client));
}

void Server::handle_read(HttpConnect* client) {
  extent_time(client);
  this->threadpool->add_task(std::bind(&Server::on_read, this, client));
}

void Server::extent_time(HttpConnect* client) {
  if (this->timeout_inMs > 0) {
    this->timer->update_time(client->get_fd(), this->timeout_inMs);
    //std::cout << "update timer\n";
  }
}

void Server::on_write(HttpConnect* client) {
  int result = -1;
  int write_errno = 0;
  result = client->write(&write_errno);

  if (client->bytes_to_write() == 0 && client->is_keep_alive()) {
    on_process(client);
    return;
  } else if (result < 0 && write_errno == EAGAIN) {
    this->epoller->mod_fd(client->get_fd(), this->connect_event | EPOLLOUT);
    return;
  }
  
  close_connect(client);
}

void Server::on_read(HttpConnect* client) {
  int result = -1;
  int read_errno = 0;
  result = client->read(&read_errno);
  if (result <= 0 && read_errno != EAGAIN) {
    close_connect(client);
  }
  on_process(client);
}

void Server::on_process(HttpConnect* client) {
  uint32_t flag;

  if (client->process()) {
    flag = this->connect_event | EPOLLOUT;
  } else {
    flag = this->connect_event | EPOLLIN;
  }
  this->epoller->mod_fd(client->get_fd(), flag);
}

int Server::set_fd_nonblock(int fd) {
  return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

bool Server::init_socket() {
  int result;
  struct sockaddr_in address;
  if (port > 65535 || port < 1024) {
    std::cout << "port error\n.";
    return false;
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(this->port);
  struct linger optLinger = {0};
  if (this->openLinger) {
    optLinger.l_onoff = 1;
    optLinger.l_linger = 1;
  }
  this->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (this->listen_fd < 0) {
    std::cout << "Error at socket creating\n";
    return false;
  }
  result = setsockopt(this->listen_fd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
  if (result < 0) {
    close(this->listen_fd);
    std::cout << "LINGER ERROR\n";
    return false;
  }
  int optvalue = 1;
  result = setsockopt(this->listen_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(int));
  if (result == -1) {
    std::cout << "set socket error\n";
    close(this->listen_fd);
    return false;
  }
  result = bind(this->listen_fd, (struct sockaddr*)&address, sizeof(address));
  if (result < 0) {
    close(this->listen_fd);
    return false;
  }
  result = listen(this->listen_fd, 6);
  if (result < 0) {
    std::cout << "listen error\n";
    close(this->listen_fd);
    return false;
  }
  result = this->epoller->add_fd(this->listen_fd, this->listen_event | EPOLLIN);
  if (result == 0) {
    std::cout << "epoller add error\n";
    close(this->listen_fd);
    return false;
  }
  set_fd_nonblock(this->listen_fd);
  return true;
}

void Server::set_close() {
  this->is_close = true;
}
