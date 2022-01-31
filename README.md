# event_pool

## 基本介绍：

a header-only timer based on c++11.

一个基于c++11标准，仅需要头文件的定时器实现:)。

## 使用方法：

参考example/test.cc

```c++
#include <iostream>
#include "event_pool/import.h"

int main() {
  // 使用定时器，首先需要定义一个EventPool类型的变量，初始化时接受的参数为支持同时存储未触发的定时器的最大数量
  EventPool::EventPool pool(102400);
  // 调用start函数开启后台处理线程
  pool.Start();
  // 定义并初始化一个TimeEvent定时器，设置到达触发时间后的回调函数，参数exit为false则表示到时触发，参数为true表示由于pool析构或者Stop触发
  EventPool::TimeEvent time_event(std::chrono::seconds(2));
  time_event.SetCallBack([](bool exit) -> bool {
    if (exit == true) {
      std::cout << "pool exit" << std::endl;
      return false;
    }
    std::cout << "hello world" << std::endl;
    return true;
  });
  // TimeEvent的初始化有两种模式，第一种如上所示，传入一个std::chrono::duration，代表周期性的经过一段时间后触发，
  // 这种模式下回调函数的返回值具有意义：如果返回为true则会更新触发时间（调用时的时间+初始化时的duration）并再次加入等待队列中
  // 从而可以反复触发。
  // 第二种模式如下所示，传入一个std::chrono::time_point，代表在某个时间点触发一次，
  // 这种模式下回调函数的返回值无意义，返回true or false 均可。
  EventPool::TimeEvent time_event2(EventPool::SecondsAfter(5));
  time_event2.SetCallBack([](bool exit) -> bool {
    if (exit == true) {
      std::cout << "pool exit" << std::endl;
      return false;
    }
    std::cout << "hello world2" << std::endl;
    return false;
  });
  // 定义并初始化的定时器通过PushTimeEvent函数丢进pool
  pool.PushTimeEvent(std::move(time_event));
  auto handler = pool.PushTimeEvent(std::move(time_event2));
  // 这里是本示例程序为了观察到定时器的调用而等待了8s，与定时器本身无关
  std::this_thread::sleep_for(std::chrono::seconds(8));
  // PushTimeEvent函数会返回一个handler，改handler可以用来通过调用Stop函数提前终止定时器
  // Stop返回true则终止成功，否则终止失败（比如在调用终止的时候后台线程已经取出该定时器并准备执行）
  // 本库保证这种语义：只要Stop函数返回true，则该定时器一定不会被执行，并且该函数的调用始终是安全的，无论对应的定时器是否已经被触发并被销毁。
  // 只有time point类型的定时器可以使用handler->Stop函数提前终止
  bool succ = handler->Stop();
  if (succ == true) {
    std::cout << "time event stop succ!" << std::endl;
  }
  std::this_thread::sleep_for(std::chrono::seconds(7));
  // 在需要停止定时器时调用Stop函数，该函数会通知后台线程，将所有还未就绪的定时器事件的回调函数传入true并调用，然后回收后台线程并返回。
  // 该函数可以手动调用，不调用的话会在EventPool的析构函数中调用，可以跨线程的多次调用。
  pool.Stop();
  return 0;
}
```