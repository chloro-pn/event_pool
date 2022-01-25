
cc_library(
  name = "event_pool",
  srcs = ["include/time_event.h"],
  hdrs = ["include/event_pool.h"],
  includes = ["include"],
)

cc_binary(
  name = "test",
  deps = [":event_pool"],
  srcs = ["example/test.cc"],
)