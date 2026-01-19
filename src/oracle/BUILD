load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "oracle",
    srcs = ["raft.cc"],
    hdrs = ["oracle.h"],
    deps = [
        "//src/io:io",
        "//src/system:system",
        "//src/node:node",
    ],
    visibility = ["//visibility:public"],
)