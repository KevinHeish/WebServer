#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <string>
#include <unordered_map>

#include "Buffer/Buffer.h"

class HttpResponse {
 private:
  static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
  static const std::unordered_map<int, std::string> CODE_STATUS;
  static const std::unordered_map<int, std::string> CODE_PATH;
  char* mm_file;
  struct stat mm_file_status;
  std::string path;
  const std::string src_dir = "./resource/root";
  bool is_keep_alive;
  int code;
  std::string get_file_type();
  void error_html();
  void add_header(Buffer& buf);
  void add_state_line(Buffer& buf);
  void add_content(Buffer& buf);

 public:
  HttpResponse();
  ~HttpResponse();

  void init(std::string& path, bool is_keep_alive = false, int code = -1);
  void make_response(Buffer& buff);
  void unmap_file();
  char* get_file();
  size_t get_file_len() const;
  void error_content(Buffer& buf, std::string message);
  int get_code() const;
};

#endif
