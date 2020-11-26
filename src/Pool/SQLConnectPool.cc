#include <Pool/SQLConnectPool.h>
#include <assert.h>
using namespace std;

SQLConnectPool::SQLConnectPool() {
  this->user_count = 0;
  this->free_count = 0;
}

SQLConnectPool::~SQLConnectPool() {
  ClosePool();
}

SQLConnectPool* SQLConnectPool::get_instance() {
  static SQLConnectPool connect_pool;
  return &connect_pool;
}

void SQLConnectPool::init(const std::string& host, const std::string& user, const std::string& passwd, int port, std::string db_name, int connect_size = 10) {
  assert(connect_size > 0);
  for (int i = 0; i < connect_size; ++i) {
    MYSQL* sql = nullptr;
    sql = mysql_init(sql);
    if (!sql) {
      //LOG_ERROR("Mysql init error");
      assert(sql);
    }
    sql = mysql_real_connect(sql, (const char*)&host[0], (const char*)&user[0], (const char*)&passwd[0], (const char*)&db_name[0], 0, NULL, 0);
    if (!sql) {
      assert(sql);
      //LOG_ERROR("Mysql connect error");
    }
    this->connect_queue.push(sql);
  }
  this->max_connection = connect_size;
  sem_init(&this->semph_id, 0, this->max_connection);
}

MYSQL* SQLConnectPool::get_connect() {
  MYSQL* sql = nullptr;
  if (this->connect_queue.empty()) {
    //LOG_WARN("Sql connect pool has no free thread");
    return nullptr;
  }
  sem_wait(&this->semph_id);
  {
    std::lock_guard<mutex> locker(this->lock);
    sql = this->connect_queue.front();
    this->connect_queue.pop();
  }
  return sql;
}

void SQLConnectPool::free_connect(MYSQL* sql_connect) {
  lock_guard<mutex> locker(this->lock);
  this->connect_queue.push(sql_connect);
  sem_post(&this->semph_id);
}

void SQLConnectPool::ClosePool() {
  lock_guard<mutex> locker(this->lock);
  while (!this->connect_queue.empty()) {
    auto connection = this->connect_queue.front();
    this->connect_queue.pop();
    mysql_close(connection);
  }
  mysql_library_end();
}

int SQLConnectPool::get_free_connect_count() {
  lock_guard<mutex> locker(this->lock);
  return this->connect_queue.size();
}
