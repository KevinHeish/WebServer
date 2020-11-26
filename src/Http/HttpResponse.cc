#include "Http/HttpResponse.h"

#include <assert.h>

#include "Http/HttpConnect.h"
using namespace std;

const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    {200, "OK"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
};

const unordered_map<int, string> HttpResponse::CODE_PATH = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

HttpResponse::HttpResponse() {
  this->code = -1;
  this->path = "";
  this->is_keep_alive = false;
  this->mm_file = nullptr;
  this->mm_file_status = {0};
}

HttpResponse::~HttpResponse() {
  unmap_file();
}

void HttpResponse::init(string& path, bool is_keep_alive, int code) {
  assert(src_dir.empty() != true);
  this->path = path;
  this->is_keep_alive = is_keep_alive;
  this->code = code;
  if (this->mm_file) {
    unmap_file();
  }
  this->mm_file = nullptr;
  this->mm_file_status = {0};
}

void HttpResponse::make_response(Buffer& buf) {
  if (stat((this->src_dir + this->path).data(), &this->mm_file_status) < 0 || S_ISDIR(this->mm_file_status.st_mode)) {
    this->code = 404;
  } else if (!(this->mm_file_status.st_mode & S_IROTH)) {
    this->code = 403;
  } else if (this->code == -1) {
    this->code = 200;
  }
  error_html();
  add_state_line(buf);
  add_header(buf);
  add_content(buf);
}

char* HttpResponse::get_file() {
  return this->mm_file;
}

size_t HttpResponse::get_file_len() const {
  return this->mm_file_status.st_size;
}

void HttpResponse::error_html() {
  auto is_find = CODE_PATH.find(this->code);
  if (is_find != CODE_PATH.end()) {
    stat((this->src_dir + this->path).data(), &this->mm_file_status);
  }
}

void HttpResponse::add_state_line(Buffer& buf) {
  string status;
  auto status_code_value = CODE_STATUS.find(this->code);
  if (status_code_value != CODE_STATUS.end()) {
    status = status_code_value->second;
  } else {
    this->code = 400;
    status = (CODE_STATUS.find(400))->second;
  }
  buf.append("HTTP/1.1 " + to_string(this->code) + " " + status + "\r\n");
}

void HttpResponse::add_header(Buffer& buf) {
  buf.append("Connection: ");
  if (this->is_keep_alive) {
    buf.append("keep-alive\r\n");
    buf.append("keep-alive: max=6, timeout=120\r\n");
  } else {
    buf.append("close\r\n");
  }
  buf.append("Content-type: " + get_file_type() + "\r\n");
}

void HttpResponse::add_content(Buffer& buf) {
  int src_fd = open((this->src_dir + this->path).data(), O_RDONLY);
  //std::cout << (this->src_dir + this->path).data() << '\n';
  if (src_fd < 0) {
    error_content(buf, "File NOT FOUND.!\n");
    return;
  }
  //LOG_DEBUG("file path %s", (this->src_dir + this->path).data() );
  int* mm_ret = (int*)mmap(0, this->mm_file_status.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
  if (!mm_ret && (*mm_ret == -1)) {
    error_content(buf, "File NOT FOUND at map file!");
    return;
  }
  this->mm_file = (char*)mm_ret;
  close(src_fd);
  buf.append("Content-length: " + to_string(this->mm_file_status.st_size) + "\r\n\r\n");
}

string HttpResponse::get_file_type() {
  string::size_type index = this->path.find_last_of('.');
  if (index != string::npos) {
    string suffix = this->path.substr(index);
    auto is_find = SUFFIX_TYPE.find(suffix);
    if (is_find != SUFFIX_TYPE.end()) {
      return is_find->second;
    }
  }
  return "text/plain";
}

void HttpResponse::error_content(Buffer& buf, string msg) {
  string body;
  string status;
  body += "<html><title>Error</title>";
  body += "<body bgcolor=\"ffffff\">";
  auto is_find = CODE_STATUS.find(this->code);
  if (is_find != CODE_STATUS.end()) {
    status = is_find->second;
  } else {
    status = "Bad Request";
  }
  body += to_string(this->code) + " : " + status + "\n";
  body += "<p>" + msg + "</p>";
  body += "<hr><em>Server</em></body></html>";

  buf.append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
  buf.append(body);
}

void HttpResponse::unmap_file() {
  if (this->mm_file) {
    munmap(this->mm_file, this->mm_file_status.st_size);
    this->mm_file = nullptr;
  }
}
