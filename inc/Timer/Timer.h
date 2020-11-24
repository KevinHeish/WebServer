#ifndef TIMER_H
#define TIMER_H

#include <arpa/inet.h>
#include <time.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <queue>
#include <unordered_map>

typedef std::function<void()> timeout_callBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds mS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
  int id;
  TimeStamp expires;
  timeout_callBack func;
  bool operator<(const TimerNode& T) {
    return expires < T.expires;
  }
};

class Timer {
 public:
  Timer();
  ~Timer();
  void add_node(int id, int timeout, const timeout_callBack& func);
  void pop();
  void update_time(int id, int timeout);
  int next_remove_start();

 private:
  std::vector<TimerNode> heap;
  std::unordered_map<int, size_t> ref_to_node;
  void siftup(size_t index);
  bool siftdown(size_t index, size_t size);
  void swap(size_t index_i, size_t index_j);
  void del(size_t index);
  void remove_overtime_node();
};

#endif
