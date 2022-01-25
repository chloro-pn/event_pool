#include <iostream>
#include "event_pool.h"

int main() {
  EventPool::EventPool pool(102400);
  pool.Start();
  EventPool::TimeEvent time_event(std::chrono::seconds(3));
  time_event.SetCallBack([]() -> bool {
    std::cout << "hello world" << std::endl;
    return true;
  });
  EventPool::TimeEvent time_event2(EventPool::SecondsAfter(2));
  time_event2.SetCallBack([]() -> bool {
    std::cout << "hello world2" << std::endl;
    return false;
  });
  pool.PushTimeEvent(time_event);
  pool.PushTimeEvent(time_event2);
  std::this_thread::sleep_for(std::chrono::seconds(50));
  pool.Stop();
  return 0;
}

