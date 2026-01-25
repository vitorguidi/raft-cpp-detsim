load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "fuzz_input",
    srcs = ["fuzz_input.cc"],
    hdrs = ["fuzz_input.h"],
    deps = [
    ],
    visibility = ["//visibility:public"],
)


cc_library(
    name = "harness",
    srcs = ["simulation_harness.cc"],
    hdrs = ["simulation_harness.h"],
    deps = [
        ":fuzz_input",
        "//src/node:state",
        "//src/rng:rng",
        "//src/clock:clock",
        "//src/executor:executor",
        "//src/scheduler:scheduler",
        "//src/io:io",
        "//src/system:system",
        "//src/node:node",
        "//src/routing:routing",
        "//src/oracle:oracle",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "raft_simulation",
    srcs = ["raft_simulation.cc"],
    # Binary rules usually don't have separate 'hdrs' or 'visibility' 
    # unless they are being used as a dependency elsewhere.
    deps = [
        "//src/rng:rng",
        "//src/clock:clock",
        "//src/executor:executor",
        "//src/scheduler:scheduler",
        "//src/routing:routing",
        "//src/system:system",
        "//src/node:node",
        "//src/oracle:oracle",
    ],
)