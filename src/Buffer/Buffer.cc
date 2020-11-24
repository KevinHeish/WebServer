#include "Buffer/Buffer.h"

#include <assert.h>

Buffer::Buffer(int default_size) : data(default_size), read_index(0), write_index(0) {}

Buffer::~Buffer() {
}

size_t Buffer::get_readable_size() const {
  return (this->write_index - this->read_index);
}

size_t Buffer::get_writeable_size() const {
  return (this->data.size() - this->write_index);
}

size_t Buffer::get_prepared_size() const {
  return this->read_index;
}

char* Buffer::begin_pos() {
  return &*(this->data.begin());
}

const char* Buffer::begin_pos() const {
  return &*(this->data.begin());
}

void Buffer::append(std::string& text) {
  assert(!text.empty());
  append(text.data(), text.size());
}

void Buffer::append(const std::string& text) {
  assert(!text.empty());
  append(text.data(), text.size());
}

void Buffer::append(const char* text, size_t len) {
  assert(text);
  if (len > get_writeable_size()) {
    make_expand(len);
  }
  std::copy(text, text + len, begin_pos() + this->write_index);
  update_written_pos(len);
}

void Buffer::update_written_pos(size_t len) {
  this->write_index += len;
}

void Buffer::append(char* text, size_t len) {
  assert(text);
  append(static_cast<const char*>(text), len);
}

const char* Buffer::peek() const {
  return begin_pos() + read_index;
}

void Buffer::retrieve_until(const char* pos) {
  retrieve(pos - peek());
}

void Buffer::retrieve(size_t len) {
  this->read_index += len;
}

void Buffer::clear_all() {
  bzero(&this->data[0], this->data.size());
  read_index = 0;
  write_index = 0;
}

void Buffer::append(const Buffer& buf) {
  append(buf.peek(), buf.get_readable_size());
}

void Buffer::make_expand(size_t length) {
  if (get_writeable_size() - get_prepared_size() < length) {
    this->data.resize(this->write_index + length + 1);
  } else {
    size_t read_frg_size = get_readable_size();
    std::copy(this->data.begin() + this->read_index,
              this->data.begin() + this->write_index,
              this->data.begin());
    this->read_index = 0;
    this->write_index = this->read_index + read_frg_size;
  }
}
ssize_t Buffer::read_fd(int fd, int* save_errno) {
  char external_buf[65535];
  struct iovec io_v[2];
  const size_t writeable = get_writeable_size();
  io_v[0].iov_base = begin_pos() + this->write_index;
  io_v[0].iov_len = writeable;
  io_v[1].iov_base = external_buf;
  io_v[1].iov_len = sizeof(external_buf);

  const ssize_t len = readv(fd, io_v, 2);
  if (len < 0) {
    *save_errno = errno;
  } else if (static_cast<size_t>(len) <= writeable) {
    this->write_index += len;
  } else {
    this->write_index = this->data.size();
    append(external_buf, len - writeable);
  }
  return len;
}

ssize_t Buffer::write_fd(int fd, int* save_errno) {
  size_t read_size = get_readable_size();
  ssize_t len = write(fd, peek(), read_size);
  if (len < 0) {
    *save_errno = errno;
    return len;
  }
  this->read_index += len;
  return len;
}

char* Buffer::next_write_pos() {
  return begin_pos() + this->write_index;
}
