#pragma once

#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <thread>

#include "time_event.h"

namespace EventPool {
class EventPool {
 public:
  explicit EventPool(uint64_t max_count) : max_time_event_count_(max_count), stop_(false) {
    
  }

  void PushTimeEvent(const TimeEvent& te) {
    std::unique_lock<std::mutex> guard(mut_);
    not_fill_cv_.wait(guard, [this]() -> bool {
      return this->timer_queue_.size() < this->max_time_event_count_;
    });
    timer_queue_.push(te);
    at_least_one_cv_.notify_all();
  }

  void PushTimeEvents(const std::vector<TimeEvent>& tes) {
    std::unique_lock<std::mutex> guard(mut_);
    not_fill_cv_.wait(guard, [&, this]()->bool {
      return this->timer_queue_.size() + tes.size() < this->max_time_event_count_;
    });
    for(auto& each : tes) {
      timer_queue_.push(each);
    }
    at_least_one_cv_.notify_all();
  }

  std::vector<TimeEvent> GetReady(bool& stop) {
    std::vector<TimeEvent> result;
    std::unique_lock<std::mutex> guard(mut_);
    auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
    while(timer_queue_.empty() == false) {
      if (now >= timer_queue_.top().GetTimePoint()) {
        result.push_back(timer_queue_.top());
        timer_queue_.pop();
      } else {
        break;
      }
    }
    stop = stop_;
    // 三种情况
    // 情况1 ： 这次取到了，则直接返回
    // 情况2 ： 这次没取到，而且timer_queue_中是空的
    // 情况3 ： 这次没取到，timer_queue_不是空的
    if (result.empty() == false) {
      not_fill_cv_.notify_all();
      return result;
    }
    if (timer_queue_.empty() == true) {
      at_least_one_cv_.wait(guard, [this]()->bool {
        return this->timer_queue_.empty() == false || stop_ == true;
      });
    } else {
      std::chrono::milliseconds dt = timer_queue_.top().GetTimePoint() - now;
      at_least_one_cv_.wait_for(guard, dt);
    }
    stop = stop_;
    return result;
  }

  void Run() {
    while(true) {
      bool stop = false;
      std::vector<TimeEvent> events = GetReady(stop);
      std::vector<TimeEvent> continue_to;
      for(auto& each : events) {
        bool more = each.OnExpire();
        // 对于DURATION类型的事件而言，返回true则表示更新时间戳并继续放入事件池中。
        if (more && each.GetType() == Type::DURATION) {
          each.UpdateTimePoint();
          continue_to.push_back(each);
        }
      }
      if (stop == true) {
        return;
      }
      // 后台线程应该直接加锁并push，然后通知等待在at_least_one_cv_上的线程，不能调用PushTimeEvents，否则引起死锁。
      std::unique_lock<std::mutex> guard(mut_);
      for (auto& each : continue_to) {
        timer_queue_.push(each);
      }
      at_least_one_cv_.notify_all();
    }
  }

  void Stop() {
    std::unique_lock<std::mutex> guard(mut_);
    // 确保只唤醒一次
    if (stop_ == true) {
      return;
    }
    stop_ = true;
    guard.unlock();
    // 唤醒Run函数
    at_least_one_cv_.notify_all();
    if (backend_.joinable()) {
      backend_.join();
    }
  }

  ~EventPool() {
    Stop();
  }

  void Start() {
    backend_ = std::thread([this]() {
      this->Run();
    });
  }

 private:
  std::mutex mut_;
  struct cmp_for_time_event {
    bool operator()(const TimeEvent& t1, const TimeEvent& t2) {
      return t1.GetTimePoint() > t2.GetTimePoint();
    }
  };
  std::priority_queue<TimeEvent, std::vector<TimeEvent>, cmp_for_time_event> timer_queue_;
  uint64_t max_time_event_count_;
  std::condition_variable not_fill_cv_;
  std::condition_variable at_least_one_cv_;
  bool stop_;
  std::thread backend_;
};
}