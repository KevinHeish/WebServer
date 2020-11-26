#ifndef SQLCONNECTPOOL_H
#define SQLCONNECTPOOL_H

#include <mysql/mysql.h>
#include <semaphore.h>

#include <mutex>
#include <queue>
#include <string>
#include <thread>
//#include<log/log.h>

class SQLConnectPool {
 public:
  static SQLConnectPool* get_instance();
  MYSQL* get_connect();
  void free_connect(MYSQL* sql_connect);
  int get_free_connect_count();
  void init(const std::string& host, const std::string& user, const std::string& passwd, int port, std::string db_name, int connect_size);
  void ClosePool();

 private:
  SQLConnectPool();
  ~SQLConnectPool();
  int max_connection;
  int user_count;
  int free_count;
  std::queue<MYSQL*> connect_queue;
  std::mutex lock;
  sem_t semph_id;
};

#endif
