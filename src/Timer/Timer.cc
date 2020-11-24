#include <Timer/Timer.h>

#include <assert.h>

#include <iostream>
Timer::Timer() {
  this->heap.reserve(64);
}

Timer::~Timer() {
  this->heap.clear();
  this->ref_to_node.clear();
}

void Timer::add_node(int id, int timeout, const timeout_callBack& func) {
  size_t i;
  auto is_node_exist = this->ref_to_node.find(id);
  if (is_node_exist == this->ref_to_node.end()) {
    i = this->heap.size();
    this->ref_to_node[id] = i;
    this->heap.push_back({id, Clock::now() + mS(timeout), func});
    siftup(i);
  } else {
    i = this->ref_to_node[id];
    this->heap[i].expires = Clock::now() + mS(timeout);
    this->heap[i].func = func;
    if (!siftdown(i, this->heap.size())) {
      siftup(i);
    }
  }
}

void Timer::siftup(size_t i) {
  int p = (int)(i - 1);
  p >>= 1;
  while (p >= 0) {
    if (this->heap[p] < this->heap[i]) {
      break;
    }
    swap(p, i);
    i = p;
    p = i - 1;
    p >>= 1;
  }
}

//Return value indicates if the update node has been moved down.
//True : Origin node value has changed so need another siftup adjustment.
//False : Origin node value is same ,so no needs to siftup adj.
bool Timer::siftdown(size_t i, size_t size) {
  if (size == 0 || size == 1) {
    return true;
  }  
  size_t start_index = i;
  while (i < size ) {
    size_t left = (i << 1) + 1;
    size_t right = (i << 1) + 2;
    size_t index;
    
    if (left >= size) {   //no  children
      return i >= start_index;
    } else if (right >= size) {   // no  right child
      index = left;
    } else {   // has two child pick the bigger one 
      index = this->heap[left] < this->heap[right] ? left : right;
    }
    if (this->heap[i] < this->heap[index]) {
      return i >= start_index;
    }
    swap(index, i);
    i = index;
  }
  return i >= start_index;
}

void Timer::update_time(int id, int timeout) {
  auto IsNotExsit = this->ref_to_node.find(id);
  if (IsNotExsit == this->ref_to_node.end()) {
    return;
  }
  this->heap[this->ref_to_node[id]].expires = Clock::now() + mS(timeout);
  siftdown(this->ref_to_node[id], this->heap.size());
}

void Timer::pop() {
  del(0);
}

void Timer::remove_overtime_node() {
  if (this->heap.empty()) {
    return;
  }
  
  //node.expires = node_start+ timeout
  //node_start_time                now                    node.expires
  //|                              |                          |
  //-----------------------------------------------------------
  //If first element's expires later than Clock::now the operation is done.

  while (!this->heap.empty()) {
    auto node = this->heap.front();
    if (std::chrono::duration_cast<mS>(node.expires - Clock::now()).count() > 0) {
      return;
    }
    node.func();
    pop();
  }
}

int Timer::next_remove_start() {
  remove_overtime_node();
  size_t time_to_next = -1;
  if (!this->heap.empty()) {
    time_to_next = std::chrono::duration_cast<mS>(this->heap.front().expires - Clock::now()).count();
    if (time_to_next < 0) {
      time_to_next = 0;
    }
  }
  return time_to_next;
}

void Timer::del(size_t index) {
  size_t last_index = this->heap.size() - 1;
  if (index <= last_index) {
    swap(index, last_index);
    if (false == siftdown(index, this->heap.size())) {
      siftup(index);
    }
  }
  if (this->heap.back().func) {
    this->heap.back().func = nullptr;
  }
  this->ref_to_node.erase(this->heap.back().id);
  this->heap.pop_back();
}

void Timer::swap(size_t index_i, size_t index_j) {
  if (index_i == index_j) {
    return;
  }

  std::swap(this->heap[index_j], this->heap[index_i]);
  this->ref_to_node[index_i] = index_j;
  this->ref_to_node[index_j] = index_i;
}
