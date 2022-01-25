# event_pool

## 基本介绍：

a header-only timer based on c++11.

一个基于c++11标准，仅需要头文件的定时器实现:)。

## 使用方法：

参考example/test.cc

```c++
#include <iostream>
#include "event_pool.h"

int main() {
  // 定义一个EventPool类型的变量，构造参数为定pool中最多可同时存在的定时器事件
  EventPool::EventPool pool(102400);
  // 开启后台处理线程
  pool.Start();
  // 定义一个TimeEvent类型的变量，构造参数可以是一个std::chrono::duration，表示从此刻起，duration时间间隔后触发事件。
  // 这个类型的定时器事件可以根据回调函数的返回值反复触发-只要回调函数返回true，则会将TimeEvent的下次触发时间改为 本次触发时间 + duration（构造参数），然后重新丢入EventPool中。
  EventPool::TimeEvent time_event(std::chrono::seconds(3));
  // 设置到达指定时间后的回调函数
  time_event.SetCallBack([]() -> bool {
    std::cout << "hello world" << std::endl;
    return true;
  });
  // 构造参数也可以是一个std::chrono::time_point, 表示在指定的时刻触发事件。
  // EventPool中封装了一个方便使用的函数SecondsAfter(x)，表示当前时刻之后x秒的时刻。
  // 当然你可以指定任意的std::chrono::time_point<std::chrono::steady_clock, Duration>变量。
  EventPool::TimeEvent time_event2(EventPool::SecondsAfter(2));
  time_event2.SetCallBack([]() -> bool {
    std::cout << "hello world2" << std::endl;
    return false;
  });
  pool.PushTimeEvent(time_event);
  pool.PushTimeEvent(time_event2);
  std::this_thread::sleep_for(std::chrono::seconds(50));
  // 调用Stop函数会通知处理线程退出并join该线程，未被触发的事件会被丢弃。
  // 如果不调用，pool的析构函数中也会调用。
  pool.Stop();
  return 0;
}


```