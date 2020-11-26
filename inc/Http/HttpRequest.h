#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <errno.h>

#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Buffer/Buffer.h"

class HttpRequest {
 public:
  enum PARSE_STATE {
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISH,
  };

  enum HTTP_CODE {
    NO_REQUEST = 0,
    GET_REQUEST,
    BAD_REQUEST,
    NO_RESOURSE,
    FORBIDDENT_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION,
  };

  HttpRequest() { init(); }
  ~HttpRequest() = default;

  void init();
  bool parse(Buffer& buff);

  std::string get_url() const;
  std::string& get_url();
  std::string get_method() const;
  std::string get_version() const;
  std::string get_post(const std::string& key) const;
  std::string get_post(const char* key) const;

  bool is_keep_alive() const;

 private:
  bool parse_request(const std::string& line);
  void parse_header(const std::string& line);
  void parse_body(const std::string& line);

  void parse_path();
  void parse_post();
  void parse_from_urlencoded();

  static bool user_verify(const std::string& name, const std::string& pwd);
  //return true if account is create , otherwise false;
  static bool create_account(const std::string& name, const std::string& pwd);

  PARSE_STATE state;

  std::string method, url, version, body;
  std::unordered_map<std::string, std::string> header;
  std::unordered_map<std::string, std::string> post;

  static const std::unordered_set<std::string> DEFAULT_HTML;
  static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
  static int convert_hex2digit(char ch);
};

#endif
