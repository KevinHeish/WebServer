#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
 public:
  explicit ThreadPool(size_t thread_count = 8) : pool_(std::make_shared<Pool>()) {
    for (size_t i = 0; i < thread_count; ++i) {
      std::thread([pool = pool_] {
          std::unique_lock<std::mutex> locker(pool->mutex_lock);
        while (true) {
          if (!pool->task.empty()) {
            auto task = std::move(pool->task.front());
            pool->task.pop();
            locker.unlock();
            task();
	    locker.lock();
          } else if (pool->is_close) {
            break;
          } else {
            pool->condition.wait(locker);
          }
        }
      }).detach();
    }
  };
  ThreadPool(ThreadPool&&) = default;
  ThreadPool() = default;
  ~ThreadPool() {
    if (static_cast<bool>(pool_)) {
      {
        std::lock_guard<std::mutex> locker(pool_->mutex_lock);
        pool_->is_close = true;
      }
      pool_->condition.notify_all();
    }
  }

  template <class T>
  void add_task(T&& task) {
    {
      std::lock_guard<std::mutex> locker(pool_->mutex_lock);
      pool_->task.emplace(std::forward<T>(task));
    }
    pool_->condition.notify_one();
  }

 private:
  struct Pool {
    std::mutex mutex_lock;
    std::condition_variable condition;
    bool is_close;
    std::queue<std::function<void()>> task;
  };
  std::shared_ptr<Pool> pool_;
};

#endif
