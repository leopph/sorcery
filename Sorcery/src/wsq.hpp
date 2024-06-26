#pragma once

/******************************
 * TASKFLOW WORK STEALING QUEUE
 *****************************/

/*MIT License

Copyright (c) 2020 T.-W. Huang

University of Utah, Salt Lake City, UT, USA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#include <atomic>
#include <vector>
#include <optional>
#include <cassert>

/**
@class: WorkStealingQueue

@tparam T data type

@brief Lock-free unbounded single-producer multiple-consumer queue.

This class implements the work stealing queue described in the paper, 
"Correct and Efficient Work-Stealing for Weak Memory Models,"
available at https://www.di.ens.fr/~zappa/readings/ppopp13.pdf.

Only the queue owner can perform pop and push operations,
while others can steal data from the queue.
*/
template<typename T>
class WorkStealingQueue {
  struct Array {
    int64_t C;
    int64_t M;
    std::atomic<T>* S;


    explicit Array(int64_t c) :
      C{c},
      M{c - 1},
      S{new std::atomic<T>[static_cast<size_t>(C)]} {}


    ~Array() {
      delete [] S;
    }


    auto capacity() const noexcept -> int64_t {
      return C;
    }


    template<typename O>
    auto push(int64_t i, O&& o) noexcept -> void {
      S[i & M].store(std::forward<O>(o), std::memory_order_relaxed);
    }


    auto pop(int64_t i) noexcept -> T {
      return S[i & M].load(std::memory_order_relaxed);
    }


    auto resize(int64_t b, int64_t t) -> Array* {
      Array* ptr = new Array{2 * C};
      for (int64_t i = t; i != b; ++i) {
        ptr->push(i, pop(i));
      }
      return ptr;
    }
  };


  std::atomic<int64_t> _top;
  std::atomic<int64_t> _bottom;
  std::atomic<Array*> _array;
  std::vector<Array*> _garbage;

public:
  /**
  @brief constructs the queue with a given capacity

  @param capacity the capacity of the queue (must be power of 2)
  */
  explicit WorkStealingQueue(int64_t capacity = 1024);

  /**
  @brief destructs the queue
  */
  ~WorkStealingQueue();

  /**
  @brief queries if the queue is empty at the time of this call
  */
  auto empty() const noexcept -> bool;

  /**
  @brief queries the number of items at the time of this call
  */
  auto size() const noexcept -> size_t;

  /**
  @brief queries the capacity of the queue
  */
  auto capacity() const noexcept -> int64_t;

  /**
  @brief inserts an item to the queue

  Only the owner thread can insert an item to the queue. 
  The operation can trigger the queue to resize its capacity 
  if more space is required.

  @tparam O data type 

  @param item the item to perfect-forward to the queue
  */
  template<typename O>
  auto push(O&& item) -> void;

  /**
  @brief pops out an item from the queue

  Only the owner thread can pop out an item from the queue. 
  The return can be a @std_nullopt if this operation failed (empty queue).
  */
  auto pop() -> std::optional<T>;

  /**
  @brief steals an item from the queue

  Any threads can try to steal an item from the queue.
  The return can be a @std_nullopt if this operation failed (not necessary empty).
  */
  auto steal() -> std::optional<T>;
};


// Constructor
template<typename T>
WorkStealingQueue<T>::WorkStealingQueue(int64_t c) {
  assert(c && (!(c & (c-1))));
  _top.store(0, std::memory_order_relaxed);
  _bottom.store(0, std::memory_order_relaxed);
  _array.store(new Array{c}, std::memory_order_relaxed);
  _garbage.reserve(32);
}


// Destructor
template<typename T>
WorkStealingQueue<T>::~WorkStealingQueue() {
  for (auto a : _garbage) {
    delete a;
  }
  delete _array.load();
}


// Function: empty
template<typename T>
auto WorkStealingQueue<T>::empty() const noexcept -> bool {
  int64_t b = _bottom.load(std::memory_order_relaxed);
  int64_t t = _top.load(std::memory_order_relaxed);
  return b <= t;
}


// Function: size
template<typename T>
auto WorkStealingQueue<T>::size() const noexcept -> size_t {
  int64_t b = _bottom.load(std::memory_order_relaxed);
  int64_t t = _top.load(std::memory_order_relaxed);
  return static_cast<size_t>(b >= t ? b - t : 0);
}


// Function: push
template<typename T>
template<typename O>
auto WorkStealingQueue<T>::push(O&& o) -> void {
  int64_t b = _bottom.load(std::memory_order_relaxed);
  int64_t t = _top.load(std::memory_order_acquire);
  Array* a = _array.load(std::memory_order_relaxed);

  // queue is full
  if (a->capacity() - 1 < (b - t)) {
    Array* tmp = a->resize(b, t);
    _garbage.push_back(a);
    std::swap(a, tmp);
    _array.store(a, std::memory_order_relaxed);
  }

  a->push(b, std::forward<O>(o));
  std::atomic_thread_fence(std::memory_order_release);
  _bottom.store(b + 1, std::memory_order_relaxed);
}


// Function: pop
template<typename T>
auto WorkStealingQueue<T>::pop() -> std::optional<T> {
  int64_t b = _bottom.load(std::memory_order_relaxed) - 1;
  Array* a = _array.load(std::memory_order_relaxed);
  _bottom.store(b, std::memory_order_relaxed);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t t = _top.load(std::memory_order_relaxed);

  std::optional<T> item;

  if (t <= b) {
    item = a->pop(b);
    if (t == b) {
      // the last item just got stolen
      if (!_top.compare_exchange_strong(t, t + 1,
        std::memory_order_seq_cst,
        std::memory_order_relaxed)) {
        item = std::nullopt;
      }
      _bottom.store(b + 1, std::memory_order_relaxed);
    }
  } else {
    _bottom.store(b + 1, std::memory_order_relaxed);
  }

  return item;
}


// Function: steal
template<typename T>
auto WorkStealingQueue<T>::steal() -> std::optional<T> {
  int64_t t = _top.load(std::memory_order_acquire);
  std::atomic_thread_fence(std::memory_order_seq_cst);
  int64_t b = _bottom.load(std::memory_order_acquire);

  std::optional<T> item;

  if (t < b) {
    Array* a = _array.load(std::memory_order_consume);
    item = a->pop(t);
    if (!_top.compare_exchange_strong(t, t + 1,
      std::memory_order_seq_cst,
      std::memory_order_relaxed)) {
      return std::nullopt;
    }
  }

  return item;
}


// Function: capacity
template<typename T>
auto WorkStealingQueue<T>::capacity() const noexcept -> int64_t {
  return _array.load(std::memory_order_relaxed)->capacity();
}
