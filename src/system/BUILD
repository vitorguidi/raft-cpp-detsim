load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "system",
    srcs = ["system.cc", "sleeper_node.cc", "pinger_node.cc"],
    hdrs = ["system.h"],
    deps = [
        "//src/scheduler:scheduler",
        "//src/executor:executor",
        "//src/rng:rng",
        "//src/clock:clock",
    ],
    visibility = ["//visibility:public"],
)