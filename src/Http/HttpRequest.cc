#include "Http/HttpRequest.h"

#include <mysql/mysql.h>

#include "Pool/SQLConnectRAII.h"

using namespace std;

enum user_table_info {
  USERNAME = 0,
  PASSWORD
};

const unordered_set<string> HttpRequest::DEFAULT_HTML{
    "/index",
    "/register",
    "/log",
    "/welcome",
    "/video",
    "/picture",
};

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0},
    {"/log.html", 1},
};

void HttpRequest::init() {
  this->method = "";
  this->url = "";
  this->version = "";
  this->body = "";
  this->state = REQUEST_LINE;
  this->header.clear();
  this->post.clear();
}
bool HttpRequest::is_keep_alive() const {
  auto is_find = this->header.find("Connection");
  if (is_find != this->header.end()) {
    return is_find->second == "keep-alive" && this->version == "1.1";
  } else {
    return false;
  }
}

bool HttpRequest::parse(Buffer& buff) {
  const char CRLF[] = "\r\n";
  if (buff.get_readable_size() <= 0) {
    return false;
  }
  while (buff.get_readable_size() && this->state != FINISH) {
    const char* line_end = search(buff.peek(), (const char*)buff.next_write_pos(), &CRLF[0], CRLF + 2);
    string read_text(buff.peek(), line_end);
    switch (this->state) {
      case REQUEST_LINE:
        if (true != parse_request(read_text)) {
          return false;
        }
        parse_path();
        break;
      case HEADERS:
        parse_header(read_text);
        if (buff.get_readable_size() < 2) {
          this->state = FINISH;
        }
        break;
      case BODY:
        parse_body(read_text);
        break;
      default:
        break;
    }
    if (line_end == buff.next_write_pos()) {
      break;
    }
    buff.retrieve_until(line_end + 2);
  }
  return true;
}

bool HttpRequest::parse_request(const string& line) {
  //[^ ] : any char except ' '
  //^: outside the brackets represents start-of-line
  //$: outside the brackets represents end-of-line
  regex request_format("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
  smatch parse_result;
  if (regex_match(line, parse_result, request_format)) {
    //parse_result[0] stores the origin string varialbe "line"
    this->method = parse_result[1];
    this->url = parse_result[2];
    this->version = parse_result[3];
    this->state = HEADERS;
    return true;
  }

  //LOG_ERROR("Request line parse error");
  return false;
}

void HttpRequest::parse_path() {
  if (this->url == "/") {
    this->url = "/index.html";
  } else {
    for (auto& key : DEFAULT_HTML) {
      if (this->url == key) {
        this->url += ".html";
        break;
      }
    }
  }
}

void HttpRequest::parse_header(const string& line) {
  regex format("^([^:]*): ?(.*)$");
  smatch parse_result;
  if (regex_match(line, parse_result, format)) {
    header[parse_result[1]] = parse_result[2];
  } else {
    this->state = BODY;
  }
}

void HttpRequest::parse_body(const string& line) {
  this->body = line;
  parse_post();
  this->state = FINISH;
  //LOG_DEBUG("Body:%s, length:%d", line.c_string(), line.size());
}

int HttpRequest::convert_hex2digit(char ch) {
  if (ch >= 'A' || ch <= 'F') {
    ch -= 'A';
    return static_cast<int>(ch + 10);
  }
  if (ch >= 'a' || ch <= 'f') {
    ch -= 'a';
    return static_cast<int>(ch + 10);
  }
  return static_cast<int>(ch);
}

void HttpRequest::parse_post() {
  if (this->method == "POST" && this->header["Content-Type"] == "application/x-www-form-urlencoded") {
    parse_from_urlencoded();
    if (DEFAULT_HTML_TAG.count(this->url)) {
      int tag = DEFAULT_HTML_TAG.find(this->url)->second;
      if (tag == 0 || tag == 1) {
        bool is_login = (tag == 1);
        std::cout << "Connecting to SQL DB";
        if (is_login) {
          if (user_verify(this->post["username"], this->post["password"])) {
            this->url = "/welcome.html";
          } else {
            this->url = "/logError.html";
          }
        } else {
          if (create_account(this->post["username"], this->post["password"])) {
            this->url = "/log.html";
          } else {
            this->url = "/registerError.html";
          }
        }
      }
    }
  }
}

void HttpRequest::parse_from_urlencoded() {
  int body_len = body.size();
  if (body_len == 0) {
    return;
  }

  string key, value;
  int num = 0;
  int i = 0, j = 0;

  for (; i < body_len; i++) {
    char ch = this->body[i];
    switch (ch) {
      case '=':
        key = this->body.substr(j, i - j);
        j = i + 1;
        break;
      case '+':
        this->body[i] = ' ';
        break;
      case '%':
        num = convert_hex2digit(this->body[i + 1]) * 16 + convert_hex2digit(this->body[i + 2]);
        this->body[i + 2] = num % 10 + '0';
        this->body[i + 1] = num / 10 + '0';
        i += 2;
        break;
      case '&':
        value = this->body.substr(j, i - j);
        j = i + 1;
        this->post[key] = value;
        //LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
        break;
      default:
        break;
    }
  }
  if (this->post.count(key) == 0 && j < i) {
    value = this->body.substr(j, i - j);
    this->post[key] = value;
  }
}
bool HttpRequest::create_account(const std::string& name, const std::string& passwd) {
  if (name.empty() || passwd.empty()) {
    return false;
  }
  MYSQL* sql;
  SQLConnectRAII(&sql, SQLConnectPool::get_instance());
  if (sql == nullptr) {
    std::cout << "SQL DB connect failed.\n";
    return false;
  }

  MYSQL_FIELD* fields = nullptr;
  MYSQL_RES* result;
  char query[128] = "";

  sprintf(query, "SELECT username FROM user");

  //legal query return 0 else non-zero.
  if (mysql_query(sql, query)) {
    std::cout << "Illegal query statement.\n";
  } else {
    std::cout << "Username selected.\n Check password.\n";
  }

  result = mysql_store_result(sql);
  fields = mysql_fetch_fields(result);
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) {
    string account_in_db(row[0]);
    if (account_in_db == name) {
      std::cout << "Username is duplicate.\n";
      mysql_free_result(result);
      SQLConnectPool::get_instance()->free_connect(sql);
      return false;
    }
  }
  //insert
  memset(query, '\0', 128);
  sprintf(query, "INSERT INTO user (username,passwd) values('%s','%s');", &name[0], &passwd[0]);

  if (mysql_query(sql, query)) {
    std::cout << "Illegal query statement.\n";
    mysql_free_result(result);
    SQLConnectPool::get_instance()->free_connect(sql);
    return false;
  } else {
    std::cout << "Create account success.";
  }
  SQLConnectPool::get_instance()->free_connect(sql);
  return true;
}

bool HttpRequest::user_verify(const string& name, const string& passwd) {
  if (name.empty() || passwd.empty()) {
    return false;
  }
  MYSQL* sql;
  SQLConnectRAII(&sql, SQLConnectPool::get_instance());
  if (sql == nullptr) {
    std::cout << "SQL DB connect failed.\n";
    return false;
  }

  MYSQL_FIELD* fields = nullptr;
  MYSQL_RES* result;
  char query[128] = "";

  sprintf(query, "SELECT username ,passwd FROM user WHERE username='%s'", &name[0]);

  //legal query return 0 else non-zero.
  if (mysql_query(sql, query)) {
    SQLConnectPool::get_instance()->free_connect(sql);
    return false;
  } else {
    std::cout << "Username selected.\n Check password.\n";
  }

  result = mysql_store_result(sql);
  fields = mysql_fetch_fields(result);

  int row_num = mysql_num_rows(result);
  if (row_num == 0) {
    SQLConnectPool::get_instance()->free_connect(sql);
    return false;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  string password_in_db(row[1]);
  if (0 != strcmp(passwd.c_str(), password_in_db.c_str())) {
    std::cout << "Wrong password";
    mysql_free_result(result);
    SQLConnectPool::get_instance()->free_connect(sql);
    return false;
  }
  mysql_free_result(result);
  SQLConnectPool::get_instance()->free_connect(sql);
  return true;
}

string HttpRequest::get_url() const {
  return this->url;
}

string& HttpRequest::get_url() {
  return this->url;
}

string HttpRequest::get_method() const {
  return this->method;
}

string HttpRequest::get_version() const {
  return this->version;
}

string HttpRequest::get_post(const string& key) const {
  if (key.empty()) {
    return "";
  }
  auto is_find = this->post.find(key);
  if (is_find != this->post.end()) {
    return is_find->second;
  }
  return "";
}

string HttpRequest::get_post(const char* key) const {
  if (key == nullptr) {
    return "";
  }
  auto is_find = this->post.find(key);
  if (is_find != this->post.end()) {
    return is_find->second;
  }
  return "";
}
