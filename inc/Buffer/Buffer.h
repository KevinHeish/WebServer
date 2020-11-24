#ifndef BUFFER_H
#define BUFFER_H

#include <sys/uio.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <vector>

class Buffer {
 private:
  void make_expand(size_t len);
  std::vector<char> data;
  std::atomic<std::size_t> read_index;
  std::atomic<std::size_t> write_index;
  char* begin_pos();
  const char* begin_pos() const;
  void update_written_pos(size_t len);

 public:
  Buffer(int default_size = 1024);
  ~Buffer();
  size_t get_writeable_size() const;
  size_t get_readable_size() const;
  size_t get_prepared_size() const;
  void append(char*, size_t);
  void append(const char*, size_t);
  void append(std::string&);
  void append(const std::string&);
  void append(const Buffer& buf);
  void clear_all();
  ssize_t read_fd(int fd, int*);
  ssize_t write_fd(int fd, int*);
  char* next_write_pos();
  const char* peek() const;
  void retrieve_until(const char* pos);
  void retrieve(size_t len);
};

#endif
