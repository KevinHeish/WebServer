#include "Http/HttpConnect.h"

#include <assert.h>

const char* HttpConnect::src_dir = "./resource/root";
const std::string HttpConnect::src_dir_str = "./resource/root";
std::atomic<int> HttpConnect::connection_count;
bool HttpConnect::is_ET;

using namespace std;

HttpConnect::HttpConnect() {
  this->fd = -1;
  this->address = {0};
  this->is_close = true;
}

HttpConnect::~HttpConnect() {
  if (this->is_close == false) {
    close_connect();
  }
}

void HttpConnect::init(int fd, struct sockaddr_in& address) {
  this->fd = fd;
  this->address = address;
  ++connection_count;
  this->write_buffer.clear_all();
  this->read_buffer.clear_all();
  this->is_close = false;
}

void HttpConnect::close_connect() {
  this->response.unmap_file();
  if (this->is_close == false) {
    this->is_close = true;
    connection_count--;
    close(this->fd);
    //LOG_INFO
  }
}

int HttpConnect::get_fd() const {
  return this->fd;
}

bool HttpConnect::is_keep_alive() const {
  return this->request.is_keep_alive();
}

struct sockaddr_in HttpConnect::get_address() const {
  return this->address;
}

const char* HttpConnect::get_ip() const {
  return inet_ntoa(this->address.sin_addr);
}

int HttpConnect::get_port() const {
  return this->address.sin_port;
}

ssize_t HttpConnect::read(int* save_errno) {
  ssize_t len = -1;
  do {
    len = this->read_buffer.read_fd(this->fd, save_errno);
    if (len < 0) break;
  } while (is_ET);
  return len;
}

int HttpConnect::bytes_to_write() {
  return this->io_vector[0].iov_len + this->io_vector[1].iov_len;
}

ssize_t HttpConnect::write(int* save_errno) {
  ssize_t len = -1;
  do {
    len = writev(this->fd, this->io_vector, this->iov_count);
    if (len <= 0) {
      *save_errno = errno;
      break;
    }
    if ((this->io_vector[0].iov_len + this->io_vector[1].iov_len) == 0) {
      break;
    } else if (static_cast<size_t>(len) > this->io_vector[0].iov_len) {
      this->io_vector[1].iov_base = static_cast<uint8_t*>(this->io_vector[1].iov_base) + (len - this->io_vector[0].iov_len);
      this->io_vector[1].iov_len -= (len - this->io_vector[0].iov_len);
      if (this->io_vector[0].iov_len) {
        this->write_buffer.clear_all();
        this->io_vector[0].iov_len = 0;
      }
    } else {
      this->io_vector[0].iov_base = static_cast<uint8_t*>(this->io_vector[0].iov_base) + len;
      this->io_vector[0].iov_len = 0;
      this->write_buffer.retrieve(len);
    }
  } while (this->is_ET || bytes_to_write() > 1024);  //bytes_to_write is confused.
  return len;
}

bool HttpConnect::process() {
  this->request.init();
  if (this->read_buffer.get_readable_size() <= 0) {
    return false;
  } else if (this->request.parse(this->read_buffer)) {
    this->response.init(this->request.get_url(), this->request.is_keep_alive(), 200);
  } else {
    this->response.init(this->request.get_url(), false, 400);
  }

  this->response.make_response(this->write_buffer);
  this->io_vector[0].iov_base = const_cast<char*>(this->write_buffer.peek());
  this->io_vector[0].iov_len = this->write_buffer.get_readable_size();
  this->iov_count = 1;

  char* file_ptr = this->response.get_file();
  size_t file_size = this->response.get_file_len();

  if (file_ptr != nullptr && file_size > 0) {
    this->io_vector[1].iov_base = file_ptr;
    this->io_vector[1].iov_len = file_size;
    this->iov_count = 2;
  }
  return true;
}
