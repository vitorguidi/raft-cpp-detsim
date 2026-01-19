load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "node",
    srcs = ["sleeper_node.cc", "pinger_node.cc"],
    hdrs = ["node.h"],
    deps = [
        "//src/scheduler:scheduler",
        "//src/system:system",
    ],
    visibility = ["//visibility:public"],
)