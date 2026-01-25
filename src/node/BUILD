load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "node",
    srcs = ["raft_node.cc"],
    hdrs = ["node.h"],
    deps = [
        "//src/scheduler:scheduler",
        "//src/system:system",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "state",
    srcs = ["state.cc"],
    hdrs = ["state.h"],
    deps = [
        "//src/node",
    ],
    visibility = ["//visibility:public"],
)
