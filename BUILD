
cc_library(
  name = "event_pool",
  srcs = glob(["include/**/*.h"]),
  hdrs = glob(["include/**/*.h"]),
  includes = ["include"],
)

cc_binary(
  name = "test",
  deps = [":event_pool"],
  srcs = ["example/test.cc"],
)