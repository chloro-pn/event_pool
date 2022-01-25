#pragma once

#include <chrono>
#include <functional>

namespace EventPool {
enum class Type {
  TIME_POINT,
  DURATION,
};
class TimeEvent {
 public:
  using time_type = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;

  template <typename Duration>
  explicit TimeEvent(std::chrono::time_point<std::chrono::steady_clock, Duration> time_point)
      : time_point_(std::chrono::time_point_cast<std::chrono::milliseconds>(time_point)), type_(Type::TIME_POINT) {}

  template <typename Rep, typename Period>
  explicit TimeEvent(std::chrono::duration<Rep, Period> duration)
      : time_point_(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now())),
        type_(Type::DURATION),
        duration_(std::chrono::duration_cast<std::chrono::milliseconds>(duration)) {
    time_point_ = time_point_ + duration_;
  }

  TimeEvent(const TimeEvent&) = default;
  TimeEvent& operator=(const TimeEvent&) = default;
  TimeEvent(TimeEvent&&) = default;
  TimeEvent& operator=(TimeEvent&&) = default;
  ~TimeEvent() = default;

  // 完美转发
  template <typename TaskType>
  void SetCallBack(TaskType&& task) {
    call_back_ = std::forward<TaskType>(task);
  }

  bool OnExpire() {
    if (call_back_) {
      return call_back_();
    }
    return false;
  }

  time_type& GetTimePoint() { return time_point_; }

  const time_type& GetTimePoint() const { return time_point_; }

  Type GetType() const { return type_; }

  void UpdateTimePoint() {
    if (GetType() != Type::DURATION) {
      return;
    }
    time_point_ = time_point_ + duration_;
  }

 private:
  time_type time_point_;
  Type type_;
  std::function<bool()> call_back_;
  std::chrono::milliseconds duration_;
};

std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds> SecondsAfter(uint64_t sec) {
  return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() +
                                                                 std::chrono::seconds(sec));
}
}  // namespace EventPool