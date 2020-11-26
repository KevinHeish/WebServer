#ifndef SQLCONNECTRAII_H
#define SQLCONNECTRAII_H

#include <Pool/SQLConnectPool.h>

class SQLConnectRAII {
 public:
  SQLConnectRAII(MYSQL** sql, SQLConnectPool* connect_pool) {
    *sql = connect_pool->get_connect();
    this->sql = *sql;
    this->connect_pool = connect_pool;
  }
  ~SQLConnectRAII() {
    if (this->sql) {
      this->connect_pool->free_connect(this->sql);
    }
  }

 private:
  MYSQL* sql;
  SQLConnectPool* connect_pool;
};

#endif
